#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "module/camera.h"
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QDateTime>

#include "module/rfid.h"
#include "module/servodriver.h"
#include "testDialog/servotestdialog.h"
#include "testDialog/rfidtestdialog.h"
#include "testDialog/cameratestdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

//    int *drug_positioning(cv::Mat frame);
    int *drug_positioning(cv::Mat frame, cv::Mat *resultFrame);
private slots:

    void on_servoTestButton_clicked();

    void on_testGroupBox_clicked(bool checked);

    void on_RFIDTestButton_clicked();

    void on_cameraTestButton_clicked();

private:
    Ui::MainWindow *ui;
    RFID* reader;
    ServoDriver* servoDriver;
    Camera* camera;
    QThread* cameraThread;

    ServoTestDialog* servoTestDialog;
    RFIDTestDialog* myRFIDTestDialog;
    CameraTestDialog* cameraTestDialog;
};
#endif // MAINWINDOW_H
