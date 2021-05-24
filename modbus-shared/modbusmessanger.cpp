#include "modbusmessanger.h"


///[!] type-converter
#include "src/base/convertatype.h"
#include "src/base/prettyvalues.h"

#include <QDebug>


//-------------------------------------------------------------------------

ModbusMessanger::ModbusMessanger(QObject *parent) : QObject(parent)
{

}

//-------------------------------------------------------------------------

qint32 ModbusMessanger::getNumberFromTheList(const ModbusList &list, const int &startindx)
{
//    const qint32 leftp = list.at(startindx);
//    const qint32 rightp = list.at(startindx+1) << 8;

//    const qint32 outp = (leftp | rightp);

//    return outp; it doesn't work
    return ((qint32)list.at(startindx) << 8 | list.at(startindx+1));
}

//-------------------------------------------------------------------------

MODBUSDIVIDED_UINT16 ModbusMessanger::getDivided(const quint16 &value)
{
    //    const QString
        MODBUSDIVIDED_UINT16 r;
        r.lowbyte = value & 0xFF;
        r.hightbyte = (value >> 8);
        return r;
}

//-------------------------------------------------------------------------

void ModbusMessanger::addDivided2thelist(ModbusList &list, const quint16 &value)
{
    const MODBUSDIVIDED_UINT16 r = getDivided(value);
    list.append(r.hightbyte);
    list.append(r.lowbyte);
}

//-------------------------------------------------------------------------

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

//-------------------------------------------------------------------------

QByteArray ModbusMessanger::getModbusReadRegisterMessage(const quint8 &address, const quint16 &startaddress, const quint16 &len)
{
    const ModbusList list = getModbusReadRegisterMessageList(address, startaddress, len);
    return  ConvertAtype::uint8list2array(list, 0, list.length());//is ready to send message
}

//-------------------------------------------------------------------------

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

//-------------------------------------------------------------------------

bool ModbusMessanger::isMessageLenGoodTCP(const ModbusList &list, const qint32 &len, ModbusDecodedParams &messageparams)
{
    //0-1b 2 bytes transactionid;
    //2-3b 2 bytes protocolid
    //4-5b 2 bytes length field
    //6b   1 byte unit id, dev id

    if(len < MODBUS_MIN_READLENTCP && len > MODBUS_MAXIMUM_DATA_LEN_APU)
        return false;

    const qint32 v = getNumberFromTheList(list, 4) + 6;

    if(v != len)
        return false;


    messageparams.modbusmode = MODBUS_MODE_TCP;

    messageparams.transactionid = getNumberFromTheList(list, 0);
    messageparams.protocolid = getNumberFromTheList(list, 1);

    return true;



}

//-------------------------------------------------------------------------

bool ModbusMessanger::isMessageListCrcGood(const ModbusList &list, const int &len)
{
    //check crc
    const MODBUSDIVIDED_UINT16 messagecrc = getCrc16(list.mid(0, len - 2));

    return (messagecrc.hightbyte == list.at(len-2) && messagecrc.lowbyte == list.at(len-1));
}

//-------------------------------------------------------------------------

bool ModbusMessanger::isFunctionCodeGood(const ModbusList &list, quint8 &functioncode)
{
    bool haserr;
    return isFunctionCodeGoodExt(list, MODBUS_MODE_RTU, functioncode, haserr);
}

//-------------------------------------------------------------------------

bool ModbusMessanger::isFunctionCodeGoodTcp(const ModbusList &list, quint8 &functioncode)
{
    bool haserr;
    return isFunctionCodeGoodExt(list, MODBUS_MODE_TCP, functioncode, haserr);
}

//-------------------------------------------------------------------------

bool ModbusMessanger::isFunctionCodeGoodExt(const ModbusList &list, const quint8 &modbusmode, quint8 &functioncode, bool &haserror)
{

    const int findex = (modbusmode == MODBUS_MODE_TCP) ? 7 : 1;


    QBitArray bitarr = ConvertAtype::uint8ToBitArray(list.at(findex))    ;
    haserror = bitarr.at(7);
    if(haserror)
        bitarr.setBit(7, false);
    functioncode = ConvertAtype::bitArrToByteArr(bitarr, false).at(0);

    return (functioncode == MODBUS_READFUNCTION || functioncode == MODBUS_WRITEFUNCTION);
}

//-------------------------------------------------------------------------

