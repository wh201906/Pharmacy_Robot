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

    QList<QByteArray> file2list(QString path);
private slots:

    void on_servoTestButton_clicked();

    void on_testGroupBox_clicked(bool checked);

    void on_RFIDTestButton_clicked();

    void on_cameraTestButton_clicked();

    void on_testButton_clicked();

private:
    Ui::MainWindow *ui;
    RFID* reader;
    ServoDriver* servoDriver;
    Camera* camera;
    QThread* cameraThread;

    ServoTestDialog* servoTestDialog;
    RFIDTestDialog* myRFIDTestDialog;
    CameraTestDialog* cameraTestDialog;
    QList<QByteArray> drugInfo;
};
#endif // MAINWINDOW_H
