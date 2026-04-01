#include <QApplication>
#include <memory>
#include "src/controller/ScannerController.h"
#include "src/view/QtView.h"

int main(int argc, char* argv[]) {
    // Mode console si des arguments sont passés
    if (argc > 1) {
        auto model = std::make_shared<ScannerModel>();
        ScannerController controller(model);
        return controller.run(argc, argv);
    }
    
    // Mode GUI par défaut
    QApplication app(argc, argv);
    
    // Créer le modèle et le contrôleur
    auto model = std::make_shared<ScannerModel>();
    auto controller = std::make_shared<ScannerController>(model);
    
    // Créer et afficher la fenêtre Qt
    QtView window(model, controller);
    window.show();
    
    return app.exec();
}
