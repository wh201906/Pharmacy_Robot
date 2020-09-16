#ifndef SERVODRIVER_H
#define SERVODRIVER_H

#include <QObject>
#include <QSerialPort>
#include <QDebug>

class ServoDriver : public QObject
{
    Q_OBJECT
public:
    explicit ServoDriver(QObject *parent = nullptr);

    enum Move_Axis
    {
        MOVE_AXIS_X,
        MOVE_AXIS_Y,
        MOVE_AXIS_Z,
    };

    bool move_connect(const QString &port);
    bool move_sendMotion(Move_Axis axis, float step, float speed);
    bool move_stop();
signals:

private:
    QSerialPort* moveController;
    QSerialPort* rotateController;
};


#endif // SERVODRIVER_H
