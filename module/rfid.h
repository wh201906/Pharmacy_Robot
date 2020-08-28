#ifndef RFID_H
#define RFID_H

#include <QObject>
#include <QDebug>
#include <QString>
#include <QThread>
#include <QApplication>
#include <nfc/nfc.h>

class RFID : public QObject
{
    Q_OBJECT
public:
    explicit RFID(QObject *parent = nullptr);
    static bool init();
    static bool openDevice();

    static QString get14aUID();
    static QString getIDCard_CNUID();
signals:

private:
    static nfc_device* nfcPn532;
    static nfc_target nfcTarget;
    static nfc_context* nfcContext;

    static QString hex2str(uint8_t hex[], size_t len);
};

#endif // RFID_H
