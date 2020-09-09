#ifndef MODBUSMESSANGER_H
#define MODBUSMESSANGER_H

#include <QObject>
#include <QVariantHash>
#include <QBitArray>


#include "modbustypes.h"


class ModbusMessanger : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMessanger(QObject *parent = nullptr);

    static MODBUSDIVIDED_UINT16 getDivided(const quint16 &value);

    static void addDivided2thelist(ModbusList &list, const quint16 &value);

    static MODBUSDIVIDED_UINT16 getCrc16(const ModbusList &list);

    static QByteArray getModbusReadRegisterMessage(const quint8 &address, const quint16 &startaddress, const quint16 &len);

    static ModbusList getModbusReadRegisterMessageList(const quint8 &address, const quint16 &startaddress, const quint16 &len);



    static bool isMessageListCrcGood(const ModbusList &list, const int &len);

    static bool isFunctionCodeGood(const ModbusList &list);
    static bool isFunctionCodeGoodExt(const ModbusList &list, bool &haserror);

    static bool isThisMessageYoursLoop(const QByteArray &arr, quint8 &devaddress);

    static bool isThisMessageYours(const QByteArray &arr, quint8 &devaddress);

    static bool isReceivedMessageValid(const QByteArray &arr, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out);

    static bool isReceivedMessageListValid(const ModbusList &list, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out);

    static bool isReceivedMessageValidFastCheck(const ModbusList &list, const quint8 &address, quint8 &errorcode);

    static MessageValidatorResult messageIsValidExt(const QByteArray &arr, const quint8 &address);


    static QList<qint32> convertTwoRegisters2oneValue(const ModbusAnswerList &l);

    static QStringList convertTwoRegisters2oneValueStr(const ModbusAnswerList &l, const qreal &multiplier, const int &prec);

};

#endif // MODBUSMESSANGER_H
