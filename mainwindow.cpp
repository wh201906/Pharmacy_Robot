#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cap = new cv::VideoCapture;
    videoFrame = new cv::Mat;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_camOpenButton_clicked()
{
    int timeoutCounter = 0;
    qDebug() << "Open Camera:" << cap->open(0);
    while(!cap->isOpened() && timeoutCounter < 100)
    {
        QThread::sleep(10);
        timeoutCounter++;
    }
    if(!cap->isOpened())
    {
        QMessageBox::information(this, "Error", "Can't open the camera.");
    }
}

void MainWindow::on_getFrameButton_clicked()
{
    cap->read(*videoFrame);
    ui->imgLabel->setPixmap(QPixmap::fromImage(QImage((const unsigned char*)videoFrame->data, videoFrame->cols, videoFrame->rows, videoFrame->step, QImage::Format_RGB888)));
}

void MainWindow::on_camCloseButton_clicked()
{
    cap->release();
}

void MainWindow::on_readCardButton_clicked()
{
    // Initialize libnfc and set the nfc_nfcContext
    nfc_init(&nfcContext);
    if(nfcContext == NULL)
    {
        qDebug() << ("Unable to init libnfc (malloc)\n");
        return;
    }

    nfcPn532 = nfc_open(nfcContext, NULL);

    if(nfcPn532 == NULL)
    {
        qDebug() << ("ERROR: %s\n", "Unable to open NFC device.");
        return;
    }
    // Set opened NFC device to initiator mode
    if(nfc_initiator_init(nfcPn532) < 0)
    {
        nfc_perror(nfcPn532, "nfc_initiator_init");
        return;
    }

    qDebug() << ("NFC reader: %s opened\n", nfc_device_get_name(nfcPn532));

    // Poll for a ISO14443A (MIFARE) tag
    const nfc_modulation nmMifare =
    {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };
    if(nfc_initiator_select_passive_target(nfcPn532, nmMifare, NULL, 0, &nfcTarget) > 0)
    {
        qDebug() << ("The following (NFC) ISO14443A tag was found:\n");
        qDebug() << ("    ATQA (SENS_RES): ");
        qDebug() << (nfcTarget.nti.nai.abtAtqa, 2);
        qDebug() << ("       UID (NFCID%c): ", (nfcTarget.nti.nai.abtUid[0] == 0x08 ? '3' : '1'));
        qDebug() << (nfcTarget.nti.nai.abtUid, nfcTarget.nti.nai.szUidLen);
        qDebug() << ("      SAK (SEL_RES): ");
        qDebug() << (&nfcTarget.nti.nai.btSak, 1);
        if(nfcTarget.nti.nai.szAtsLen)
        {
            qDebug() << ("          ATS (ATR): ");
            qDebug() << (nfcTarget.nti.nai.abtAts, nfcTarget.nti.nai.szAtsLen);
        }
    }
    // Close NFC device
    nfc_close(nfcPn532);
    // Release the nfcContext
    nfc_exit(nfcContext);
}
