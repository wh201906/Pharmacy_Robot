#ifndef SERVOTESTDIALOG_H
#define SERVOTESTDIALOG_H

#include <QDialog>
#include <QDebug>
#include "module/servodriver.h"

namespace Ui
{
class ServoTestDialog;
}

class ServoTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServoTestDialog(ServoDriver* driver, QWidget *parent = nullptr);
    ~ServoTestDialog();

private slots:
    void on_moveConnectButton_clicked();

    void on_moveXPButton_clicked();

    void on_moveXNButton_clicked();

    void on_moveYPButton_clicked();

    void on_moveYNButton_clicked();

    void on_moveZPButton_clicked();

    void on_moveZNButton_clicked();

    void on_moveStopButton_clicked();

    void on_rotateConnectButton_clicked();

    void on_rotateSuckButton_clicked();

    void on_rotateStopSuckButton_clicked();

    void on_rotateTopSlider_sliderMoved(int position);

    void on_rotateBottomSlider_sliderMoved(int position);

private:
    Ui::ServoTestDialog *ui;
    ServoDriver* driver;
};

#endif // SERVOTESTDIALOG_H
