#include "camera.h"

Camera::Camera(QThread* thread, QObject *parent) : QObject(parent)
{
    this->thread = thread;
    moveToThread(thread);
    cam = new cv::VideoCapture;
    rawFrame = new cv::Mat;
    refreshTimer = new QTimer();
    refreshTimer->moveToThread(thread);
    refreshTimer->setInterval(50);
    connect(refreshTimer, &QTimer::timeout, this, &Camera::onRefreshTimeout);
}

void Camera::openCam(int id)
{
    bool res = cam->open(id);
    qDebug() << res;
    if(res)
    {
        refreshTimer->start();
    }
}

void Camera::onRefreshTimeout()
{
    cam->read(*rawFrame);
    emit frameRefreshed();
}

void Camera::closeCam()
{
    refreshTimer->stop();
    cam->release();
}

void Camera::getFrameAddr()
{
    emit frameAddr(rawFrame);
}
