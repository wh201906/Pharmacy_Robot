#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <opencv.hpp>
#include <opencv2/imgproc/types_c.h>

class Camera : public QObject
{
    Q_OBJECT
public:
    explicit Camera(QObject *parent = nullptr);

signals:

};

#endif // CAMERA_H
