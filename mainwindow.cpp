#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    faceRecognizer = new FaceRecognizer(this);
    reader = new RFID;
    servoDriver = new ServoDriver;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_camOpenButton_clicked()
{
    faceRecognizer->init({"CPU", "CPU"}, {"/home/hdu/test/models/intel/face-detection-adas-0001/FP32/face-detection-adas-0001.xml", "/home/hdu/test/models/intel/facial-landmarks-35-adas-0002/FP32/facial-landmarks-35-adas-0002.xml"});
}

void MainWindow::on_getFrameButton_clicked()
{
    while(true)
    {
        ui->imgLabel->setPixmap(QPixmap::fromImage(faceRecognizer->getResult()));
        QApplication::processEvents();
    }
}

void MainWindow::on_camCloseButton_clicked()
{
    QApplication::exit();
}

void MainWindow::on_readCardButton_clicked()
{
    qDebug() << reader->get14aUID();
    //RFID::getIDCard_CNUID();
}

void MainWindow::on_saveImageButton_clicked()
{
    qDebug() << "./img/" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".jpg";
    qDebug() << faceRecognizer->getFrame().save("./img/" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".jpg");
}

void MainWindow::on_testButton_clicked()
{
    servoDriver->move_sendMotion(ServoDriver::MOVE_AXIS_X, -100000, 50);
}
