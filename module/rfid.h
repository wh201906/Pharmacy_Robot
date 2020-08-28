#ifndef RFID_H
#define RFID_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QSerialPort>
#include <QSerialPortInfo>

class RFID : public QObject
{
    Q_OBJECT
public:
    explicit RFID(QObject *parent = nullptr);

    void init();
    QString get14aUID();
signals:

private:
    QSerialPort* port;
    QList<QSerialPortInfo> infoList;
};

#endif // RFID_H
