#include "MainWindow.h"
#include <qfile.h>
#include <qmenubar.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_configDialog(new ConfigDialog(this))
{
    ui->setupUi(this);

    // ��ʼ״̬
    ui->actionDisconnect->setEnabled(false);

    // �����ź�
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
            statusBar()->showMessage(tr("�������Ӵ��� %1...").arg(m_configDialog->portName()), 3000);
        }
        else {
            statusBar()->showMessage(tr("��������ʧ��"), 3000);
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
        statusBar()->showMessage(tr("�����ӵ������豸"), 5000);
    }
    else {
        statusBar()->showMessage(tr("�ѶϿ�����"), 3000);
    }
}

void MainWindow::onErrorOccurred(const QString& message)
{
    QMessageBox::warning(this, tr("Modbus ����"), message);
    statusBar()->showMessage(tr("����: %1").arg(message), 5000);
}

MainWindow::~MainWindow()
{
}
