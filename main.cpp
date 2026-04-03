#ifdef QT_FOUND
#include <QApplication>
#include "src/view/QtView.h"
#endif

#include <memory>
#include "src/controller/ScannerController.h"

int main(int argc, char* argv[]) {
    // Mode console (CLI) - utilisé si des arguments sont passés ou si Qt n'est pas disponible
    if (argc > 1) {
        auto model = std::make_shared<ScannerModel>();
        ScannerController controller(model);
        return controller.run(argc, argv);
    }
    
#ifdef QT_FOUND
    // Mode GUI par défaut (si Qt est disponible)
    QApplication app(argc, argv);
    
    // Créer le modèle et le contrôleur
    auto model = std::make_shared<ScannerModel>();
    auto controller = std::make_shared<ScannerController>(model);
    
    // Créer et afficher la fenêtre Qt
    QtView window(model, controller);
    window.show();
    
    return app.exec();
#else
    // Si Qt n'est pas disponible, lancer le mode CLI avec usage
    auto model = std::make_shared<ScannerModel>();
    ScannerController controller(model);
    const char* usage[] = {argv[0]};
    return controller.run(1, (char**)usage);
#endif
}
