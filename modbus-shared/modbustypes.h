#ifndef MODBUSTYPES_H
#define MODBUSTYPES_H


#include <QList>


#define MODBUS_READFUNCTION         3 //READ REGISTERS
#define MODBUS_WRITEFUNCTION        0x10 //write REGISTERS
#define MODBUS_RELAYFUNCTION        5  //DUNNO


#define MODBUS_MAXIMUM_DATA_LEN     98 //0x62
#define MODBUS_MAXIMUM_DATA_LEN_PDU 253 //0x62
#define MODBUS_MAXIMUM_DATA_LEN_APU 253 + 3 //0x62


//modbus protocol codes
#define MODBUS_ERROR_HAS_NO_ERRORS      0
#define MODBUS_ERROR_ILLEGAL_FUNCT_CODE 1
#define MODBUS_ERROR_ILLEGAL_DATA_ADDR  2
#define MODBUS_ERROR_ILLEGAL_DATA_VALUE 3 //Value is not accepted by slave
#define MODBUS_ERROR_ILLEGAL_SLVEDEVFLR 4 //Unrecoverable error occurred while slave was attempting to perform requested action

#define MODBUS_ERROR_ACKNOWLEDGE        5 //I need more time to get the data
#define MODBUS_ERROR_SLAVE_DEVICE_BUSY  6 //please retry later


//my decoder error codes
#define MODBUS_DECODER_ERROR_HAS_NO_ERRORS      0
#define MODBUS_DECODER_ERROR_ILLEGAL_FUNCT_CODE 1
#define MODBUS_DECODER_ERROR_ILLEGAL_DATA_ADDR  2
#define MODBUS_DECODER_ERROR_ILLEGAL_LEN        4
#define MODBUS_DECODER_ERROR_ILLEGAL_CRC        5
#define MODBUS_DECODER_ERROR_ILLEGAL_DEVICEADDR 6

#define MODBUS_MIN_READLEN      5 //RTU
#define MODBUS_MIN_READLENTCP   8

#define MODBUS_MODE_UNK    0
#define MODBUS_MODE_RTU    1
#define MODBUS_MODE_TCP    2
//#define MODBUS_MODE_ASCII  3




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
    ModbusAnswerList listMeterMessage;//listMeterMess
    quint8 errCode;//from a control byte
    QString errstr;

    bool isValid;

    MessageValidatorResult() : errCode(0), isValid(false) {}
};


struct ModbusDecodedParams
{
    bool masterRead;//direction - true from slave to master, false master to slave

    quint8 devaddress;//Station address (RTU) or Unit identifier (TCP)

    quint8 functionCode;
    quint8 modbusmode;

    quint16 transactionid;
    quint16 protocolid;

    ModbusList byteslist;

    quint8 decodeErr;

    ModbusDecodedParams() : masterRead(true), devaddress(0xFF), functionCode(0xFF), modbusmode(MODBUS_MODE_UNK),
        transactionid(0), protocolid(0), decodeErr(MODBUS_DECODER_ERROR_HAS_NO_ERRORS) {}

    ModbusDecodedParams(bool masterRead, quint8 devaddress, quint8 functionCode) :
        masterRead(masterRead), devaddress(devaddress), functionCode(functionCode), modbusmode(MODBUS_MODE_RTU),
        transactionid(0), protocolid(0), decodeErr(MODBUS_DECODER_ERROR_HAS_NO_ERRORS) {}

    ModbusDecodedParams(bool masterRead, quint8 devaddress, quint8 functionCode, quint8 modbusmode, quint16 transactionid, quint16 protocolid) :
        masterRead(masterRead), devaddress(devaddress), functionCode(functionCode), modbusmode(modbusmode),
        transactionid(transactionid), protocolid(protocolid), decodeErr(MODBUS_DECODER_ERROR_HAS_NO_ERRORS) {}

};


struct ModbusRequestParams
{
    bool isValid;
    quint16 messageid;
    quint8 devaddr;

    ModbusAnswerList registers;

    ModbusRequestParams() : isValid(false), messageid(0), devaddr(0xFF)  {}

    ModbusRequestParams(quint16 messageid, quint8 devaddr, ModbusAnswerList registers) : isValid(true), messageid(messageid), devaddr(devaddr), registers(registers) {}
};


#endif // MODBUSTYPES_H
