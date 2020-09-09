#ifndef MODBUSMESSANGER_H
#define MODBUSMESSANGER_H

#include <QObject>

class ModbusMessanger : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMessanger(QObject *parent = nullptr);

signals:

};

#endif // MODBUSMESSANGER_H
