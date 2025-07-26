#pragma once

#include <QObject>

class modbusmanager : public QObject
{
	Q_OBJECT

public:
	modbusmanager(QObject *parent);
	~modbusmanager();
};
