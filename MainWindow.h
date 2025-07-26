#pragma once

#include <QWidget>
#include "ui_MainWindow.h"
#include "modbusmanager.h"

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow ui;
	ModbusManager* m_modbusManager;
};
