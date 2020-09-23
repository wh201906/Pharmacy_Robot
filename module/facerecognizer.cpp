#include "facerecognizer.h"

FaceRecognizer::FaceRecognizer(QObject *parent) : QObject(parent)
{

}

void FaceRecognizer::init(const QStringList deviceList, const QStringList modelList)
{
    bool FLAGS_async = false;
    float FLAGS_bb_enlarge_coef = 1.2f;
    float FLAGS_t = 0.5f;
    bool FLAGS_r = false;
    float FLAGS_dx_coef = 1;
    float  FLAGS_dy_coef = 1;
    using namespace InferenceEngine;
    Core ie;

    QSet<QString> loadedDevices;
    faceDetector = new FaceDetection(modelList[0].toStdString(), deviceList[0].toStdString(), 1, false, FLAGS_async, FLAGS_t, FLAGS_r,
                                     static_cast<float>(FLAGS_bb_enlarge_coef), static_cast<float>(FLAGS_dx_coef), static_cast<float>(FLAGS_dy_coef));
    landmarksDetector = new FacialLandmarksDetection(modelList[1].toStdString(), deviceList[1].toStdString(), 16, false, FLAGS_async, FLAGS_r);
    loadedDevices = QSet<QString>(deviceList.begin(), deviceList.end());

    /** Per-layer metrics **/
    if(true)
    {
        ie.SetConfig({{PluginConfigParams::KEY_PERF_COUNT, PluginConfigParams::YES}});
    }

    Load(*faceDetector).into(ie, deviceList[0].toStdString(), false);
    Load(*landmarksDetector).into(ie, deviceList[1].toStdString(), false);

    camera = new cv::VideoCapture;
    camera->open(0);

    currentFrame = new cv::Mat;
}
cv::Rect FaceRecognizer::detect()
{
    faceDetector->enqueue(*currentFrame);
    faceDetector->submitRequest();
    faceDetector->wait();
    faceDetector->fetchResults();
    QVector<FaceDetection::Result> results = QVector<FaceDetection::Result>(faceDetector->results.begin(), faceDetector->results.end());
    if(results.size() == 0)
        return cv::Rect();
    else
    {
        return results[0].location;
    }
}



QImage FaceRecognizer::getFrame()
{
    if(currentFrame->empty())
        return QImage();
    else
        return frame2image(*currentFrame);
}

cv::Mat FaceRecognizer::getRawFrame()
{
    return *currentFrame;
}

QImage FaceRecognizer::getResult(RecogResult* result)
{
    if(!camera->read(*currentFrame))
    {
        write2ptr(result, RecogResult(RESULT_ERROR));
        return QImage();
    }
    cv::Mat resultFrame = currentFrame->clone();
    cv::Rect faceArea = detect();
    if(faceArea.area() < 10)
    {
        write2ptr(result, RecogResult(RESULT_ERROR));
        return getFrame();
    }
    cv::rectangle(resultFrame, faceArea, cv::Scalar(0, 255, 0));
    cv::Mat face = (*currentFrame)(faceArea & cv::Rect(0, 0, currentFrame->cols, currentFrame->rows));
    landmarksDetector->enqueue(face);
    landmarksDetector->submitRequest();
    landmarksDetector->wait();
    std::vector<float> normed_landmarks = (*landmarksDetector)[0];
    size_t n_lm = normed_landmarks.size();
    for(size_t i_lm = 0UL; i_lm < n_lm / 2; ++i_lm)
    {
        float normed_x = normed_landmarks[2 * i_lm];
        float normed_y = normed_landmarks[2 * i_lm + 1];
        int x_lm = faceArea.x + static_cast<int>(faceArea.width * normed_x);
        int y_lm = faceArea.y + static_cast<int>(faceArea.height * normed_y);

        cv::circle(resultFrame, cv::Point(x_lm, y_lm), 1 + static_cast<int>(0.012 * faceArea.width), cv::Scalar(0, 255, 255), -1);
    }
    return frame2image(resultFrame);
}

template<class T>
void FaceRecognizer::write2ptr(T* ptr, T value)
{
    if(ptr != nullptr)
        *ptr = value;
}

QImage FaceRecognizer::frame2image(cv::Mat& frame)
{
    return QImage((const unsigned char*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888).rgbSwapped();
}
