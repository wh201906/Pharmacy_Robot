#include "servodriver.h"

ServoDriver::ServoDriver(QObject *parent) : QObject(parent)
{
    moveController = new QSerialPort;
    rotateController = new QSerialPort;
}

bool ServoDriver::move_connect(const QString& port)
{
    moveController->setPortName(port);
    moveController->setBaudRate(115200);
    moveController->setDataBits(QSerialPort::Data8);
    moveController->setStopBits(QSerialPort::OneStop);
    moveController->setParity(QSerialPort::NoParity);

    return moveController->open(QSerialPort::ReadWrite);
}

bool ServoDriver::move_sendMotion(Move_Axis axis, float step, float speed)
{
    QByteArray targetData;
    quint8 checkSum = 0;
    quint16 checkByte = 0;
    targetData += QByteArray::fromHex("0E 00 04 01");
    targetData += QByteArray::fromRawData((char*)&axis, 1);
    targetData += QByteArray::fromRawData((char*)&step, 4);
    targetData += QByteArray::fromRawData((char*)&speed, 4);
    qDebug() << targetData;
    for(quint8 byte : targetData)
    {
        checkSum += byte;
    }
    checkByte = 256 - checkSum;
    checkByte &= 0xFF;
    targetData += QByteArray::fromRawData((char*)&checkByte, 1);
    return moveController->write(targetData) != -1;
}

bool ServoDriver::move_stop()
{
    return moveController->write(QByteArray::fromHex("O5 00 07 01 F3")) != -1;
}
