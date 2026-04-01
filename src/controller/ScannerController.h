#pragma once
#include "ScannerModel.h"
#include "ConsoleView.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

class ScannerController {
private:
    ScannerModel model;
    ConsoleView view;

    std::string getPortUsage(int port) const {
        switch (port) {
            case 20: return "FTP Data";
            case 21: return "FTP Control";
            case 22: return "SSH (administration distante)";
            case 23: return "Telnet (non securise)";
            case 25: return "SMTP (envoi d'emails)";
            case 53: return "DNS";
            case 67: return "DHCP Server";
            case 68: return "DHCP Client";
            case 80: return "HTTP (web)";
            case 110: return "POP3";
            case 123: return "NTP (synchronisation heure)";
            case 143: return "IMAP";
            case 161: return "SNMP";
            case 389: return "LDAP";
            case 443: return "HTTPS (web securise)";
            case 445: return "SMB (partage Windows)";
            case 587: return "SMTP Submission";
            case 993: return "IMAPS";
            case 995: return "POP3S";
            case 1433: return "Microsoft SQL Server";
            case 1521: return "Oracle Database";
            case 3306: return "MySQL/MariaDB";
            case 3389: return "RDP (Bureau a distance)";
            case 5432: return "PostgreSQL";
            case 6379: return "Redis";
            case 8080: return "HTTP alternatif/proxy";
            case 27017: return "MongoDB";
            default: return "Service inconnu ou specifique";
        }
    }

    std::string createLogFileName(const std::string& ip) const {
        auto now = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        std::tm localTm = {};
#ifdef _WIN32
        localtime_s(&localTm, &nowTime);
#else
        localtime_r(&nowTime, &localTm);
#endif

        std::ostringstream timestamp;
        timestamp << std::put_time(&localTm, "%Y%m%d_%H%M%S");

        std::string ipSafe = ip;
        std::replace(ipSafe.begin(), ipSafe.end(), '.', '_');
        return "scan_log_" + ipSafe + "_" + timestamp.str() + ".txt";
    }

    bool writeOpenPortsLog(const std::string& ip,
                           int startPort,
                           int endPort,
                           const std::vector<std::pair<int, std::string>>& openPorts,
                           std::string& outputFileName) const {
        const std::filesystem::path logsDir("Logs");
        std::error_code ec;
        std::filesystem::create_directories(logsDir, ec);

        const std::filesystem::path logPath = logsDir / createLogFileName(ip);
        outputFileName = logPath.string();
        std::ofstream logFile(logPath);
        if (!logFile) {
            return false;
        }

        logFile << "TCP Scanner - Rapport de scan\n";
        logFile << "Cible: " << ip << "\n";
        logFile << "Plage scannee: " << startPort << "-" << endPort << "\n";
        logFile << "Nombre de ports ouverts: " << openPorts.size() << "\n\n";

        if (openPorts.empty()) {
            logFile << "Aucun port ouvert detecte sur cette plage.\n";
            return true;
        }

        logFile << "Ports ouverts et utilite actuelle:\n";
        for (const auto& entry : openPorts) {
            logFile << "- Port " << entry.first << " | Utilite: " << getPortUsage(entry.first);
            if (!entry.second.empty()) {
                logFile << " | Banner: " << entry.second;
            }
            logFile << "\n";
        }

        return true;
    }

public:
    int run(int argc, char* argv[]) {
        if (argc != 4) {
            view.displayUsage(argv[0]);
            return 1;
        }

        std::string ip = argv[1];
        int start, end;

        try {
            start = std::stoi(argv[2]);
            end = std::stoi(argv[3]);
        } catch (...) {
            view.displayError("Ports invalides (doivent être des nombres)");
            return 1;
        }

        if (!model.setTarget(ip, start, end)) {
            view.displayError("Adresse IP ou plage de ports invalide");
            return 1;
        }

        view.displayScanStart(model.getTargetIp(), model.getStartPort(), model.getEndPort());

        std::vector<std::pair<int, std::string>> openPorts;

        for (int port = model.getStartPort(); port <= model.getEndPort(); ++port) {
            std::string banner;
            bool isOpen = model.scanPort(port, banner);
            view.displayPortResult(port, isOpen, banner);
            if (isOpen) {
                openPorts.emplace_back(port, banner);
            }
        }

        std::string logFileName;
        if (writeOpenPortsLog(model.getTargetIp(), model.getStartPort(), model.getEndPort(), openPorts, logFileName)) {
            std::cout << "Log du scan cree: " << logFileName << "\n";
        } else {
            view.displayError("Impossible de creer le fichier log du scan");
        }

        return 0;
    }
};