#include "HRWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QEventLoop>
#include <QHeaderView>
#include <QApplication>
#include <QStandardItem>
#include <QIntValidator>
#include <QVector>

HRWidget::HRWidget(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    initTableModels();
    initUI();
    setupConnections();
}

HRWidget::~HRWidget()
{
    safeDeleteReply(m_singleHRReadReply);
    safeDeleteReply(m_singleHRWriteReply);
    safeDeleteReply(m_multipleHRReadReply);
    safeDeleteReply(m_multipleHRWriteReply);
}

void HRWidget::setModbusConnection(ModbusConnection* connection)
{
    m_modbusConnection = connection;
}

void HRWidget::initUI()
{
    ui.hrReadSingleAddressSpinBox->setRange(0, 65535);
    ui.hrReadSingleDataSpinBox->setReadOnly(true);
    ui.hrReadSingleDataSpinBox->setRange(0, 65535);

    ui.hrWriteSingleAddressSpinBox->setRange(0, 65535);
    QIntValidator* validator = new QIntValidator(0, 65535, this);
    ui.hrWriteSingleDataLineEdit->setValidator(validator);

    ui.hrReadMultipleAddressSpinBox->setRange(0, 65535);
    ui.hrReadMultipleCountSpinBox->setRange(1, 125);

    ui.hrWriteMultipleAddressSpinBox->setRange(0, 65535);
    ui.hrWriteMultipleCountSpinBox->setRange(1, 123);
    ui.hrWriteMultipleCountSpinBox->setValue(10);

    updateWriteTable();
}

void HRWidget::initTableModels()
{
    m_hrReadModel = new QStandardItemModel(this);
    m_hrReadModel->setColumnCount(3);
    m_hrReadModel->setHorizontalHeaderLabels({ tr("Address"), tr("Hex"), tr("Decimal") });
    ui.hrReadMultipleTableView->setModel(m_hrReadModel);
    ui.hrReadMultipleTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_hrWriteModel = new QStandardItemModel(this);
    m_hrWriteModel->setColumnCount(3);
    m_hrWriteModel->setHorizontalHeaderLabels({ tr("Address"), tr("Hex"), tr("Decimal") });
    ui.hrWriteMultipleTableView->setModel(m_hrWriteModel);
    ui.hrWriteMultipleTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(m_hrWriteModel, &QStandardItemModel::itemChanged,
        this, &HRWidget::handleWriteItemChanged);
}

void HRWidget::setupConnections()
{
    connect(ui.hrReadSingleBtn, &QPushButton::clicked, this, &HRWidget::onReadSingleHR);
    connect(ui.hrWriteSingleBtn, &QPushButton::clicked, this, &HRWidget::onWriteSingleHR);
    connect(ui.hrReadMultipleBtn, &QPushButton::clicked, this, &HRWidget::onReadMultipleHR);
    connect(ui.hrWriteMultipleBtn, &QPushButton::clicked, this, &HRWidget::onWriteMultipleHR);

    connect(ui.hrWriteMultipleAddressSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &HRWidget::updateWriteTable);
    connect(ui.hrWriteMultipleCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &HRWidget::updateWriteTable);
}

void HRWidget::safeDeleteReply(QModbusReply* reply)
{
    if (!reply) return;

    QObject::disconnect(reply, nullptr, this, nullptr);

    if (reply->isFinished()) {
        reply->deleteLater();
    } else {
        QEventLoop loop;
        connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        reply->deleteLater();
	}
}

void HRWidget::updateWriteTable()
{
    m_hrWriteModel->removeRows(0, m_hrWriteModel->rowCount());

    int startAddr = ui.hrWriteMultipleAddressSpinBox->value();
    int count = ui.hrWriteMultipleCountSpinBox->value();

    for (int i = 0; i < count; ++i) {
        QList<QStandardItem*> rowItems;
        int address = startAddr + i;

        QStandardItem* addrItem = new QStandardItem(QString::number(address));
        addrItem->setEditable(false);

        QStandardItem* hexItem = new QStandardItem("0x0000");
        hexItem->setEditable(true);
        hexItem->setData(address, Qt::UserRole);

        QStandardItem* decItem = new QStandardItem("0");
        decItem->setEditable(true);
        decItem->setData(address, Qt::UserRole);

        rowItems << addrItem << hexItem << decItem;
        m_hrWriteModel->appendRow(rowItems);
    }
}

