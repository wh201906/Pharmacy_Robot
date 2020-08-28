#include "rfid.h"

nfc_device* RFID::nfcPn532;
nfc_target RFID::nfcTarget;
nfc_context* RFID::nfcContext;

RFID::RFID(QObject *parent) : QObject(parent)
{

}

bool RFID::init()
{
    setenv("LIBNFC_INTRUSIVE_SCAN", "yes", 1);
    nfc_init(&nfcContext);
    return(nfcContext != NULL);
}
bool RFID::openDevice()
{
    nfcPn532 = nfc_open(nfcContext, NULL);
    if(nfcPn532 == NULL) return false;

    if(nfc_initiator_init(nfcPn532) < 0)
    {
        nfc_perror(nfcPn532, "nfc_initiator_init");
        return false;
    }

    return true;
}
QString RFID::get14aUID()
{
    QString result;
    if(!init()) return "";
    if(!openDevice())
    {
        nfc_exit(nfcContext);
        return "";
    }
    nfc_device_set_property_bool(nfcPn532, NP_FORCE_ISO14443_B, false);
    nfc_device_set_property_bool(nfcPn532, NP_FORCE_ISO14443_A, true);
    nfc_device_set_property_bool(nfcPn532, NP_HANDLE_CRC, true);
    const nfc_modulation nmMifare =
    {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };
    if(nfc_initiator_select_passive_target(nfcPn532, nmMifare, NULL, 0, &nfcTarget) > 0)
    {
        result = hex2str(nfcTarget.nti.nai.abtUid, nfcTarget.nti.nai.szUidLen);
    }
    nfc_close(nfcPn532);
    nfc_exit(nfcContext);
    return result;
}

QString RFID::getIDCard_CNUID()
{
    uint8_t cmd[3][11] =
    {
        {0x05, 0x00, 0x00, 0x71, 0xff},
        {0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x08, 0xf3, 0x10},
        {0x00, 0x36, 0x00, 0x00, 0x08, 0x57, 0x44},
    };
    uint8_t recvBuf[20] = {0};
    QString result;
    nfc_target targets[20];
    if(!init()) return "";
    if(!openDevice())
    {
        nfc_exit(nfcContext);
        return "";
    }
    qDebug() << "-------------start config------------";
    nfc_device_set_property_bool(nfcPn532, NP_FORCE_ISO14443_A, false);
    nfc_device_set_property_bool(nfcPn532, NP_FORCE_ISO14443_B, true);
    nfc_device_set_property_bool(nfcPn532, NP_HANDLE_CRC, true);
    //nfc_device_set_property_bool(nfcPn532, NP_HANDLE_PARITY, false);
    nfc_modulation nmIDCard_CN =
    {
        .nmt = NMT_ISO14443B,
        .nbr = NBR_106
    };
    nfc_device_set_property_bool(nfcPn532, NP_EASY_FRAMING, false);
    nfc_device_set_property_bool(nfcPn532, NP_ACTIVATE_FIELD, true);
    nfc_device_set_property_int(nfcPn532, NP_TIMEOUT_ATR, 1000);
    nfc_device_set_property_int(nfcPn532, NP_TIMEOUT_COM, 3000);

    qDebug() << "-------------start transceive------------";
    nfc_initiator_transceive_bytes(nfcPn532, cmd[0], 3, recvBuf, 20, 0);
    qDebug() << hex2str(recvBuf, 20);
    nfc_initiator_transceive_bytes(nfcPn532, cmd[1], 9, recvBuf, 20, 0);
    qDebug() << hex2str(recvBuf, 20);
    nfc_initiator_transceive_bytes(nfcPn532, cmd[2], 5, recvBuf, 20, 0);
    qDebug() << hex2str(recvBuf, 20);
    qDebug() << "-------------stop transceive三------------";
    nfc_device_set_property_bool(nfcPn532, NP_ACTIVATE_FIELD, false);

    nfc_close(nfcPn532);
    nfc_exit(nfcContext);
    return result;
}

QString RFID::hex2str(uint8_t hex[], size_t len)
{
    QString result, tmp;
    for(size_t i = 0; i < len; i++)
    {
        tmp = QString::number(hex[i], 16);
        if(tmp.length() == 1)
            tmp = "0" + tmp;
        result += tmp;
    }
    return result;
}
