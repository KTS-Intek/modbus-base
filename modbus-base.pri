# Version=0.0.1
# Dependencies
# type-converter
#
QT       += core

INCLUDEPATH  += $$PWD\
                $$PWD/../../defines/defines

SOURCES += \
    $$PWD/modbus-shared/modbusmessanger.cpp

HEADERS += \
    $$PWD/modbus-shared/modbusmessanger.h \
    $$PWD/modbus-shared/modbustypes.h

