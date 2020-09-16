#include "servotestdialog.h"
#include "ui_servotestdialog.h"

ServoTestDialog::ServoTestDialog(ServoDriver* driver, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServoTestDialog)
{
    ui->setupUi(this);
    this->driver = driver;
}

ServoTestDialog::~ServoTestDialog()
{
    delete ui;
}

void ServoTestDialog::on_moveConnectButton_clicked()
{
    qDebug() << driver->move_connect(ui->movePortEdit->text());
}

void ServoTestDialog::on_moveXPButton_clicked()
{
    qDebug() << driver->move_sendMotion(ServoDriver::MOVE_AXIS_X, ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveXNButton_clicked()
{
    qDebug() << driver->move_sendMotion(ServoDriver::MOVE_AXIS_X, -ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveYPButton_clicked()
{
    qDebug() << driver->move_sendMotion(ServoDriver::MOVE_AXIS_Y, ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveYNButton_clicked()
{
    qDebug() << driver->move_sendMotion(ServoDriver::MOVE_AXIS_Y, -ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveZPButton_clicked()
{
    qDebug() << driver->move_sendMotion(ServoDriver::MOVE_AXIS_Z, ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveZNButton_clicked()
{
    qDebug() << driver->move_sendMotion(ServoDriver::MOVE_AXIS_Z, -ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveStopButton_clicked()
{
    qDebug() << driver->move_stop();
}

void ServoTestDialog::on_rotateConnectButton_clicked()
{
    qDebug() << driver->rotate_connect(ui->rotatePortEdit->text());
}

void ServoTestDialog::on_rotateSuckButton_clicked()
{
    qDebug() << driver->rotate_suck();
}

void ServoTestDialog::on_rotateStopSuckButton_clicked()
{
    qDebug() << driver->rotate_stopSuck();
}

void ServoTestDialog::on_rotateTopSlider_sliderMoved(int position)
{
    qDebug() << driver->rotate_sendMotion(ServoDriver::ROTATE_SERVO_TOP, position);
}

void ServoTestDialog::on_rotateBottomSlider_sliderMoved(int position)
{
    qDebug() << driver->rotate_sendMotion(ServoDriver::ROTATE_SERVO_BOTTOM, position);
}
