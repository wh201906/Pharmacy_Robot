#include "rfid.h"

RFID::RFID(QObject *parent) : QObject(parent)
{
    port = new QSerialPort;
}

void RFID::init()
{
    infoList = QSerialPortInfo::availablePorts();
    for(QSerialPortInfo info : infoList)
    {
        if(info.vendorIdentifier() == 6790 && info.productIdentifier() == 29987)
        {
            port->setPort(info);
            break;
        }
    }
    port->setBaudRate(115200);
    port->setDataBits(QSerialPort::Data8);
    port->setStopBits(QSerialPort::OneStop);
    port->setParity(QSerialPort::NoParity);
    port->open(QSerialPort::ReadWrite);
}

QString RFID::get14aUID()
{
    int waitTime = 100;
    QByteArray wakeupCmd = QByteArray::fromHex("55550000000000000000000000000000ff03fdd414011700");
    QByteArray getUIDCmd = QByteArray::fromHex("0000ff04fcd44a0200e000");

    init();
    port->write(wakeupCmd);
    port->waitForBytesWritten(waitTime); //Essential!!! Only using waitForReadyRead() will cause problems.
    port->waitForReadyRead(waitTime);
    port->readAll();
    port->write(getUIDCmd);
    port->waitForBytesWritten(waitTime);
    port->waitForReadyRead(waitTime);
    port->readAll();
    port->waitForReadyRead(waitTime);
    QString result = port->readAll().toHex();
    port->close();
    if(result != "")
        return result.right(12).left(8);
    else
        return "";
}
