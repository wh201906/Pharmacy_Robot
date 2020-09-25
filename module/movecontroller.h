#ifndef MOVECONTROLLER_H
#define MOVECONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QDebug>

class MoveController : public QSerialPort
{
    Q_OBJECT
public:
    explicit MoveController(QThread* targetThread, QObject *parent = nullptr);
    struct Move_Servo_State
    {
        bool isValid = false;
        bool isRunning = false;
        double x;
        double y;
        double z;
        Move_Servo_State(bool running, double x, double y, double z)
        {
            isValid = true;
            this->isRunning = running;
            this->x = x;
            this->y = y;
            this->z = z;
        }
        Move_Servo_State()
        {
            isValid = false;
        }
    };
    struct Move_Controller_State
    {
        bool isOpened = false;
        QString portName;
    };

public slots:
    void writeSlot(QByteArray data);
    void openSlot(MoveController::OpenMode mode);
    void getControllerState();
    void closeSlot();
    void setPortNameSlot(QString name);
private slots:
    void onTimeout();
    void onReadyRead();
private:
    QTimer* stateTimer;
    QByteArray* buffer;
    Move_Controller_State* state;
signals:
    void newServoState(Move_Servo_State st);
    void controllerError();
    void currState(Move_Controller_State st);

};

#endif // MOVECONTROLLER_H
