#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Python.h>
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QDateTime>
#include <opencv.hpp>
#include <opencv2/imgproc/types_c.h>

#include "module/rfid.h"
#include "module/servodriver.h"
#include "module/camera.h"
#include "servotestdialog.h"
#include "rfidtestdialog.h"
#include "cameratestdialog.h"

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
    void on_camOpenButton_clicked();

    void on_getFrameButton_clicked();

    void on_camCloseButton_clicked();

    void on_saveImageButton_clicked();

    void on_testButton_clicked();

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
