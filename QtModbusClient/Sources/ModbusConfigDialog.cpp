#include "ModbusConfigDialog.h"
#include <qserialportinfo.h>
#include <qvariant.h>

//baud rate mapping
namespace {
	const QMap<qint32, QString> baudMap = {
		{QSerialPort::Baud1200,"1200"},
		{QSerialPort::Baud2400,"2400"},
		{QSerialPort::Baud4800,"4800"},
		{QSerialPort::Baud9600,"9600"},
		{QSerialPort::Baud19200,"19200"},
		{QSerialPort::Baud38400,"38400"},
		{QSerialPort::Baud57600,"57600"},
		{QSerialPort::Baud115200,"115200"}
	};
}

ModbusConfigDialog::ModbusConfigDialog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	initUI();
	setupConnections();
}

ModbusConfigDialog::~ModbusConfigDialog() {}

//Acquire parameter configuration
QString ModbusConfigDialog::getPort() const noexcept
{
	return ui.portComboBox->currentText();
}

qint32 ModbusConfigDialog::getBaudRate() const noexcept
{
	return ui.baudComboBox->currentData().toInt();
}

QSerialPort::DataBits ModbusConfigDialog::getDataBits() const noexcept
{
	return qvariant_cast<QSerialPort::DataBits>(ui.dataBitsComboBox->currentData());
}

QSerialPort::Parity ModbusConfigDialog::getParity() const noexcept
{
	return qvariant_cast<QSerialPort::Parity>(ui.parityComboBox->currentData());
}

QSerialPort::StopBits ModbusConfigDialog::getStopBits() const noexcept
{
	return qvariant_cast<QSerialPort::StopBits>(ui.stopBitsComboBox->currentData());
}

int ModbusConfigDialog::getSlaveID() const noexcept
{
	bool ok;
	const quint8 id = ui.slaveIDLineEdit->text().toUShort(&ok);
	return ok ? id : 1;
}

//Event handler for showing the dialog
void ModbusConfigDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
	refreshPorts();
}

// Slot function to apply the configuration and close the dialog
void ModbusConfigDialog::applyConfiguration()
{
	emit configurationApplied();
	accept();
}

// Initialize the user interface components
void ModbusConfigDialog::initUI()
{
	initComboBoxes();
	initRefreshPortsBtn();
	refreshPorts();
}

void ModbusConfigDialog::initComboBoxes()
{
	// Baud rates
	ui.baudComboBox->clear();
	for (auto it = baudMap.cbegin(); it != baudMap.cend(); ++it) {
		ui.baudComboBox->addItem(it.value(), it.key());
	}
	ui.baudComboBox->setCurrentIndex(3);

	// Data bits
	ui.dataBitsComboBox->clear();
	ui.dataBitsComboBox->addItem("5", QSerialPort::Data5);
	ui.dataBitsComboBox->addItem("6", QSerialPort::Data6);
	ui.dataBitsComboBox->addItem("7", QSerialPort::Data7);
	ui.dataBitsComboBox->addItem("8", QSerialPort::Data8);
	ui.dataBitsComboBox->setCurrentIndex(3);

	// Parity
	ui.parityComboBox->clear();
	ui.parityComboBox->addItem("None", QSerialPort::NoParity);
	ui.parityComboBox->addItem("Even", QSerialPort::EvenParity);
	ui.parityComboBox->addItem("Odd", QSerialPort::OddParity);
	ui.parityComboBox->setCurrentIndex(0);

	// Stop bits
	ui.stopBitsComboBox->clear();
	ui.stopBitsComboBox->addItem("1", QSerialPort::OneStop);
	ui.stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
	ui.stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
	ui.stopBitsComboBox->setCurrentIndex(0);

	// Slave ID
	ui.slaveIDLineEdit->setText("1");
	ui.slaveIDLineEdit->setValidator(new QIntValidator(1, 247, this));
	ui.slaveIDLineEdit->setInputMask("000");
	ui.slaveIDLineEdit->setMaxLength(3);
}

void ModbusConfigDialog::initRefreshPortsBtn()
{
	connect(ui.portRefreshBtn, &QPushButton::clicked, this, &ModbusConfigDialog::refreshPorts);
}

void ModbusConfigDialog::setupConnections()
{
	connect(ui.okBtn, &QPushButton::clicked, this, &ModbusConfigDialog::applyConfiguration);
	connect(ui.cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

// Refresh the list of available serial ports
void ModbusConfigDialog::refreshPorts()
{
	const QString currentPort = ui.portComboBox->currentText();
	ui.portComboBox->clear();

	const auto ports = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& info : ports) {
		ui.portComboBox->addItem(info.portName());
	}

	const int index = ui.portComboBox->findText(currentPort);
	if (index != -1) {
		ui.portComboBox->setCurrentIndex(index);
	}

	if (!currentPort.isEmpty()) {
		int index = ui.portComboBox->findText(currentPort);
		if (index >= 0) ui.portComboBox->setCurrentIndex(index);
	}
}