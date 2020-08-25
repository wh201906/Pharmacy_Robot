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
//    qDebug() << RFID::get14aUID();
    RFID::getIDCard_CNUID();
}
