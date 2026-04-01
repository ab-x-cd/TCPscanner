#pragma once
#include <string>

class ScannerModel {
private:
    std::string targetIp;
    int startPort;
    int endPort;

public:
    ScannerModel() : startPort(0), endPort(0) {}

    bool isValidIPv4(const std::string& ip) const;
    bool setTarget(const std::string& ip, int start, int end);
    bool scanPort(int port, std::string& banner_out) const;

    std::string getTargetIp() const { return targetIp; }
    int getStartPort() const { return startPort; }
    int getEndPort() const { return endPort; }
};