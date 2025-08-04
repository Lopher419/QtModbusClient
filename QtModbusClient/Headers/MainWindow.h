#pragma once

#include <QMainWindow>
#include <QScopedPointer>
#include <QPointer>

#include "ui_MainWindow.h"
#include "ModbusConfigDialog.h"
#include "ModbusConnection.h"
#include "CoilWidget.h"
#include "DIWidget.h"
#include "IRWidget.h"
#include "HRWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void connectionStateChanged(bool connected);

private slots:
    void onConnectTriggered();
    void onDisconnectTriggered();
    void onConnected();

private:
    void setupCoilTab();
    void setupDITab();
    void setupIRTab();
    void setupHRTab();
    void setupConnections();

    Ui::MainWindow ui;
    QScopedPointer<ModbusConfigDialog> modbusDialog;
    ModbusConnection* m_connection = nullptr;
    CoilWidget* m_coilWidget = nullptr;
    DIWidget* m_diWidget = nullptr;
    IRWidget* m_irWidget = nullptr;
	HRWidget* m_hrWidget = nullptr;
};