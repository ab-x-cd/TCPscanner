#include "QtView.h"
#include "ScannerModel.h"
#include "ScannerController.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFont>
#include <QApplication>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

QtView::QtView(std::shared_ptr<ScannerModel> model, 
               std::shared_ptr<ScannerController> controller,
               QWidget* parent)
    : QMainWindow(parent), model(model), controller(controller) {
    setWindowTitle("TCP Port Scanner");
    setGeometry(100, 100, 800, 600);
    
    setupUI();
    connectSignals();
}

QtView::~QtView() = default;

void QtView::setupUI() {
    // Widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Layout principal
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // --- Groupe des paramètres d'entrée ---
    QGroupBox* inputGroupBox = new QGroupBox("Paramètres de scan", this);
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroupBox);

    // IP input
    QHBoxLayout* ipLayout = new QHBoxLayout();
    QLabel* ipLabel = new QLabel("Adresse IP cible:", this);
    ipInput = new QLineEdit(this);
    ipInput->setPlaceholderText("ex: 192.168.1.1");
    ipLayout->addWidget(ipLabel);
    ipLayout->addWidget(ipInput);
    inputLayout->addLayout(ipLayout);

    // Ports input
    QHBoxLayout* portsLayout = new QHBoxLayout();
    QLabel* startPortLabel = new QLabel("Port de début:", this);
    startPortInput = new QSpinBox(this);
    startPortInput->setRange(0, 65535);
    startPortInput->setValue(1);
    portsLayout->addWidget(startPortLabel);
    portsLayout->addWidget(startPortInput);

    QLabel* endPortLabel = new QLabel("Port de fin:", this);
    endPortInput = new QSpinBox(this);
    endPortInput->setRange(0, 65535);
    endPortInput->setValue(1000);
    portsLayout->addWidget(endPortLabel);
    portsLayout->addWidget(endPortInput);
    portsLayout->addStretch();
    inputLayout->addLayout(portsLayout);

    // Checkbox pour afficher l'usage des ports
    showPortUsageCheckbox = new QCheckBox("Afficher l'usage des ports", this);
    showPortUsageCheckbox->setChecked(true);
    inputLayout->addWidget(showPortUsageCheckbox);

    mainLayout->addWidget(inputGroupBox);

    // --- Boutons de scan et log ---
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    
    scanButton = new QPushButton("Démarrer le scan", this);
    scanButton->setMinimumHeight(40);
    QFont buttonFont = scanButton->font();
    buttonFont.setPointSize(11);
    buttonFont.setBold(true);
    scanButton->setFont(buttonFont);
    scanButton->setStyleSheet("background-color: #4CAF50; color: white; border: none; padding: 5px;");
    buttonsLayout->addWidget(scanButton);

    openLogButton = new QPushButton("Ouvrir le dernier log", this);
    openLogButton->setMinimumHeight(40);
    openLogButton->setFont(buttonFont);
    openLogButton->setStyleSheet("background-color: #2196F3; color: white; border: none; padding: 5px;");
    openLogButton->setEnabled(false);  // Désactivé jusqu'à ce qu'un scan soit effectué
    buttonsLayout->addWidget(openLogButton);

    mainLayout->addLayout(buttonsLayout);

    // --- Zone de résultats ---
    QGroupBox* resultsGroupBox = new QGroupBox("Résultats du scan", this);
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroupBox);
    resultsDisplay = new QTextEdit(this);
    resultsDisplay->setReadOnly(true);
    resultsDisplay->setFont(QFont("Courier", 10));
    resultsLayout->addWidget(resultsDisplay);
    mainLayout->addWidget(resultsGroupBox);

    centralWidget->setLayout(mainLayout);
}

void QtView::connectSignals() {
    connect(scanButton, &QPushButton::clicked, this, &QtView::onScanButtonClicked);
    connect(openLogButton, &QPushButton::clicked, this, &QtView::onOpenLogButtonClicked);
}

