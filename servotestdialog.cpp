#include "servotestdialog.h"
#include "ui_servotestdialog.h"

ServoTestDialog::ServoTestDialog(ServoDriver* driver, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServoTestDialog)
{
    ui->setupUi(this);
    this->driver = driver;
    installEventFilter(this);
    if(driver->move_getPort() != "")
        ui->movePortEdit->setText(driver->move_getPort());
    if(driver->rotate_getPort() != "")
        ui->rotatePortEdit->setText(driver->rotate_getPort());
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
    on_rotateTopEdit_returnPressed();
    on_rotateBottomEdit_returnPressed();
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
    ui->rotateTopEdit->setText(QString::number(position));
}

void ServoTestDialog::on_rotateBottomSlider_sliderMoved(int position)
{
    qDebug() << driver->rotate_sendMotion(ServoDriver::ROTATE_SERVO_BOTTOM, position);
    ui->rotateBottomEdit->setText(QString::number(position));
}

void ServoTestDialog::on_rotateTopEdit_returnPressed()
{
    qDebug() << driver->rotate_sendMotion(ServoDriver::ROTATE_SERVO_TOP, ui->rotateTopEdit->text().toInt());
    ui->rotateTopSlider->setValue(ui->rotateTopEdit->text().toInt());
}

void ServoTestDialog::on_rotateBottomEdit_returnPressed()
{
    qDebug() << driver->rotate_sendMotion(ServoDriver::ROTATE_SERVO_BOTTOM, ui->rotateBottomEdit->text().toInt());
    ui->rotateBottomSlider->setValue(ui->rotateBottomEdit->text().toInt());
}

void ServoTestDialog::on_moveStateButton_clicked()
{
    ServoDriver::Move_State st = driver->move_getState();
    qDebug() << st.isValid << st.isRunning << st.x << st.y << st.z;
}

bool ServoTestDialog::eventFilter(QObject *watched, QEvent *event)
{
    // for move servo:
    // use WASD to control the X axis and Y axis
    // use RF to control the Z axis
    // use [] to change the step
    // use ,. to change the speed
    //
    // for rotate servo:
    // use Up/Down to control the top servo
    // use Left/Right to control the bottom servo
    // use 0/1 to control the sucker

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = (QKeyEvent*)event;
        qDebug() << keyEvent;
        switch(keyEvent->key())
        {
        case Qt::Key_W:
            on_moveXNButton_clicked();
            break;
        case Qt::Key_A:
            on_moveYPButton_clicked();
            break;
        case Qt::Key_S:
            on_moveXPButton_clicked();
            break;
        case Qt::Key_D:
            on_moveYNButton_clicked();
            break;
        case Qt::Key_F:
            on_moveZPButton_clicked();
            break;
        case Qt::Key_R:
            on_moveZNButton_clicked();
            break;
        case Qt::Key_BracketLeft:
            ui->moveStepEdit->setText(QString::number(ui->moveStepEdit->text().toInt() - 5));
            break;
        case Qt::Key_BracketRight:
            ui->moveStepEdit->setText(QString::number(ui->moveStepEdit->text().toInt() + 5));
            break;
        case Qt::Key_Comma:
            ui->moveSpeedEdit->setText(QString::number(ui->moveSpeedEdit->text().toInt() - 5));
            break;
        case Qt::Key_Period:
            ui->moveSpeedEdit->setText(QString::number(ui->moveSpeedEdit->text().toInt() + 5));
            break;
        case Qt::Key_Space:
            on_moveStopButton_clicked();
            break;
        case Qt::Key_0:
            on_rotateStopSuckButton_clicked();
            break;
        case Qt::Key_1:
            on_rotateSuckButton_clicked();
            break;
        case Qt::Key_Up:
            ui->rotateTopEdit->setText(QString::number(ui->rotateTopEdit->text().toInt() - 10));
            on_rotateTopEdit_returnPressed();
            break;
        case Qt::Key_Down:
            ui->rotateTopEdit->setText(QString::number(ui->rotateTopEdit->text().toInt() + 10));
            on_rotateTopEdit_returnPressed();
            break;
        case Qt::Key_Left:
            ui->rotateBottomEdit->setText(QString::number(ui->rotateBottomEdit->text().toInt() - 10));
            on_rotateBottomEdit_returnPressed();
            break;
        case Qt::Key_Right:
            ui->rotateBottomEdit->setText(QString::number(ui->rotateBottomEdit->text().toInt() + 10));
            on_rotateBottomEdit_returnPressed();
            break;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void ServoTestDialog::on_moveDisconnectButton_clicked()
{
    driver->move_disconnect();
}

void ServoTestDialog::on_rotateDisconnectButton_clicked()
{
    driver->rotate_disconnect();
}
