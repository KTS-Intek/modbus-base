#include "modbusmessanger.h"


///[!] type-converter
#include "src/base/convertatype.h"
#include "src/base/prettyvalues.h"

#include <QDebug>


ModbusMessanger::ModbusMessanger(QObject *parent) : QObject(parent)
{

}

MODBUSDIVIDED_UINT16 ModbusMessanger::getDivided(const quint16 &value)
{
    //    const QString
        MODBUSDIVIDED_UINT16 r;
        r.lowbyte = value & 0xFF;
        r.hightbyte = (value >> 8);
        return r;
}

void ModbusMessanger::addDivided2thelist(ModbusList &list, const quint16 &value)
{
    const MODBUSDIVIDED_UINT16 r = getDivided(value);
    list.append(r.hightbyte);
    list.append(r.lowbyte);
}

MODBUSDIVIDED_UINT16 ModbusMessanger::getCrc16(const ModbusList &list)
{
    quint16 crc = 0xFFFF;

    for(int pos = 0, posMax = list.size(); pos < posMax; pos++){
        crc ^= (quint16)list.at(pos); // XOR byte into least sig. byte of crc

        for(int i = 8; i != 0; i--){    // Loop over each bit
            if((crc & 0x0001) != 0){    // If the LSB is set
                crc >>= 1;              // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }else{                      // Else LSB is not set
                crc >>= 1;              // Just shift right
            }
        }
    }

    const MODBUSDIVIDED_UINT16 r = getDivided(crc);

    return MODBUSDIVIDED_UINT16(r.lowbyte, r.hightbyte); //crc is inverted
}

QByteArray ModbusMessanger::getModbusReadRegisterMessage(const quint8 &address, const quint16 &startaddress, const quint16 &len)
{
    const ModbusList list = getModbusReadRegisterMessageList(address, startaddress, len);
    return  ConvertAtype::uint8list2array(list, 0, list.length());//is ready to send message
}

ModbusList ModbusMessanger::getModbusReadRegisterMessageList(const quint8 &address, const quint16 &startaddress, const quint16 &len)
{
    /*  01   03   00 00   00 0D   84 0F
     *  <addr>
     *  <function>
     *  <start address>
     *  <len>
     *  <crc>
     */

    ModbusList list;
    list.append(address);
    list.append(MODBUS_READFUNCTION);
    addDivided2thelist(list, startaddress);
    addDivided2thelist(list, len);

    const MODBUSDIVIDED_UINT16 crc = getCrc16(list);
    list.append(crc.hightbyte);
    list.append(crc.lowbyte);
    return list;
    //    return  ConvertAtype::uint8list2array(list, 0, list.length());//is ready to send message
}

bool ModbusMessanger::isMessageListCrcGood(const ModbusList &list, const int &len)
{
    //check crc
    const MODBUSDIVIDED_UINT16 messagecrc = getCrc16(list.mid(0, len - 2));

    return (messagecrc.hightbyte == list.at(len-2) && messagecrc.lowbyte == list.at(len-1));
}

bool ModbusMessanger::isFunctionCodeGood(const ModbusList &list)
{
    bool haserr;
    return isFunctionCodeGoodExt(list, haserr);
}

bool ModbusMessanger::isFunctionCodeGoodExt(const ModbusList &list, bool &haserror)
{
    QBitArray bitarr = ConvertAtype::uint8ToBitArray(list.at(1))    ;
    haserror = bitarr.at(7);
    if(haserror)
        bitarr.setBit(7, false);
    const quint8 functioncode = ConvertAtype::bitArrToByteArr(bitarr, false).at(0);

    return (functioncode == MODBUS_READFUNCTION || functioncode == MODBUS_WRITEFUNCTION);
}

bool ModbusMessanger::isThisMessageYoursLoop(const QByteArray &arr, quint8 &devaddress)
{
    for(int i = 0, imax = arr.length() - 5; i < imax; i++){
        if(isThisMessageYours(arr.mid(i), devaddress))
            return true;
    }
    return false;
}

bool ModbusMessanger::isThisMessageYours(const QByteArray &arr, quint8 &devaddress)
{
    //direction: to SPM33,

    devaddress = 0xFF;
    const ModbusList list = ConvertAtype::convertArray2uint8list(arr);
    const int len = list.length(); //check len
    if(len < MODBUS_MIN_READLEN)
        return false;

    if(!isMessageListCrcGood(list, len))
        return false;

    if(!isFunctionCodeGood(list))
        return false;


    devaddress = list.at(0);
    return true;
}

bool ModbusMessanger::isReceivedMessageValid(const QByteArray &arr, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out)
{
    return isReceivedMessageListValid(ConvertAtype::convertArray2uint8list(arr), address, errorcode, out);
}

bool ModbusMessanger::isReceivedMessageListValid(const ModbusList &list, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out)
{
    if(!isReceivedMessageValidFastCheck(list, address, errorcode))
        return false;

    if(errorcode != MODBUS_ERROR_HAS_NO_ERRORS)
        return true;//has an error


    const int messsagelen = list.at(2);
    const int len = list.length(); //check len

    for(int i = 3, j = 0, imax = len - 2; i < imax && j < messsagelen; i += 2, j += 2){
        const quint16 v = ((quint16)list.at(i) << 8 | list.at(i+1));
        out.append(v);
    }


    return true;
}

bool ModbusMessanger::isReceivedMessageValidFastCheck(const ModbusList &list, const quint8 &address, quint8 &errorcode)
{
    //direction: from SPM33
    errorcode = MODBUS_ERROR_HAS_NO_ERRORS;

    //01 83 02 C0 F1 - error, the minimu length
    const int len = list.length(); //check len
    if(len < MODBUS_MIN_READLEN)
        return false;

    if(!isMessageListCrcGood(list, len))
        return false;

    bool haserror;
    if(!isFunctionCodeGoodExt(list, haserror))
        return false;

    if(address < 0xFF && address != list.at(0))
        return false;//is not my address


    if(haserror){
        errorcode = list.at(2);//error code
        return true;
    }
    const int messsagelen = list.at(2);
    if((messsagelen+5) != len)
        return false; //message to body ratio is 5 bytes

    return true;
}

MessageValidatorResult ModbusMessanger::messageIsValidExt(const QByteArray &arr, const quint8 &address)
{
    MessageValidatorResult decoderesult;
    decoderesult.isValid = isReceivedMessageValid(arr, address, decoderesult.errCode, decoderesult.listMeterMess);
    return decoderesult;
}

QList<qint32> ModbusMessanger::convertTwoRegisters2oneValue(const ModbusAnswerList &l)
{
    QList<qint32> out; // AB and CD  to CDAB
    for(int i = 0, imax = l.size(); i < imax; i += 2){
        const qint32 v = ((qint32)l.at(i+1) << 16 | l.at(i));
        out.append(v);
    }
    return out;
}

QStringList ModbusMessanger::convertTwoRegisters2oneValueStr(const ModbusAnswerList &l, const qreal &multiplier, const int &prec)
{
    const QList<qint32> values = convertTwoRegisters2oneValue(l);

    QStringList out;
    for(int i = 0, imax = values.size(); i < imax; i++){
        const qreal r = qreal(values.at(i)) * multiplier;
        out.append(PrettyValues::prettyNumber(r, prec, prec));
    }
    return out;
}



