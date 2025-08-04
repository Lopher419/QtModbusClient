// DIWidget.cpp
#include "DIWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QEventLoop>
#include <QHeaderView>
#include <QApplication>
#include <QStandardItem>

DIWidget::DIWidget(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    initUI();
    initTableModel();
    setupConnections();
}

DIWidget::~DIWidget()
{
    safeDeleteReply(m_singleDIReadReply);
    safeDeleteReply(m_multipleDIReadReply);
}

void DIWidget::setModbusConnection(ModbusConnection* connection)
{
    m_modbusConnection = connection;
}

void DIWidget::setupConnections()
{
    connect(ui.diReadSingleBtn, &QPushButton::clicked, this, &DIWidget::onReadSingleDI);
    connect(ui.diReadmultiplePtn, &QPushButton::clicked, this, &DIWidget::onReadMultipleDI);
}

void DIWidget::initUI()
{
    ui.diReadSingleAddressSpinBox->setRange(0, 65535);
    ui.diReadSingleDataLineEdit->setReadOnly(true);

    ui.diReadMultipleAddressSpinBox->setRange(0, 65535);
    ui.diReadMultipleCountSpinBox->setRange(1, 2000);
    ui.diReadMultipleCountSpinBox->setValue(16);
}

void DIWidget::initTableModel()
{
    m_diModel = new QStandardItemModel(this);
    m_diModel->setColumnCount(3);
    m_diModel->setHorizontalHeaderLabels({ tr("Address"), tr("Hex"), tr("Binary") });
    ui.diReadMultipleDataTableView->setModel(m_diModel);
    ui.diReadMultipleDataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.diReadMultipleDataTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DIWidget::safeDeleteReply(QModbusReply* reply)
{
    if (!reply) return;

    disconnect(reply, nullptr, nullptr, nullptr);
    if (!reply->isFinished()) {
        QEventLoop loop;
        connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }
    reply->deleteLater();
}

// Handle single discrete input read request
void DIWidget::onReadSingleDI()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, "Error", "Not connected to any device");
        return;
    }

    safeDeleteReply(m_singleDIReadReply);
    ui.diReadSingleBtn->setEnabled(false);

    const int address = ui.diReadSingleAddressSpinBox->value();
    m_singleDIReadReply = m_modbusConnection->readRegister(
        ModbusConnection::DiscreteInputs, address, 1);

    if (m_singleDIReadReply) {
        connect(m_singleDIReadReply, &QModbusReply::finished,
            this, &DIWidget::handleSingleDIReadResult);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to send read request");
        ui.diReadSingleBtn->setEnabled(true);
    }
}

// Process single discrete input read result
void DIWidget::handleSingleDIReadResult()
{
    auto* reply = qobject_cast<QModbusReply*>(sender());
    ui.diReadSingleBtn->setEnabled(true);

    if (!reply || reply != m_singleDIReadReply) {
        if (reply) reply->deleteLater();
        m_singleDIReadReply = nullptr;
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        processSingleDIResult(reply);
    }
    else {
        qDebug() << "Single DI read error:" << reply->errorString();
    }

    safeDeleteReply(m_singleDIReadReply);
}

void DIWidget::processSingleDIResult(QModbusReply* reply)
{
    const QModbusDataUnit result = reply->result();
    if (result.registerType() == QModbusDataUnit::DiscreteInputs && result.valueCount() > 0) {
        const bool diState = result.value(0);
        ui.diReadSingleDataLineEdit->setText(diState ? "1" : "0");

        const int address = ui.diReadSingleAddressSpinBox->value();
        qDebug() << "DI read successful - Address:" << address << "Value:" << diState;
        QApplication::beep();
    }
    else {
        qDebug() << "Invalid DI data received";
    }
}

// Handle multiple discrete inputs read request
void DIWidget::onReadMultipleDI()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, tr("Error"), tr("Not connected to any device"));
        return;
    }

    safeDeleteReply(m_multipleDIReadReply);
    ui.diReadmultiplePtn->setEnabled(false);

    const int address = ui.diReadMultipleAddressSpinBox->value();
    const int count = ui.diReadMultipleCountSpinBox->value();

    m_multipleDIReadReply = m_modbusConnection->readRegister(
        ModbusConnection::DiscreteInputs, address, count);

    if (m_multipleDIReadReply) {
        connect(m_multipleDIReadReply, &QModbusReply::finished,
            this, &DIWidget::handleMultipleDIReadResult);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to send read request"));
        ui.diReadmultiplePtn->setEnabled(true);
    }
}

// Process multiple discrete inputs read results
void DIWidget::handleMultipleDIReadResult()
{
    auto* reply = qobject_cast<QModbusReply*>(sender());
    ui.diReadmultiplePtn->setEnabled(true);

    if (!reply || reply != m_multipleDIReadReply) {
        if (reply) reply->deleteLater();
        m_multipleDIReadReply = nullptr;
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        processMultipleDIResult(reply);
    }
    else {
        qDebug() << "Multiple DI read error:" << reply->errorString();
    }

    safeDeleteReply(m_multipleDIReadReply);
}

void DIWidget::processMultipleDIResult(QModbusReply* reply)
{
    const QModbusDataUnit result = reply->result();
    if (result.registerType() != QModbusDataUnit::DiscreteInputs || result.valueCount() <= 0) {
        qDebug() << "Invalid multiple DI data received";
        return;
    }

    m_diModel->removeRows(0, m_diModel->rowCount());
    const int startAddr = result.startAddress();
    const int count = result.valueCount();
    const int byteCount = (count + 7) / 8;
    QVector<quint8> bytes(byteCount, 0);

    for (int i = 0; i < count; ++i) {
        const bool value = result.value(i);
        const int addr = startAddr + i;
        const int byteIndex = i / 8;
        const int bitIndex = i % 8;

        if (value) {
            bytes[byteIndex] |= (1 << bitIndex);
        }

        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::number(addr));
        rowItems << new QStandardItem();
        rowItems << new QStandardItem(value ? "1" : "0");
        m_diModel->appendRow(rowItems);
    }

    updateHexColumn(bytes, byteCount);
    qDebug() << "Multiple DI read successful - Start address:" << startAddr << "Count:" << count;
    QApplication::beep();
}

void DIWidget::updateHexColumn(const QVector<quint8>& bytes, int byteCount)
{
    for (int i = 0; i < byteCount; ++i) {
        const int rowIndex = i * 8;
        if (rowIndex < m_diModel->rowCount()) {
            const QModelIndex index = m_diModel->index(rowIndex, 1);
            m_diModel->setData(index, QString("0x%1").arg(bytes[i], 2, 16, QChar('0')).toUpper());
        }
    }
}