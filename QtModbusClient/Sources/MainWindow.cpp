#include "MainWindow.h"
#include <QPointer>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    m_connection = new ModbusConnection(this);
    modbusDialog.reset(new ModbusConfigDialog(this));

    setupCoilTab();
    setupDITab();
    setupIRTab();
	setupHRTab();
    setupConnections();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupCoilTab()
{
    if (auto coilPlaceholder = ui.tabCoil->findChild<QWidget*>("coilWidget")) {
        auto layout = new QVBoxLayout(coilPlaceholder);
        layout->setContentsMargins(0, 0, 0, 0);

        m_coilWidget = new CoilWidget(coilPlaceholder);
        layout->addWidget(m_coilWidget);
        m_coilWidget->setModbusConnection(m_connection);
    }
    else {
        qWarning() << "Coil placeholder widget not found!";
    }
}

void MainWindow::setupDITab()
{
    if (auto diPlaceholder = ui.tabDI->findChild<QWidget*>("diWidget")) {
        auto layout = new QVBoxLayout(diPlaceholder);
        layout->setContentsMargins(0, 0, 0, 0);

        m_diWidget = new DIWidget(diPlaceholder);
        layout->addWidget(m_diWidget);
        m_diWidget->setModbusConnection(m_connection);
    }
    else {
        qWarning() << "DI placeholder widget not found!";
    }
}

void MainWindow::setupIRTab()
{
    if (auto irPlaceholder = ui.tabIr->findChild<QWidget*>("irWidget")) {
        auto layout = new QVBoxLayout(irPlaceholder);
        layout->setContentsMargins(0, 0, 0, 0);

        m_irWidget = new IRWidget(irPlaceholder);
        layout->addWidget(m_irWidget);
        m_irWidget->setModbusConnection(m_connection);
    }
    else {
        qWarning() << "IR placeholder widget not found!";
    }
}

void MainWindow::setupHRTab()
{
    if (auto hrPlaceholder = ui.tabHR->findChild<QWidget*>("hrWidget")) {
        auto layout = new QVBoxLayout(hrPlaceholder);
        layout->setContentsMargins(0, 0, 0, 0);

        m_hrWidget = new HRWidget(hrPlaceholder);
        layout->addWidget(m_hrWidget);
        m_hrWidget->setModbusConnection(m_connection);
    }
    else {
        qWarning() << "HR placeholder widget not found!";
    }
}

void MainWindow::setupConnections()
{
    connect(ui.actionConnect, &QAction::triggered, this, &MainWindow::onConnectTriggered);
    connect(ui.actionDisconnect, &QAction::triggered, this, &MainWindow::onDisconnectTriggered);
    connect(m_connection, &ModbusConnection::connectionOpened, this, &MainWindow::onConnected);
    connect(m_connection, &ModbusConnection::connectionOpened, this, [this]() {
        emit connectionStateChanged(true);
        });
}

// Handle connect action
void MainWindow::onConnectTriggered()
{
    if (modbusDialog->exec() == QDialog::Accepted) {
        m_connection->connectToDevice(
            modbusDialog->getPort(),
            modbusDialog->getBaudRate(),
            modbusDialog->getDataBits(),
            modbusDialog->getParity(),
            modbusDialog->getStopBits(),
            modbusDialog->getSlaveID());
    }
}

// Handle disconnect action
void MainWindow::onDisconnectTriggered()
{
    if (m_connection) {
        m_connection->closeConnection();
    }
}

// Handle successful connection
void MainWindow::onConnected()
{
    qDebug() << "Modbus connection established";

    QPointer<QModbusReply> reply(m_connection->readRegister(
        ModbusConnection::HoldingRegisters, 0, 10));

    if (reply) {
        connect(reply, &QModbusReply::finished, [reply]() {
            if (!reply) return;

            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit result = reply->result();
            }
            reply->deleteLater();
            });
    }
}