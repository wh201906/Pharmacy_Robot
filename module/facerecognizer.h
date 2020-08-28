#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QRect>
#include <QImage>
#include <QDebug>

#include <inference_engine.hpp>
#include <ie_iextension.h>

#include "detectors.hpp"

class FaceRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit FaceRecognizer(QObject *parent = nullptr);

    enum FaceResult
    {
        RESULT_ERROR,
        RESULT_NO_FACE,
        RESULT_NO_IDENTIY,
        RESULT_SUCCESS,
    };
    struct RecogResult
    {
        FaceResult result = RESULT_NO_FACE;
        QString id = "";
        QString name = "";
        RecogResult(FaceResult result, QString id, QString name)
        {
            this->result = result;
            this->id = id;
            this->name = name;
        }
        RecogResult(FaceResult result): RecogResult(result, "", "") {}
    };

    void init(const QStringList deviceList, const QStringList modelList);
    cv::Rect detect();
    QImage getFrame();
    QImage getResult(RecogResult* result = nullptr);

    template<class T>
    void write2ptr(T *ptr, T value);
    QImage frame2image(cv::Mat &frame);
signals:

private:
    FaceDetection* faceDetector;
    FacialLandmarksDetection* landmarksDetector;
    cv::VideoCapture* camera;
    cv::Mat* currentFrame;
    bool isSuccessful;
};

#endif // FACERECOGNIZER_H
