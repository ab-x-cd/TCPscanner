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
#include <memory>

class ScannerController {
private:
    std::shared_ptr<ScannerModel> model;
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

    std::string getLogsDirectory() const {
        // Chercher le répertoire du projet en remontant depuis le répertoire courant
        std::filesystem::path currentPath = std::filesystem::current_path();
        
        // Si on est dans le dossier build, remonter d'un niveau
        if (currentPath.filename() == "build") {
            currentPath = currentPath.parent_path();
        }
        
        // Vérifier si on est dans le dossier tcp_scanner (ou si on peut le trouver)
        // On cherche le CMakeLists.txt pour identifier la racine du projet
        while (!currentPath.empty()) {
            if (std::filesystem::exists(currentPath / "CMakeLists.txt")) {
                return (currentPath / "Logs").string();
            }
            std::filesystem::path parent = currentPath.parent_path();
            if (parent == currentPath) {
                // On est arrivé à la racine du système
                break;
            }
            currentPath = parent;
        }
        
        // Par défaut, utiliser Logs dans le répertoire courant
        return "Logs";
    }

    bool writeOpenPortsLog(const std::string& ip,
                           int startPort,
                           int endPort,
                           const std::vector<std::pair<int, std::string>>& openPorts,
                           std::string& outputFileName) const {
        const std::filesystem::path logsDir(getLogsDirectory());
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
    ScannerController();
    explicit ScannerController(std::shared_ptr<ScannerModel> model);

    std::shared_ptr<ScannerModel> getModel() const { return model; }
    
    int run(int argc, char* argv[]);
};