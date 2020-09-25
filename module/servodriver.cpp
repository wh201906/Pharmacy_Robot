#include "servodriver.h"

ServoDriver::ServoDriver(QObject *parent) : QObject(parent)
{
    moveThread = new QThread;
    moveController = new MoveController(moveThread);
    rotateController = new QSerialPort;
    servoState = new MoveController::Move_Servo_State;
    controllerState = new MoveController::Move_Controller_State;
    moveThread->start();
    qRegisterMetaType<MoveController::OpenMode>("MoveController::OpenMode");
    connect(this, &ServoDriver::move_setPortName, moveController, &MoveController::setPortNameSlot);
    connect(this, &ServoDriver::move_connectPort, moveController, &MoveController::openSlot);
    connect(this, &ServoDriver::move_disconnectPort, moveController, &MoveController::closeSlot);
    connect(this, &ServoDriver::move_write, moveController, &MoveController::writeSlot);
    connect(this, &ServoDriver::move_getControllerState, moveController, &MoveController::getControllerState);
    connect(this, &ServoDriver::move_connectPort, moveController, &MoveController::openSlot);
    connect(moveController, &MoveController::controllerError, this, &ServoDriver::move_onControllerErrorOccurred);
    connect(moveController, &MoveController::currState, this, &ServoDriver::move_onControllerStateFetched);
    connect(moveController, &MoveController::newServoState, this, &ServoDriver::move_onServoStateFetched);
    connect(moveController, &MoveController::MotionSent, this, &ServoDriver::move_onMotionSent);
}

void ServoDriver::move_onMotionSent()
{
    motionSentFlag = true;
}

void ServoDriver::move_onControllerErrorOccurred()
{

}

void ServoDriver::move_onServoStateFetched(MoveController::Move_Servo_State st)
{
    *servoState = st;
}

void ServoDriver::move_onControllerStateFetched(MoveController::Move_Controller_State st)
{
    *controllerState = st;
    if(st.isOpened)
    {
        qDebug() << QString("move servos connected at %1").arg(st.portName);
    }
    else
    {
        qDebug() << "move servos disconnected";
    }

}

void ServoDriver::move_connect(const QString& port)
{
    emit move_setPortName(port);
    emit move_connectPort(MoveController::ReadWrite);
}

void ServoDriver::move_disconnect()
{
    emit move_disconnectPort();
}

QString ServoDriver::move_getPort()
{
    if(controllerState->isOpened)
        return controllerState->portName;
    else
        return "";
}

void ServoDriver::move_sendMotion(Move_Axis axis, float step, float speed)
{
    if(move_forceRange)
    {
        if(axis == MOVE_AXIS_X)
        {
            if(step > 0 && servoState->x + step > 0)
                step = -servoState->x;
            else if(step < 0 && servoState->x + step < MOVE_MAX_X)
                step = MOVE_MAX_X - servoState->x;
        }
        if(axis == MOVE_AXIS_Y)
        {
            if(step < 0 && servoState->y + step < 0)
                step = -servoState->y;
            else if(step > 0 && servoState->y + step > MOVE_MAX_Y)
                step = MOVE_MAX_Y - servoState->y;
        }
        if(axis == MOVE_AXIS_Z)
        {
            if(step > 0 && servoState->z + step > 0)
                step = -servoState->z;
            else if(step < 0 && servoState->z + step < MOVE_MAX_Z)
                step = MOVE_MAX_Z - servoState->z;
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
    motionSentFlag = false;
    emit move_write(targetData);
}

void ServoDriver::move_stop()
{
    emit move_write(QByteArray::fromHex("O5 00 07 01 F3"));
}

void ServoDriver::move_setForceRange(bool st)
{
    move_forceRange = st;
}

bool ServoDriver::move_getForceRange()
{
    return move_forceRange;
}

MoveController::Move_Servo_State ServoDriver::move_getServoState()
{
    return *servoState;
}

void ServoDriver::move_goto(float x, float y, float speed)
{
    float dx = x - servoState->x;
    float dy = y - servoState->y;
    float dz = -servoState->z;
    move_sendMotion(MOVE_AXIS_Z, dz, speed);
    qDebug() << move_waitMotionSent();
    qDebug() << move_waitMotionFinished();
    move_sendMotion(MOVE_AXIS_X, dx, speed);
    move_sendMotion(MOVE_AXIS_Y, dy, speed);
}

bool ServoDriver::move_waitMotionFinished(int msec)
{
    QTime targetTime = QTime::currentTime().addMSecs(300);
    QTime currTime = QTime::currentTime();
    while(currTime < targetTime)
    {
        QApplication::processEvents();
        currTime = QTime::currentTime();
    }
    targetTime = QTime::currentTime().addMSecs(msec);
    currTime = QTime::currentTime();
    while(servoState->isRunning && currTime < targetTime)
    {
        QApplication::processEvents();
        currTime = QTime::currentTime();
    }
    return currTime < targetTime;
}

bool ServoDriver::move_waitMotionSent(int msec)
{
    QTime targetTime = QTime::currentTime().addMSecs(300);
    QTime currTime = QTime::currentTime();
    while(currTime < targetTime)
    {
        QApplication::processEvents();
        currTime = QTime::currentTime();
    }
    targetTime = QTime::currentTime().addMSecs(msec);
    currTime = QTime::currentTime();
    while(!motionSentFlag && currTime < targetTime)
    {
        QApplication::processEvents();
        currTime = QTime::currentTime();
    }
    motionSentFlag = false;
    return currTime < targetTime;
}

void ServoDriver::throwDrug()
{
    float dx = -servoState->x;
    float dy = 678 - servoState->y;
    float dz = -servoState->z;
    float speed = 30;
    move_sendMotion(MOVE_AXIS_Z, dz, speed);
    move_waitMotionSent();
    move_waitMotionFinished();
    rotate_sendMotion(ROTATE_SERVO_BOTTOM, 1020, 1000);
    move_sendMotion(MOVE_AXIS_X, dx, speed);
    move_waitMotionSent();
    move_sendMotion(MOVE_AXIS_Y, dy, speed);
    move_waitMotionSent();
    move_waitMotionFinished();
    rotate_stopSuck();
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
