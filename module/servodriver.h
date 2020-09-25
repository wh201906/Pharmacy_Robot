#ifndef SERVODRIVER_H
#define SERVODRIVER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QThread>
#include <QApplication>
#include "module/movecontroller.h"

class ServoDriver : public QObject
{
    Q_OBJECT
public:
    explicit ServoDriver(QObject *parent = nullptr);

    const double MOVE_MAX_X = -697;
    const double MOVE_MAX_Y = 950;
    const double MOVE_MAX_Z = -100;
    const int ROTATE_INIT_BOTTOM = 1550;
    const int ROTATE_INIT_TOP = 2320;

    enum Move_Axis
    {
        MOVE_AXIS_X = 0,
        MOVE_AXIS_Y,
        MOVE_AXIS_Z,
    };
    enum Rotate_Servo
    {
        ROTATE_SERVO_TOP = 1,
        ROTATE_SERVO_BOTTOM,
        ROTATE_SERVO_SUCKER,
    };

    void move_connect(const QString& port);
    void move_sendMotion(Move_Axis axis, float step, float speed);
    void move_stop();
    bool rotate_connect(const QString &port);
    bool rotate_suck();
    bool rotate_stopSuck();
    bool rotate_sendMotion(Rotate_Servo servo, int pos, int speed = 1000);
    MoveController::Move_Servo_State move_getServoState();
    QString move_getPort();
    QString rotate_getPort();
    void move_disconnect();
    void rotate_disconnect();
    void move_setForceRange(bool st);
    bool move_getForceRange();
    void move_goto(float x, float y, float speed);
    void throwDrug();
    bool move_waitMotionFinished(int msec = 15000);
    bool rotate_initPos(bool withSucker = false);

signals:
    void move_setPortName(QString name);
    void move_connectPort(MoveController::OpenMode mode);
    void move_disconnectPort();
    void move_write(QByteArray data);
    void move_getControllerState();
private slots:
    void move_onControllerErrorOccurred();
    void move_onServoStateFetched(MoveController::Move_Servo_State st);
    void move_onControllerStateFetched(MoveController::Move_Controller_State st);
private:
    MoveController* moveController;
    QSerialPort* rotateController;
    QThread* moveThread;
    quint16 moveID;
    quint16 rotateID;
    MoveController::Move_Servo_State* servoState;
    MoveController::Move_Controller_State* controllerState;

    bool move_forceRange = true;
    const QList<double> layerHight = {-2.016, -127.216, -273.379, -424.317, -550.007, -697};
};


#endif // SERVODRIVER_H
