#ifndef MODBUSTYPES_H
#define MODBUSTYPES_H


#include <QList>

typedef QList<quint8> ModbusList;

typedef QList<quint16> ModbusAnswerList;

struct MODBUSDIVIDED_UINT16
{
    quint8 hightbyte;
    quint8 lowbyte;
    MODBUSDIVIDED_UINT16() : hightbyte(0), lowbyte(0) {}
    MODBUSDIVIDED_UINT16(const quint8 &hightbyte, const quint8 &lowbyte) : hightbyte(hightbyte), lowbyte(lowbyte) {}
};


struct MessageValidatorResult
{
    ModbusAnswerList listMeterMess;
    quint8 errCode;//from a control byte
    QString errstr;

    bool isValid;

    MessageValidatorResult() : errCode(0), isValid(false) {}
};



#define MODBUS_READFUNCTION         3 //READ REGISTERS
#define MODBUS_WRITEFUNCTION        0x10 //write REGISTERS
#define MODBUS_RELAYFUNCTION        5  //DUNNO


#define MODBUS_MAXIMUM_DATA_LEN     98 //0x62



#define MODBUS_ERROR_HAS_NO_ERRORS      0
#define MODBUS_ERROR_ILLEGAL_FUNCT_CODE 1
#define MODBUS_ERROR_ILLEGAL_DATA_ADDR  2
#define MODBUS_ERROR_ILLEGAL_DATA_VALUE 3

#define MODBUS_MIN_READLEN    5



#endif // MODBUSTYPES_H
