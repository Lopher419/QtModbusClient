#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "ModbusManager.h"
#include "ConfigDialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void onConnectionStateChanged(bool connected);
    void onErrorOccurred(const QString& message);

private:
    Ui::MainWindow* ui;
    ModbusManager* m_modbusManager;
    ConfigDialog* m_configDialog;
};
