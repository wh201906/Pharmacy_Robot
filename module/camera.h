#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTimer>
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
signals:
    void frameRefreshed();
    void frameAddr(cv::Mat* addr);
private slots:
    void onRefreshTimeout();
private:
    QThread* thread;
    cv::VideoCapture* cam;
    QTimer* refreshTimer;
    cv::Mat* rawFrame;
};

#endif // CAMERA_H
