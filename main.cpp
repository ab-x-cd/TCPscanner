#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <cerrno>

using string = std::string ;

bool is_valid_ipv4(const char* ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1;
}

bool scan_port_nonblock(const char* target_ip, int port, std::string& banner_out);

int main(int argc, char* argv[]) { 

    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <IP> <PORT_DEBUT> <PORT_FIN>" << std::endl;
        std::cout << "Exemple: " << argv[0] << " 127.0.0.1 20 80" << std::endl;
        return 1;
    }
    const char* ip = argv[1];
    if (!is_valid_ipv4(ip)) {
        std::cout << "Adresse IP invalide" << std::endl;
        return 1;
    }
    string startPort = argv[2];
    string endPort = argv[3];

    int start, end;
    try {
        start = std::stoi(startPort);
        end = std::stoi(endPort);
    } catch (...) {
        std::cout << "Ports invalides (doivent être des nombres)" << std::endl;
        return 1;
    }
    if (start < 1 || start > 65535 || end < 1 || end > 65535 || start > end) {
        std::cout << "Ports invalides (doivent être entre 1 et 65535, et début <= fin)" << std::endl;
        return 1;
    }

    for(int port = start; port <= end; ++port) 
    {
        std::string banner;
        if (scan_port_nonblock(ip, port, banner )) {
            std::cout << "Port " << port << " ouvert";
            if (!banner.empty()) {
                std::cout << " | Service : " << banner ;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}

bool scan_port_nonblock(const char* target_ip, int port, std::string& banner_out) 
{ 
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;


    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK); 
    

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port); //convserion host to network short (htons)

    // consersion IP string => binaire
    if (inet_pton(AF_INET, target_ip, &server.sin_addr) <= 0) {
        std::cerr << "Adresse IP invalide" << std::endl;
        close(sock);
        return false;
    }


    // Step 1 : connexion
    bool connected = false;
    int result = connect(sock, (struct sockaddr*)&server, sizeof(server));

    if (result == 0) {
        connected = true; // port immédiat, chances que ce soit en local
    }
    else if (errno == EINPROGRESS) {
        fd_set write_fds;
        struct timeval timeout = {1, 0}; 
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);

        int select_res = select(sock + 1, nullptr, &write_fds, nullptr, &timeout);
        if (select_res > 0) {
            int socket_error;
            socklen_t len = sizeof(socket_error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &socket_error, &len) == 0) {
                if (socket_error == 0) {
                    connected = true;
                }
            }
        }
    }

    //Step 2 : banner grabbing

    bool is_open = false;

    if (connected) {
        is_open = true;

        // awake un coup
        const char* trigger = "HEAD / HTTP/1.0\r\n\r\n";
        send(sock, trigger, strlen(trigger), 0);

        // écoute prête
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);


        struct timeval timeout = {0, 500000}; // 500 ms

        if (select(sock + 1, &read_fds, nullptr, nullptr, &timeout) > 0) {
            char buffer[1024];
            memset(buffer, 0, sizeof(buffer)); 
            
            ssize_t bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                
            
                for(int i=0; i < bytes_read; ++i) {
                    if(buffer[i] == '\r' || buffer[i] == '\n') buffer[i] = ' ';
                }
                
                banner_out = std::string(buffer);
            }
        }
        
    }

    close(sock);
    return is_open;
}