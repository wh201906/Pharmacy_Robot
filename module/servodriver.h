#ifndef SERVODRIVER_H
#define SERVODRIVER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

class ServoDriver : public QObject
{
    Q_OBJECT
public:
    explicit ServoDriver(QObject *parent = nullptr);

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

    bool move_connect(const QString& port);
    bool move_sendMotion(Move_Axis axis, float step, float speed);
    bool move_stop();
    bool rotate_connect(const QString &port);
    bool rotate_suck();
    bool rotate_stopSuck();
    bool rotate_sendMotion(Rotate_Servo servo, int pos, int speed = 1000);
signals:

private:
    QSerialPort* moveController;
    QSerialPort* rotateController;
    quint16 moveID;
    quint16 rotateID;
};


#endif // SERVODRIVER_H