bool ModbusMessanger::isThisMessageYoursLoop(const QByteArray &arr, ModbusDecodedParams &messageparams)
{
    //RTU
    const ModbusList list = ConvertAtype::convertArray2uint8list(arr);

    for(int i = 0, imax = list.length() - 5; i < imax; i++){
        if(isThisMessageYours(list.mid(i), messageparams))
            return true;
    }
    return false;
}

//-------------------------------------------------------------------------

bool ModbusMessanger::isThisMessageYoursLoopRTU(const QByteArray &arr, ModbusDecodedParams &messageparams)
{
    return isThisMessageYoursLoop(arr, messageparams);

}

//-------------------------------------------------------------------------

bool ModbusMessanger::isThisMessageYoursLoopTCP(const QByteArray &arr, ModbusDecodedParams &messageparams)
{

    //TCP
    const ModbusList list = ConvertAtype::convertArray2uint8list(arr);

    for(int i = 0, imax = list.length() - 5; i < imax; i++){
        if(isThisMessageYoursTCP(list.mid(i), messageparams))
            return true;
    }
    return false;

}
//-------------------------------------------------------------------------
bool ModbusMessanger::isThisMessageYoursTCP(const ModbusList &list, ModbusDecodedParams &messageparams)
{

    //direction: to SPM33,
    //TCP mode
    //Unit identifier - devaddress, see Wikipedia Modbus

    messageparams.devaddress = messageparams.functionCode = 0xFF;
    const qint32 len = list.length(); //check len

    bool result = true;

    if(len < MODBUS_MIN_READLENTCP){
        messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_LEN;
        result = false;
    }

    if(result && !isFunctionCodeGoodTcp(list, messageparams.functionCode)){
        messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_FUNCT_CODE;
        result = false;
    }


    if(result && !isMessageLenGoodTCP(list, len, messageparams)){
        messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_LEN;
        result = false;
    }

    if( len > 6){
        messageparams.devaddress = list.at(6);
        if(result && (messageparams.devaddress < 1 || messageparams.devaddress > 247)){
            result = false;
            messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_DEVICEADDR;
        }
    }

    messageparams.byteslist = list;
    return result;


}


//-------------------------------------------------------------------------

bool ModbusMessanger::isThisMessageYours(const ModbusList &list, ModbusDecodedParams &messageparams)
{
    //direction: to SPM33,
    //RTU mode

    messageparams.devaddress = messageparams.functionCode = 0xFF;
    const int len = list.length(); //check len

    bool result = true;

    if(len < MODBUS_MIN_READLEN){
        messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_LEN;
        result = false;
    }

    if(result && !isMessageListCrcGood(list, len)){
        messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_CRC;
        result = false;
    }

    if(result && !isFunctionCodeGood(list, messageparams.functionCode)){
        messageparams.decodeErr = MODBUS_ERROR_ILLEGAL_FUNCT_CODE;
        result = false;
    }

    if(!list.isEmpty()){
        messageparams.devaddress = list.at(0);

        if(result && (messageparams.devaddress < 1 || messageparams.devaddress > 247)){
            result = false;
            messageparams.decodeErr = MODBUS_DECODER_ERROR_ILLEGAL_DEVICEADDR;
        }
    }
    messageparams.byteslist = list;
    messageparams.modbusmode = MODBUS_MODE_RTU;
    return result;
}

//-------------------------------------------------------------------------

bool ModbusMessanger::isReceivedMessageValid(const QByteArray &arr, const quint8 &address, quint8 &errorcode, ModbusAnswerList &out)
{
    return isReceivedMessageListValid(ConvertAtype::convertArray2uint8list(arr), address, errorcode, out);
}

//-------------------------------------------------------------------------

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

//-------------------------------------------------------------------------

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
    quint8 functionCode;
    if(!isFunctionCodeGoodExt(list, MODBUS_MODE_RTU, functionCode, haserror))
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

//-------------------------------------------------------------------------

MessageValidatorResult ModbusMessanger::messageIsValidExt(const QByteArray &arr, const quint8 &address)
{
    MessageValidatorResult decoderesult;
    decoderesult.isValid = isReceivedMessageValid(arr, address, decoderesult.errCode, decoderesult.listMeterMess);
    return decoderesult;
}

//-------------------------------------------------------------------------

QList<qint32> ModbusMessanger::convertTwoRegisters2oneValue(const ModbusAnswerList &l)
{
    QList<qint32> out; // AB and CD  to CDAB
    for(int i = 0, imax = l.size(); i < imax; i += 2){
        const qint32 v = ((qint32)l.at(i+1) << 16 | l.at(i));
        out.append(v);
    }
    return out;
}

//-------------------------------------------------------------------------

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


//-------------------------------------------------------------------------


