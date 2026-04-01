#include "ScannerModel.h"
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {
#ifdef _WIN32
bool ensureWinsockInitialized() {
    static const bool initialized = []() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    }();
    return initialized;
}

bool setSocketNonBlocking(SOCKET sock) {
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
}

void closeSocket(SOCKET sock) {
    closesocket(sock);
}
#else
bool setSocketNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
}

void closeSocket(int sock) {
    close(sock);
}
#endif
} // namespace

bool ScannerModel::isValidIPv4(const std::string& ip) const {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1;
}

bool ScannerModel::setTarget(const std::string& ip, int start, int end) {
    if (!isValidIPv4(ip)) return false;
    if (start < 1 || start > 65535 || end < 1 || end > 65535 || start > end) return false;
    
    targetIp = ip;
    startPort = start;
    endPort = end;
    return true;
}

bool ScannerModel::scanPort(int port, std::string& banner_out) const {
#ifdef _WIN32
    if (!ensureWinsockInitialized()) {
        return false;
    }
#endif

    auto sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (sock == INVALID_SOCKET) return false;
#else
    if (sock < 0) return false;
#endif

    if (!setSocketNonBlocking(sock)) {
        closeSocket(sock);
        return false;
    }

    struct sockaddr_in server;
    std::memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (inet_pton(AF_INET, targetIp.c_str(), &server.sin_addr) <= 0) {
        closeSocket(sock);
        return false;
    }

    bool connected = false;
    int result = connect(sock, (struct sockaddr*)&server, sizeof(server));

    if (result == 0) {
        connected = true;
    } else {
#ifdef _WIN32
        int connectError = WSAGetLastError();
        if (connectError != WSAEINPROGRESS && connectError != WSAEWOULDBLOCK && connectError != WSAEINVAL) {
            closeSocket(sock);
            return false;
        }
#else
        if (errno != EINPROGRESS) {
            closeSocket(sock);
            return false;
        }
#endif

        fd_set write_fds;
        struct timeval timeout = {1, 0}; 
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);

        int select_res = select(static_cast<int>(sock) + 1, nullptr, &write_fds, nullptr, &timeout);
        if (select_res > 0) {
            int socket_error;
            int len = sizeof(socket_error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socket_error), &len) == 0 && socket_error == 0) {
                connected = true;
            }
        }
    }

    bool is_open = false;
    if (connected) {
        is_open = true;
        const char* trigger = "HEAD / HTTP/1.0\r\n\r\n";
        send(sock, trigger, static_cast<int>(strlen(trigger)), 0);

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        struct timeval timeout = {0, 500000}; // 500 ms
        if (select(static_cast<int>(sock) + 1, &read_fds, nullptr, nullptr, &timeout) > 0) {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer)); 
            int bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                for(int i=0; i < bytes_read; ++i) {
                    if(buffer[i] == '\r' || buffer[i] == '\n') buffer[i] = ' ';
                }
                banner_out = std::string(buffer);
            }
        }
    }
    closeSocket(sock);
    return is_open;
}