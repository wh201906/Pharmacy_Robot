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
    currState = move_getState();
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
    bool res = true;
    if(move_forceRange)
    {
        if(axis == MOVE_AXIS_X)
        {
            if(step > 0 && currState.x + step > 0)
                step = -currState.x;
            else if(step < 0 && currState.x + step < MOVE_MAX_X)
                step = MOVE_MAX_X - currState.x;
        }
        if(axis == MOVE_AXIS_Y)
        {
            if(step < 0 && currState.y + step < 0)
                step = -currState.y;
            else if(step > 0 && currState.y + step > MOVE_MAX_Y)
                step = MOVE_MAX_Y - currState.y;
        }
        if(axis == MOVE_AXIS_Z)
        {
            if(step > 0 && currState.z + step > 0)
                step = -currState.z;
            else if(step < 0 && currState.z + step < MOVE_MAX_Z)
                step = MOVE_MAX_Z - currState.z;
        }
    }

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
    qDebug() << "write:" << targetData.toHex();
    res &= moveController->write(targetData) != -1;
//    if()
    return res;
}

bool ServoDriver::move_stop()
{
    bool res = moveController->write(QByteArray::fromHex("O5 00 07 01 F3")) != -1;
    currState = move_getState();
    stopUsed = true;
    return res;
}

void ServoDriver::move_setForceRange(bool st)
{
    move_forceRange = st;
}

bool ServoDriver::move_getForceRange()
{
    return move_forceRange;
}

ServoDriver::Move_State ServoDriver::move_getState()
{
    QByteArray rawState, tmpBytes;
    moveController->readAll();
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

void ServoDriver::move_goto(float x, float y, float speed)
{
    float dx = x - currState.x;
    float dy = y - currState.y;
    float dz = -currState.z;
    move_sendMotion(MOVE_AXIS_Z, dz, speed);
    move_sendMotion(MOVE_AXIS_X, dx, speed);
    move_sendMotion(MOVE_AXIS_Y, dy, speed);
}

bool ServoDriver::move_waitMotionFinished(int msec)
{
    int time = 0;
    while(move_getState().isRunning && time <= msec)
    {
        QThread::sleep(20);
        QApplication::processEvents();
        time += 20;
    }
    return time <= msec;
}

void ServoDriver::throwDrug()
{
    float speed = 30;
    move_sendMotion(MOVE_AXIS_Z, 0, speed);
    move_waitMotionFinished();
    rotate_sendMotion(ROTATE_SERVO_BOTTOM, 1020, 1000);
    move_sendMotion(MOVE_AXIS_X, 0, speed);
    move_sendMotion(MOVE_AXIS_Y, 678, speed);
    move_waitMotionFinished();
    rotate_stopSuck();
    QThread::sleep(500);
    rotate_initPos();
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

bool ServoDriver::rotate_initPos(bool withSucker)
{
    bool res = true;
    res &= rotate_sendMotion(ROTATE_SERVO_BOTTOM, ROTATE_INIT_BOTTOM);
    res &= rotate_sendMotion(ROTATE_SERVO_TOP, ROTATE_INIT_TOP);
    if(withSucker)
        res &= rotate_sendMotion(ROTATE_SERVO_SUCKER, 1500);
    return res;
}
