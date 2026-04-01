#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <memory>
#include <string>

class ScannerModel;
class ScannerController;

class QtView : public QMainWindow {
    Q_OBJECT

private:
    // Modèle et contrôleur
    std::shared_ptr<ScannerModel> model;
    std::shared_ptr<ScannerController> controller;

    // Widgets
    QLineEdit* ipInput;
    QSpinBox* startPortInput;
    QSpinBox* endPortInput;
    QPushButton* scanButton;
    QPushButton* openLogButton;
    QTextEdit* resultsDisplay;
    QCheckBox* showPortUsageCheckbox;
    
    // État du scan
    bool isScanningActive = false;
    std::string lastLogFilePath = "";

    // Méthodes privées
    void setupUI();
    void connectSignals();
    std::string getLogsDirectory() const;
    void setScanButtonState(bool scanning);

public:
    explicit QtView(std::shared_ptr<ScannerModel> model, 
                    std::shared_ptr<ScannerController> controller,
                    QWidget* parent = nullptr);
    ~QtView() override;

private slots:
    void onScanButtonClicked();
    void onOpenLogButtonClicked();
    void displayMessage(const QString& message, bool isError = false);
    void appendPortUsage(int port, const QString& usage);
};