void QtView::onScanButtonClicked() {
    QString ip = ipInput->text();
    int startPort = startPortInput->value();
    int endPort = endPortInput->value();

    // Validation basique
    if (ip.isEmpty()) {
        displayMessage("Erreur: Veuillez entrer une adresse IP", true);
        return;
    }

    if (startPort > endPort) {
        displayMessage("Erreur: Le port de début doit être inférieur au port de fin", true);
        return;
    }

    // Marquer le scan comme actif et mettre à jour le bouton
    setScanButtonState(true);
    
    // Afficher le message de début de scan
    resultsDisplay->clear();
    displayMessage(QString("Scan de %1 sur les ports %2-%3...\n")
                   .arg(ip)
                   .arg(startPort)
                   .arg(endPort));

    // Configurer le modèle
    if (!model->setTarget(ip.toStdString(), startPort, endPort)) {
        displayMessage("Erreur: Adresse IP ou plage de ports invalide", true);
        setScanButtonState(false);
        return;
    }

    // Effectuer le scan
    std::vector<std::pair<int, std::string>> openPorts;

    for (int port = startPort; port <= endPort; ++port) {
        std::string banner;
        bool isOpen = model->scanPort(port, banner);
        
        if (isOpen) {
            QString result = QString("[OPEN] Port %1").arg(port);
            if (!banner.empty()) {
                result += QString(" | Banner: %1").arg(QString::fromStdString(banner));
            }
            displayMessage(result);
            openPorts.emplace_back(port, banner);
        } else {
            displayMessage(QString("[CLOSED] Port %1").arg(port));
        }
        
        // Permet à l'interface de rester réactive
        qApp->processEvents();
    }

    // Créer le fichier de log
    displayMessage("\nCréation du fichier log...");
    
    // Obtenir le chemin du répertoire des logs à la racine du projet
    std::string logsDir = getLogsDirectory();
    std::filesystem::create_directories(logsDir);
    
    // Créer le nom du fichier de log
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
    
    std::string ipSafe = ip.toStdString();
    std::replace(ipSafe.begin(), ipSafe.end(), '.', '_');
    std::string logFileName = logsDir + "/scan_log_" + ipSafe + "_" + timestamp.str() + ".txt";
    
    // Écrire le fichier de log
    std::ofstream logFile(logFileName);
    if (logFile) {
        logFile << "TCP Scanner - Rapport de scan\n";
        logFile << "Cible: " << ip.toStdString() << "\n";
        logFile << "Plage scannee: " << startPort << "-" << endPort << "\n";
        logFile << "Nombre de ports ouverts: " << openPorts.size() << "\n\n";

        if (openPorts.empty()) {
            logFile << "Aucun port ouvert detecte sur cette plage.\n";
        } else {
            logFile << "Ports ouverts et utilite actuelle:\n";
            for (const auto& entry : openPorts) {
                logFile << "- Port " << entry.first;
                if (!entry.second.empty()) {
                    logFile << " | Banner: " << entry.second;
                }
                logFile << "\n";
            }
        }
        logFile.close();
        lastLogFilePath = logFileName;  // Sauvegarder le chemin du log
        openLogButton->setEnabled(true); // Activer le bouton pour ouvrir le log
        displayMessage(QString("Log du scan créé: %1").arg(QString::fromStdString(logFileName)));
    } else {
        displayMessage("Erreur: Impossible de créer le fichier log du scan", true);
        openLogButton->setEnabled(false);
    }

    displayMessage("\nScan terminé.");
    
    // Marquer le scan comme inactif et réactiver le bouton
    setScanButtonState(false);
}

void QtView::displayMessage(const QString& message, bool isError) {
    if (isError) {
        resultsDisplay->append("<span style=\"color: red;\">" + message + "</span>");
    } else {
        resultsDisplay->append(message);
    }
}

void QtView::appendPortUsage(int port, const QString& usage) {
    if (showPortUsageCheckbox->isChecked()) {
        resultsDisplay->append(QString("  → %1").arg(usage));
    }
}

void QtView::setScanButtonState(bool scanning) {
    isScanningActive = scanning;
    
    if (scanning) {
        // Pendant le scan : bouton grisé et inactif
        scanButton->setText("Scan en cours...");
        scanButton->setEnabled(false);
        scanButton->setStyleSheet(
            "background-color: #CCCCCC; "
            "color: #666666; "
            "border: none; "
            "padding: 5px;"
        );
    } else {
        // Scan terminé : bouton actif et coloré
        scanButton->setText("Démarrer le scan");
        scanButton->setEnabled(true);
        scanButton->setStyleSheet(
            "background-color: #4CAF50; "
            "color: white; "
            "border: none; "
            "padding: 5px;"
        );
    }
}

std::string QtView::getLogsDirectory() const {
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

void QtView::onOpenLogButtonClicked() {
    if (lastLogFilePath.empty()) {
        displayMessage("Erreur: Aucun fichier log disponible", true);
        return;
    }

    // Lire le fichier log
    std::ifstream logFile(lastLogFilePath);
    if (!logFile.is_open()) {
        displayMessage("Erreur: Impossible d'ouvrir le fichier log", true);
        return;
    }

    // Lire le contenu du fichier
    std::stringstream buffer;
    buffer << logFile.rdbuf();
    logFile.close();

    // Afficher le contenu dans une nouvelle zone (remplacer le contenu actuel)
    resultsDisplay->clear();
    resultsDisplay->append("=== RAPPORT DE SCAN ===\n");
    resultsDisplay->append(QString::fromStdString(buffer.str()));
}

