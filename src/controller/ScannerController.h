#pragma once
#include "ScannerModel.h"
#include "ConsoleView.h"
#include <string>

class ScannerController {
private:
    ScannerModel model;
    ConsoleView view;

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

        for (int port = model.getStartPort(); port <= model.getEndPort(); ++port) {
            std::string banner;
            bool isOpen = model.scanPort(port, banner);
            view.displayPortResult(port, isOpen, banner);
        }

        return 0;
    }
};