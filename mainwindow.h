#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "module/camera.h"
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QDateTime>

#include <opencv.hpp>
#include <opencv2/imgproc/types_c.h>
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
    QMap<QString, QPointF> file2drugInfo(QString path);
    QPointF linearTransform(QPointF vPoint, QRect vRect);
    void delay(int ms);
    double getSimilarity(const QString &str1, const QString &str2);
    bool callOCR();
    bool getOCRMatchState(const QString &str);
    QPointF gotoPos(const QString &ID);
public slots:
    void onDrugRectFetched(QRect rect);
    void onFrameRefreshed();
    void onFrameAddrFetched(cv::Mat *rawAddr, cv::Mat *roiAddr, cv::Mat *roiOfRawAddr);
    void onOCRResultFetched(QString result);
private slots:

    void on_servoTestButton_clicked();

    void on_testGroupBox_clicked(bool checked);

    void on_RFIDTestButton_clicked();

    void on_cameraTestButton_clicked();

    void on_cameraGroupBox_clicked(bool checked);

    void on_startButton_clicked();
    void on_stopButton_clicked();

private:
    Ui::MainWindow *ui;
    RFID* reader;
    ServoDriver* servoDriver;
    Camera* camera;
    QThread* cameraThread;
    QString ocrResult;

    ServoTestDialog* servoTestDialog;
    RFIDTestDialog* myRFIDTestDialog;
    CameraTestDialog* cameraTestDialog;
    QMap<QString, QPointF> totalDrugInfo;
    QRect visualRect;
    cv::Mat* cameraFrame;
    QPixmap mat2Pixmap(cv::Mat *mat);

    bool isProcessing = false;
signals:
    void getFrameAddr();
    void getOCRResult();
    void getRectResult();
    void setLabelBuffer(QString bufferText);
};
#endif // MAINWINDOW_H
