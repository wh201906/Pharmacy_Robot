#include "movecontroller.h"

MoveController::MoveController(QThread* targetThread, QObject* parent) : QSerialPort(parent)
{
    moveToThread(targetThread);
    stateTimer = new QTimer();
    stateTimer->moveToThread(targetThread);
    stateTimer->setInterval(75);
    buffer = new QByteArray;
    state = new Move_Controller_State;
    qRegisterMetaType<Move_Servo_State>("Move_Servo_State");
    qRegisterMetaType<Move_Controller_State>("Move_Controller_State");
    setBaudRate(115200);
    setDataBits(QSerialPort::Data8);
    setStopBits(QSerialPort::OneStop);
    setParity(QSerialPort::NoParity);
    connect(stateTimer, &QTimer::timeout, this, &MoveController::onTimeout);
    connect(this, &MoveController::readyRead, this, &MoveController::onReadyRead);
}

void MoveController::openSlot(MoveController::OpenMode mode)
{
    if(isOpen() || open(mode))
    {
        stateTimer->start();
        state->isOpened = true;
        state->portName = portName();
    }
    else
    {
        state->isOpened = false;
        emit controllerError();
    }
    emit currState(*state);
}

void MoveController::closeSlot()
{
    stateTimer->stop();
    state->isOpened = false;
    close();
    emit currState(*state);
}

void MoveController::onTimeout()
{
    if(!state->isOpened)
    {
        stateTimer->stop();
    }
    write(QByteArray::fromHex("O5 00 03 01 F7"));
}

void MoveController::onReadyRead()
{
    buffer->append(readAll());
    if(buffer->at(0) != 0x3E)
        qDebug() << "raw:" << buffer->toHex();
    if(buffer->contains(0x65))
    {
        if(!buffer->contains(QByteArray::fromHex("65 72 72 6F 72")) && readerState < 2)
        {
            readerState++;
            return;
        }
        else if(buffer->contains(QByteArray::fromHex("65 72 72 6F 72")))
            emit controllerError();
    }
    else if(buffer->contains(0x06))
    {
        if(!buffer->contains(QByteArray::fromHex("06 00 04 01 00 F5")) && readerState < 2)
        {
            readerState++;
            return;
        }
        else if(buffer->contains(QByteArray::fromHex("06 00 04 01 00 F5")))
        {
            writeSuccessful = true;
            emit MotionSent();
        }
    }
    else if(buffer->at(0) == 0x3E)
    {
        if(buffer->size() < 62)
            return;
        else if(buffer->size() == 62)
        {
            const qint32* rawX = (const qint32*)((const void*)(buffer->data() + 13));
            const qint32* rawY = (const qint32*)((const void*)(buffer->data() + 17));
            const qint32* rawZ = (const qint32*)((const void*)(buffer->data() + 21));
            bool isRunning = buffer->at(5) == 0x04;
            emit newServoState(Move_Servo_State(isRunning, *rawX / 100000.0, *rawY / 100000.0, *rawZ / 100000.0));
        }
    }
    buffer->clear();
    readerState = 0;
}

void MoveController::writeSlot(QByteArray data)
{
    stateTimer->stop();
    writeSuccessful = false;
    for(int i = 0; i < 20 && !writeSuccessful; i++)
    {
        if(write(data) == -1)
        {
            emit controllerError();
            break;
        }
        waitForReadyRead(10);
    }

    stateTimer->start();
}

void MoveController::getControllerState()
{
    emit currState(*state);
}

void MoveController::setPortNameSlot(QString name)
{
    setPortName(name);
}
