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

void ServoDriver::move_disconnect()
{
    moveController->close();
}

QString ServoDriver::move_getPort()
{
    if(moveController->isOpen())
        return moveController->portName();
    else
        return "";
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

ServoDriver::Move_State ServoDriver::move_getState()
{
    QByteArray rawState, tmpBytes;
    moveController->clear();
    if(moveController->write(QByteArray::fromHex("O5 00 03 01 F7")) == -1)
        return Move_State();
    if(!moveController->waitForBytesWritten(500))
        return Move_State();
    if(!moveController->waitForReadyRead(500))
        return Move_State();
    rawState = moveController->readAll();
    if(rawState.size() != 62)
    {
        moveController->waitForReadyRead(500);
        rawState += moveController->readAll();
    }
    if(rawState.size() != 62)
        return Move_State();
    const qint32* rawX = (const qint32*)((const void*)(rawState.data() + 13));
    const qint32* rawY = (const qint32*)((const void*)(rawState.data() + 17));
    const qint32* rawZ = (const qint32*)((const void*)(rawState.data() + 21));
    qDebug() << *rawX << *rawY << *rawZ;
    return Move_State(rawState.at(5) == 0x04, *rawX / 100000.0, *rawY / 100000.0, *rawZ / 100000.0);
}

bool ServoDriver::rotate_connect(const QString& port)
{
    rotateController->setPortName(port);
    rotateController->setBaudRate(115200);
    rotateController->setDataBits(QSerialPort::Data8);
    rotateController->setStopBits(QSerialPort::OneStop);
    rotateController->setParity(QSerialPort::NoParity);

    return rotateController->open(QSerialPort::ReadWrite);
}

void ServoDriver::rotate_disconnect()
{
    rotateController->close();
}

QString ServoDriver::rotate_getPort()
{
    if(rotateController->isOpen())
        return rotateController->portName();
    else
        return "";
}

bool ServoDriver::rotate_suck()
{
//    return rotateController->write("#3P2500T100\r\n") != -1;
    return rotate_sendMotion(ROTATE_SERVO_SUCKER, 2500, 5000);
}

bool ServoDriver::rotate_stopSuck()
{
    return rotate_sendMotion(ROTATE_SERVO_SUCKER, 1500, 5000);
}

bool ServoDriver::rotate_sendMotion(Rotate_Servo servo, int pos, int speed)
{
    QString cmd = "#" + QString::number(servo) + "P" + QString::number(pos) + "S" + QString::number(speed) + "\r\n";
    return rotateController->write(cmd.toLocal8Bit());
}