void HRWidget::handleWriteItemChanged(QStandardItem* item)
{
    if (item->column() == 1 || item->column() == 2) {
        int row = item->row();
        bool ok;
        quint16 value;

        if (item->column() == 1) { 
            QString hexText = item->text();
            if (hexText.startsWith("0x", Qt::CaseInsensitive)) {
                hexText = hexText.mid(2);
            }
            value = hexText.toUShort(&ok, 16);

            if (ok) {
                QStandardItem* decItem = m_hrWriteModel->item(row, 2);
                if (decItem) {
                    decItem->setText(QString::number(value));
                }
            }
        }
        else if (item->column() == 2) {
            value = item->text().toUShort(&ok);

            if (ok) {
                QStandardItem* hexItem = m_hrWriteModel->item(row, 1);
                if (hexItem) {
                    hexItem->setText(QString("0x%1").arg(value, 4, 16, QChar('0')).toUpper());
                }
            }
        }
    }
}

void HRWidget::onReadSingleHR()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, "Error", "Not connected to any device");
        return;
    }

    safeDeleteReply(m_singleHRReadReply);
    ui.hrReadSingleBtn->setEnabled(false);

    const int address = ui.hrReadSingleAddressSpinBox->value();
    m_singleHRReadReply = m_modbusConnection->readRegister(
        ModbusConnection::HoldingRegisters, address, 1);

    if (m_singleHRReadReply) {
        connect(m_singleHRReadReply, &QModbusReply::finished,
            this, &HRWidget::handleSingleHRReadResult);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to send read request");
        ui.hrReadSingleBtn->setEnabled(true);
    }
}

// Process single holding register read result
void HRWidget::handleSingleHRReadResult()
{
    ui.hrReadSingleBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit result = reply->result();
        if (result.registerType() == QModbusDataUnit::HoldingRegisters && result.valueCount() > 0) {
            const quint16 value = result.value(0);
            ui.hrReadSingleDataSpinBox->setValue(value);
            qDebug() << "Single HR read successful - Address:"
                << ui.hrReadSingleAddressSpinBox->value() << "Value:" << value;
            QApplication::beep();
        }
        else {
            qDebug() << "Invalid HR data received";
            QMessageBox::warning(this, "Warning", "Invalid data received from device");
        }
    }
    else {
        qDebug() << "Single HR read error:" << reply->errorString();
        QMessageBox::critical(this, "Error", tr("Read failed: %1").arg(reply->errorString()));
    }

    safeDeleteReply(reply);
}

// Handle single holding register write request
void HRWidget::onWriteSingleHR()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, "Error", "Not connected to any device");
        return;
    }

    safeDeleteReply(m_singleHRWriteReply);
    ui.hrWriteSingleBtn->setEnabled(false);

    const int address = ui.hrWriteSingleAddressSpinBox->value();

    bool ok;
    quint16 value = ui.hrWriteSingleDataLineEdit->text().toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Input Error", "Invalid value entered");
        ui.hrWriteSingleBtn->setEnabled(true);
        return;
    }

    m_singleHRWriteReply = m_modbusConnection->writeSingleRegister(address, value);

    if (m_singleHRWriteReply) {
        connect(m_singleHRWriteReply, &QModbusReply::finished,
            this, &HRWidget::handleSingleHRWriteResult);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to send write request");
        ui.hrWriteSingleBtn->setEnabled(true);
    }
}

// Process single holding register write result
void HRWidget::handleSingleHRWriteResult()
{
    ui.hrWriteSingleBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    int writtenAddress = ui.hrWriteSingleAddressSpinBox->value();
    bool ok;
    quint16 writtenValue = ui.hrWriteSingleDataLineEdit->text().toUShort(&ok);

    if (reply->error() == QModbusDevice::NoError) {
        qDebug() << "Single HR write successful - Address:" << writtenAddress
            << "Value:" << writtenValue;
        QApplication::beep();
    }
    else {
        qDebug() << "Single HR write error:" << reply->errorString();
        QMessageBox::critical(this, "Error",
            tr("Write failed: %1").arg(reply->errorString()));
    }
    safeDeleteReply(reply);
}

