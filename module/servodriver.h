#ifndef SERVODRIVER_H
#define SERVODRIVER_H

#include <QObject>
#include <QSerialPort>

class ServoDriver : public QObject
{
    Q_OBJECT
public:
    explicit ServoDriver(QObject *parent = nullptr);

signals:

private:
    QSerialPort* moveController;
    QSerialPort* rotateController;
};


#endif // SERVODRIVER_H
