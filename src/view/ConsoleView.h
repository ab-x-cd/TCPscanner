#pragma once
#include <iostream>
#include <string>

class ConsoleView {
public:
    void displayUsage(const std::string& programName) const {
        std::cerr << "Usage: " << programName << " <IP_CIBLE> <PORT_DEBUT> <PORT_FIN>\n";
    }

    void displayError(const std::string& message) const {
        std::cerr << "Erreur: " << message << "\n";
    }

    void displayScanStart(const std::string& ip, int startPort, int endPort) const {
        std::cout << "Scan de " << ip << " sur les ports " << startPort << "-" << endPort << "\n";
    }

    void displayPortResult(int port, bool isOpen, const std::string& banner) const {
        if (isOpen) {
            std::cout << "[OPEN] Port " << port;
            if (!banner.empty()) {
                std::cout << " | Banner: " << banner;
            }
            std::cout << "\n";
            return;
        }

        std::cout << "[CLOSED] Port " << port << "\n";
    }
};