// Handle multiple holding registers read request
void HRWidget::onReadMultipleHR()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, tr("Error"), tr("Not connected to any device"));
        return;
    }

    safeDeleteReply(m_multipleHRReadReply);
    ui.hrReadMultipleBtn->setEnabled(false);

    const int address = ui.hrReadMultipleAddressSpinBox->value();
    const int count = ui.hrReadMultipleCountSpinBox->value();

    m_multipleHRReadReply = m_modbusConnection->readRegister(
        ModbusConnection::HoldingRegisters, address, count);

    if (m_multipleHRReadReply) {
        connect(m_multipleHRReadReply, &QModbusReply::finished,
            this, &HRWidget::handleMultipleHRReadResult);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to send read request"));
        ui.hrReadMultipleBtn->setEnabled(true);
    }
}

// Process multiple holding registers read results
void HRWidget::handleMultipleHRReadResult()
{
    ui.hrReadMultipleBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit result = reply->result();
        if (result.registerType() == QModbusDataUnit::HoldingRegisters && result.valueCount() > 0) {
            m_hrReadModel->removeRows(0, m_hrReadModel->rowCount());

            const int startAddr = result.startAddress();
            const int count = result.valueCount();

            for (int i = 0; i < count; ++i) {
                QList<QStandardItem*> rowItems;
                const int addr = startAddr + i;
                const quint16 value = result.value(i);

                rowItems << new QStandardItem(QString::number(addr))
                    << new QStandardItem(QString("0x%1").arg(value, 4, 16, QChar('0')).toUpper())
                    << new QStandardItem(QString::number(value));

                for (auto item : rowItems) {
                    item->setEditable(false);
                }

                m_hrReadModel->appendRow(rowItems);
            }

            qDebug() << "Multiple HR read successful - Start address:"
                << startAddr << "Count:" << count;
            QApplication::beep();
        }
        else {
            qDebug() << "Invalid multiple HR data received";
            QMessageBox::warning(this, "Warning", "Invalid data received from device");
        }
    }
    else {
        qDebug() << "Multiple HR read error:" << reply->errorString();
        QMessageBox::critical(this, "Error", tr("Read failed: %1").arg(reply->errorString()));
    }

    safeDeleteReply(reply);
}

// Handle multiple holding registers write request
void HRWidget::onWriteMultipleHR()
{
    if (!m_modbusConnection || !m_modbusConnection->isConnected()) {
        QMessageBox::warning(this, tr("Error"), tr("Not connected to any device"));
        return;
    }

    safeDeleteReply(m_multipleHRWriteReply);
    ui.hrWriteMultipleBtn->setEnabled(false);

    const int startAddr = ui.hrWriteMultipleAddressSpinBox->value();
    const int count = ui.hrWriteMultipleCountSpinBox->value();

    QVector<quint16> values;
    for (int i = 0; i < count; ++i) {
        QStandardItem* decItem = m_hrWriteModel->item(i, 2);
        if (decItem) {
            bool ok;
            quint16 value = decItem->text().toUShort(&ok);
            values.append(ok ? value : 0);
        }
        else {
            values.append(0);
        }
    }

    m_multipleHRWriteReply = m_modbusConnection->writeMultipleRegisters(
        ModbusConnection::HoldingRegisters, startAddr, values);

    if (m_multipleHRWriteReply) {
        connect(m_multipleHRWriteReply, &QModbusReply::finished,
            this, &HRWidget::handleMultipleHRWriteResult);
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to send write request"));
        ui.hrWriteMultipleBtn->setEnabled(true);
    }
}

// Process multiple holding registers write result
void HRWidget::handleMultipleHRWriteResult()
{
    ui.hrWriteMultipleBtn->setEnabled(true);

    auto* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    int startAddr = ui.hrWriteMultipleAddressSpinBox->value();
    int count = ui.hrWriteMultipleCountSpinBox->value();

    if (reply->error() == QModbusDevice::NoError) {
        qDebug() << "Multiple HR write successful - Start address:"
            << startAddr << "Count:" << count;
        QApplication::beep();
        onReadMultipleHR();
    }
    else {
        qDebug() << "Multiple HR write error:" << reply->errorString();
        QMessageBox::critical(this, tr("Error"),
            tr("Write failed: %1").arg(reply->errorString()));
    }

    safeDeleteReply(reply);
}