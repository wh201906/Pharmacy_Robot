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
    driver->move_connect(ui->movePortEdit->text());
}


void ServoTestDialog::on_moveXPButton_clicked()
{
    driver->move_sendMotion(ServoDriver::MOVE_AXIS_X, ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveXNButton_clicked()
{
    driver->move_sendMotion(ServoDriver::MOVE_AXIS_X, -ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveYPButton_clicked()
{
    driver->move_sendMotion(ServoDriver::MOVE_AXIS_Y, ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveYNButton_clicked()
{
    driver->move_sendMotion(ServoDriver::MOVE_AXIS_Y, -ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveZPButton_clicked()
{
    driver->move_sendMotion(ServoDriver::MOVE_AXIS_Z, ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveZNButton_clicked()
{
    driver->move_sendMotion(ServoDriver::MOVE_AXIS_Z, -ui->moveStepEdit->text().toFloat(), ui->moveSpeedEdit->text().toFloat());
}

void ServoTestDialog::on_moveStopButton_clicked()
{
    driver->move_stop();
}
