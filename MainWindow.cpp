#include "MainWindow.h"
#include <qfile.h>
#include <qmenubar.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_configDialog(new ConfigDialog(this))
{
    ui->setupUi(this);

    // 初始状态
    ui->actionDisconnect->setEnabled(false);

    // 连接信号
    connect(m_modbusManager, qOverload<bool>(&ModbusManager::connectionStateChanged),
        this, &MainWindow::onConnectionStateChanged);
}

void MainWindow::on_actionConnect_triggered()
{
    if (m_configDialog->exec() == QDialog::Accepted) {
        if (m_modbusManager->connectSerial(
            m_configDialog->portName(),
            m_configDialog->baudRate(),
            m_configDialog->dataBits(),
            m_configDialog->stopBits(),
            m_configDialog->parity(),
            m_configDialog->flowControl()))
        {
            statusBar()->showMessage(tr("正在连接串口 %1...").arg(m_configDialog->portName()), 3000);
        }
        else {
            statusBar()->showMessage(tr("连接启动失败"), 3000);
        }
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    m_modbusManager->disconnectDevice();
}

void MainWindow::onConnectionStateChanged(bool connected)
{
    ui->actionConnect->setEnabled(!connected);
    ui->actionDisconnect->setEnabled(connected);

    if (connected) {
        statusBar()->showMessage(tr("已连接到串口设备"), 5000);
    }
    else {
        statusBar()->showMessage(tr("已断开连接"), 3000);
    }
}

void MainWindow::onErrorOccurred(const QString& message)
{
    QMessageBox::warning(this, tr("Modbus 错误"), message);
    statusBar()->showMessage(tr("错误: %1").arg(message), 5000);
}

MainWindow::~MainWindow()
{
}
