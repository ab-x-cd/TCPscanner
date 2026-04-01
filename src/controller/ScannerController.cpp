#include "ScannerController.h"
#include <iostream>

ScannerController::ScannerController() 
    : model(std::make_shared<ScannerModel>()) {}

ScannerController::ScannerController(std::shared_ptr<ScannerModel> model)
    : model(model) {}

int ScannerController::run(int argc, char* argv[]) {
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

    if (!model->setTarget(ip, start, end)) {
        view.displayError("Adresse IP ou plage de ports invalide");
        return 1;
    }

    view.displayScanStart(model->getTargetIp(), model->getStartPort(), model->getEndPort());

    std::vector<std::pair<int, std::string>> openPorts;

    for (int port = model->getStartPort(); port <= model->getEndPort(); ++port) {
        std::string banner;
        bool isOpen = model->scanPort(port, banner);
        view.displayPortResult(port, isOpen, banner);
        if (isOpen) {
            openPorts.emplace_back(port, banner);
        }
    }

    std::string logFileName;
    if (writeOpenPortsLog(model->getTargetIp(), model->getStartPort(), model->getEndPort(), openPorts, logFileName)) {
        std::cout << "Log du scan cree: " << logFileName << "\n";
    } else {
        view.displayError("Impossible de creer le fichier log du scan");
    }

    return 0;
}
