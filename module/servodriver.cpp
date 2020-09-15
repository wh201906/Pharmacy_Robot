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
    int checkSum = 0;
    int checkByte = 0;
    targetData += QByteArray::fromHex("0E 00 04 01");
    targetData += QByteArray::fromRawData((char*)&axis, 1);
    targetData += QByteArray::fromRawData((char*)&step, 4);
    targetData += QByteArray::fromRawData((char*)&speed, 4);
    qDebug() << targetData;
    for(quint8 byte : targetData)
    {
        checkSum += byte;
        checkSum %= 0xFF;
    }
    for(int i = 2; i <= 5; i++)
    {
        checkByte = 0xff - checkSum + i;
        checkByte %= 0xFF;
        qDebug() << (targetData + QByteArray::fromRawData((char*)&checkByte, 1)).toHex();
    }

    return true;
}
