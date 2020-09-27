#ifndef CAMERA_H
#define CAMERA_H

#include <Python.h>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QRect>
#include <QFile>
#include <QImage>
#include <QProcess>
#include <opencv.hpp>
#include <opencv2/imgproc/types_c.h>

class Camera : public QObject
{
    Q_OBJECT
public:
    explicit Camera(QThread* thread, QObject *parent = nullptr);

public slots:
    void openCam(int id);
    void getFrameAddr();
    void closeCam();
    void getOCRResult();
    void setOCRState(bool enabled);
signals:
    void OCRResult(QString result);
    void frameRefreshed();
    void frameAddr(cv::Mat* rawAddr, cv::Mat* roiAddr, cv::Mat* roiOfRawAddr);
private slots:
    void onRefreshTimeout();
private:
    QThread* thread;
    cv::VideoCapture* cam;
    QTimer* refreshTimer;
    cv::Mat* rawFrame;
    cv::Mat* roiFrame;
    cv::Mat* roiOfRawFrame;
    QFile* roiFile;
    QFile* ocrResultFile;
    QRect drug_positioning(cv::Mat* frame, cv::Mat* roiFrame, cv::Mat* resultFrame, bool* isCenter);
    QString callOCR();
    bool ocrState = false;
};

#endif // CAMERA_H
