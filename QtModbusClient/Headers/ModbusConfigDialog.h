#pragma once

#include <QDialog>
#include <qserialport.h>
#include "ui_ModbusConfigDialog.h"

class ModbusConfigDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ModbusConfigDialog(QWidget* parent = nullptr);
	~ModbusConfigDialog();

	// Getters for configuration parameters
	QString getPort() const noexcept;
	qint32 getBaudRate() const noexcept;
	QSerialPort::DataBits getDataBits() const noexcept;
	QSerialPort::Parity getParity() const noexcept;
	QSerialPort::StopBits getStopBits() const noexcept;
	int getSlaveID() const noexcept;

signals:
	void configurationApplied();

protected:
	void showEvent(QShowEvent* event) override;

private slots:
	//Slot functions for interface interactions
	void refreshPorts();
	void applyConfiguration();

private:
	Ui::ModbusConfigDialog ui;

	//initialization methods
	void initUI();
	void initComboBoxes();
	void initRefreshPortsBtn();
	void setupConnections();
};