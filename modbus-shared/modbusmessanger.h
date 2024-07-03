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


    static qint32 getNumberFromTheList(const ModbusList &list, const int &startindx);


    static MODBUSDIVIDED_UINT16 getDivided(const quint16 &value);

    static void addDivided2thelist(ModbusList &list, const quint16 &value);

    static MODBUSDIVIDED_UINT16 getCrc16(const ModbusList &list);

    static QByteArray getModbusReadRegisterMessage(const quint8 &address, const quint16 &startaddress, const quint16 &len);

    static ModbusList getModbusReadRegisterMessageList(const quint8 &address, const quint16 &startaddress, const quint16 &len);


    static QByteArray getModbusReadRegisterMessageTCP(const quint8 &address, const quint16 &startaddress, const quint16 &len, const quint16 &transactionID, const qint16 &protocolId);

    static ModbusList getModbusReadRegisterMessageListTCP(const quint8 &address, const quint16 &startaddress, const quint16 &len, const quint16 &transactionID, const qint16 &protocolId);



    static bool isMessageLenGoodTCP(const ModbusList &list,  const qint32 &len, ModbusDecodedParams &messageparams);


    static bool isMessageListCrcGood(const ModbusList &list, const int &len);

    static bool isFunctionCodeGood(const ModbusList &list, quint8 &functioncode);

    static bool isFunctionCodeGoodTcp(const ModbusList &list, quint8 &functioncode);

    static bool isFunctionCodeGoodExt(const ModbusList &list, const quint8 &modbusmode, quint8 &functioncode, bool &haserror);


    static bool isThisMessageYoursLoop(const QByteArray &arr, ModbusDecodedParams &messageparams);//RTU


    static bool isThisMessageYoursLoopRTU(const QByteArray &arr, ModbusDecodedParams &messageparams);//RTU

    static bool isThisMessageYoursLoopTCP(const QByteArray &arr, ModbusDecodedParams &messageparams);

    static bool isThisMessageYoursTCP(const ModbusList &list, ModbusDecodedParams &messageparams);


    static bool isThisMessageYours(const ModbusList &list, ModbusDecodedParams &messageparams);

    static bool isReceivedMessageValid(const QByteArray &arr, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out);

    static bool isReceivedMessageListValid(const ModbusList &list, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out);

    static bool isReceivedMessageValidFastCheck(const ModbusList &list, const quint8 &address, quint8 &errorcode);

    static MessageValidatorResult messageIsValidExt(const QByteArray &arr, const quint8 &address);



    static bool isReceivedMessageValidTCP(const QByteArray &arr, const quint8 &address, const quint16 &transactionID, const qint16 &protocolId, quint8 &errorcode, ModbusAnswerList &out);

    static bool isReceivedMessageListValidTCP(const ModbusList &list, const quint8 &address, const quint16 &transactionID, const qint16 &protocolId, quint8 &errorcode, ModbusAnswerList &out);

    static bool isReceivedMessageValidFastCheckTCP(const ModbusList &list, const quint8 &address, const quint16 &transactionID, const qint16 &protocolId, quint8 &errorcode);

    static MessageValidatorResult messageIsValidExtTCP(const QByteArray &arr, const quint8 &address, const quint16 &transactionID, const qint16 &protocolId);



    static QList<qint32> convertTwoRegisters2oneValue(const ModbusAnswerList &l);
    static QList<qint32> convertTwoRegisters2oneValueBigEndian(const ModbusAnswerList &l);

    static QList<qint64> convertFourRegisters2oneValueBigEndian(const ModbusAnswerList &l);


    static QStringList convertTwoRegisters2oneValueStr(const ModbusAnswerList &l, const qreal &multiplier, const int &prec);
    static QStringList convertTwoRegisters2oneValueStrExt(const ModbusAnswerList &l, const qreal &multiplier, const int &prec, const bool &isBigEndian);

    static ModbusRequestParams decodeRequestParams(const QByteArray &readArr);

    static QByteArray getUTF8FromTheList(const ModbusAnswerList &l, const int &startIndx, const int &len);

};

#endif // MODBUSMESSANGER_H
