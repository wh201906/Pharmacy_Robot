#ifndef SERVOTESTDIALOG_H
#define SERVOTESTDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QKeyEvent>
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

    void on_rotateTopEdit_returnPressed();

    void on_rotateBottomEdit_returnPressed();

    void on_moveStateButton_clicked();

    void on_moveDisconnectButton_clicked();

    void on_rotateDisconnectButton_clicked();

    void on_moveForceRangeBox_stateChanged(int arg1);

    void on_throwDrugButton_clicked();

    void on_moveToButton_clicked();

protected:
    bool eventFilter(QObject *, QEvent *) override;
private:
    Ui::ServoTestDialog *ui;
    ServoDriver* driver;
};

#endif // SERVOTESTDIALOG_H
