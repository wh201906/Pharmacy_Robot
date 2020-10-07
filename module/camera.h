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
#include <QElapsedTimer>
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
    void getRectResult();
    void onOCRProcessFinished(int exitCode, QProcess::ExitStatus state);
    void setLabelBuffer(QString bufferText);
signals:
    void OCRResult(QString result);
    void frameRefreshed();
    void frameAddr(cv::Mat* rawAddr, cv::Mat* roiAddr, cv::Mat* roiOfRawAddr);
    void drugRect(QRect rect);
private slots:
    void onRefreshTimeout();
private:
    QThread* thread;
    cv::VideoCapture* cam;
    QTimer* refreshTimer;
    cv::Mat* rawFrame;
    cv::Mat* roiFrame;
    cv::Mat* roiOfRawFrame;
    QFile* ocrResultFile;
    QProcess* pyProcess;
    QElapsedTimer* ocrTimer;
    QString labelBuffer;
    QRect drug_positioning(cv::Mat* frame, cv::Mat* roiFrame, cv::Mat* resultFrame, bool* isCenter = nullptr);
    void callOCR();
};

#endif // CAMERA_H
