#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    reader = new RFID;
    servoDriver = new ServoDriver;
    cameraThread = new QThread;
    camera = new Camera(cameraThread);
    cameraThread->start();
    servoTestDialog = new ServoTestDialog(servoDriver);
    servoTestDialog->setModal(false);
    myRFIDTestDialog = new RFIDTestDialog(reader);
    myRFIDTestDialog->setModal(false);
    cameraTestDialog = new CameraTestDialog(camera);
    cameraTestDialog->setModal(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_testGroupBox_clicked(bool checked)
{
    ui->servoTestButton->setVisible(checked);
    ui->RFIDTestButton->setVisible(checked);
    ui->cameraTestButton->setVisible(checked);
//    ui->testGroupBox->adjustSize();
}

void MainWindow::on_servoTestButton_clicked()
{
    servoTestDialog->show();
}

void MainWindow::on_RFIDTestButton_clicked()
{
    myRFIDTestDialog->show();
}

void MainWindow::on_cameraTestButton_clicked()
{
    cameraTestDialog->show();
}

void MainWindow::on_testButton_clicked()
{
    ui->drugListWidget->clear();
    QString userID = reader->get14aUID().toUpper();
    if(userID == "")
    {
        ui->idLabel->setText("No Card");
        return;
    }
    ui->idLabel->setText("ID:" + userID);
    QList<QByteArray> patientInfo = file2list("/home/hdu/Pharmacy_Robot/" + userID + ".txt");
    if(patientInfo.size() == 0)
    {
        ui->nameLabel->setText("Name:Not Found");
        return;
    }
    ui->nameLabel->setText("Name:" + patientInfo[0]);
    for(int i = 1; i < patientInfo.size(); i++)
    {
        if(patientInfo[i].size() == 0)
            continue;
        QList<QByteArray> drugInfo = patientInfo[i].split(',');
        ui->drugListWidget->addItem(drugInfo[0]);
    }
}

QList<QByteArray> MainWindow::file2list(QString path)
{
    QList<QByteArray> result;
    QFile file(path);
    if(!file.open(QFile::Text | QFile::ReadOnly))
        return result;
    result = file.readAll().split('\n');
    return result;
}
