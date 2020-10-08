#include "camera.h"

Camera::Camera(QThread* thread, QObject *parent) : QObject(parent)
{
    this->thread = thread;
    moveToThread(thread);
    cam = new cv::VideoCapture;
    rawFrame = new cv::Mat;
    roiFrame = new cv::Mat;
    roiOfRawFrame = new cv::Mat;
    ocrResultFile = new QFile("/home/hdu/Pharmacy_Robot_RAM/ocr.txt");
    refreshTimer = new QTimer;
    ocrTimer = new QElapsedTimer;
    pyProcess = new QProcess;
    pyProcess->moveToThread(thread);
    refreshTimer->moveToThread(thread);
    refreshTimer->setInterval(50);
    connect(refreshTimer, &QTimer::timeout, this, &Camera::onRefreshTimeout);
    connect(pyProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Camera::onOCRProcessFinished);
}

void Camera::setLabelBuffer(QString bufferText)
{
    labelBuffer = bufferText;
}

void Camera::onOCRProcessFinished(int exitCode, QProcess::ExitStatus state)
{
    qDebug() << "OCR Stoped" << ocrTimer->elapsed();

    ocrResultFile->open(QFile::Text | QFile::ReadOnly);
    QString result = ocrResultFile->readAll();
    ocrResultFile->close();
    emit OCRResult(result);
}

void Camera::openCam(int id)
{
    bool res = cam->open(id);
    qDebug() << res;
    if(res)
    {
        refreshTimer->start();
    }
    getRectResult();
    emit frameAddr(rawFrame, roiFrame, roiOfRawFrame);
}

void Camera::onRefreshTimeout()
{
    bool readResult = cam->read(*rawFrame);
    if(!readResult)
    {
        cam->release();
        for(int i = 0; i < 10; i++)
        {
            if(cam->open(i))
                break;
        }
    }
    getRectResult();
    emit frameRefreshed();
}

void Camera::closeCam()
{
    refreshTimer->stop();
    cam->release();
}

void Camera::getFrameAddr()
{
    if(roiFrame->cols != 0 && roiFrame->rows != 0)
        emit frameAddr(rawFrame, roiFrame, roiOfRawFrame);
    else
        emit frameAddr(rawFrame, nullptr, nullptr);
}

void Camera::getRectResult()
{
    QRect res = drug_positioning(rawFrame, roiFrame, roiOfRawFrame);
    emit drugRect(res);
}

void Camera::getOCRResult()
{
    labelBuffer = "";
    if(cv::imwrite("/home/hdu/Pharmacy_Robot_RAM/roi.jpg", *roiFrame))
        callOCR();
}

QRect Camera::drug_positioning(cv::Mat* frame, cv::Mat* roiFrame, cv::Mat* resultFrame, bool* isCenter)
{
    if(isCenter != nullptr)
        *isCenter = false;
    QRect res;
    double scale = 1;//0.5

    res.setX(0);
    res.setY(0);
    res.setWidth(0);
    res.setHeight(0);

    using namespace cv;
    if(frame->cols == 0 || frame->rows == 0)
        return res;
    cv::Size ResImgSiz = cv::Size(frame->cols * scale, frame->rows * scale);
    cv::Mat gFrame = cv::Mat(ResImgSiz, frame->type());
    cv::resize(*frame, gFrame, ResImgSiz);

    cv::Mat rFrame;
    cvtColor(gFrame, rFrame, CV_BGR2GRAY);

    cv::Mat hsvMat;
    cv::Mat edgeX_Mat;
    cv::Mat edgeY_Mat;
    cv::Mat edgeX_Mat_out;
    cv::Mat edgeY_Mat_out;
    cv::Mat edgeX_Y_Mat;
    cv::Mat edgeX_Y_Mat_out;
    cv::Mat g_edgeX_Y_Mat_out;
    cv::Mat open, dilate, open_2;

    //		cvtColor(rFrame, hsvMat, COLOR_BGR2HSV);
    //边缘检测
    Sobel(rFrame, edgeX_Mat, CV_16SC1, 1, 0, 3);
    //		convertScaleAbs(edgeX_Mat, edgeX_Mat_out);
    Sobel(rFrame, edgeY_Mat, CV_16SC1, 0, 1, 3);
    //		convertScaleAbs(edgeY_Mat, edgeY_Mat_out);
    edgeX_Y_Mat = edgeX_Mat + edgeY_Mat;
    //图像增强
    convertScaleAbs(edgeX_Y_Mat, edgeX_Y_Mat_out);
    //二值化
    threshold(edgeX_Y_Mat_out, g_edgeX_Y_Mat_out, 190, 255, THRESH_BINARY);
    //膨胀./ramdisk.sh 123456
    cv::Mat element = getStructuringElement(MORPH_RECT, Size(4, 3), Point(-1, -1));
    morphologyEx(g_edgeX_Y_Mat_out, dilate, 1, element, Point(-1, -1));
    //连通域标记
    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(dilate, labels, stats, centroids);
    if(stats.rows > 1) //除背景外有对象
    {
        //最大周长
        int max_i = 1;
        int32_t max_Perimeter = stats.at<int>(1, 2) + stats.at<int>(1, 3);
        if(stats.rows > 2) //stats.rows == 2时只有一个对象
        {
            int i_num = num - 1;
            for(int i = 2; i <= i_num; i++)
            {
                int32_t i_Perimeter = stats.at<int>(i, 2) + stats.at<int>(i, 3);
                if(i_Perimeter > max_Perimeter)
                {
                    max_Perimeter = i_Perimeter;
                    max_i = i;
                }
            }
        }
        res.setX(stats.at<int>(max_i, 0));
        res.setY(stats.at<int>(max_i, 1));
        res.setWidth(stats.at<int>(max_i, 2));
        res.setHeight(stats.at<int>(max_i, 3));
        //画框
        int x_drug = stats.at<int>(max_i, 0);
        int y_drug = stats.at<int>(max_i, 1);
        int width_drug = stats.at<int>(max_i, 2);
        int height_drug = stats.at<int>(max_i, 3);
        float width_height = width_drug / height_drug;
        float height_width = height_drug / width_drug;

        cv::Rect rect;
        rect.x = x_drug;
        rect.y = y_drug;
        rect.width = width_drug;
        rect.height = height_drug;
        //printf("width/height=%f \n max_perimeter=%d \n", width_height, max_Perimeter);
        if(max_Perimeter > 430 && width_height < 4 && height_width < 1.2)
        {
            Point p1 = Point(x_drug, y_drug);
            Point p2 = Point(x_drug + width_drug, y_drug + height_drug);
            Rect roi = Rect(p1, p2);
            if(roi.width && roi.height && roiFrame != nullptr)
            {
                *roiFrame = gFrame(roi);
            }
            rectangle(gFrame, rect, CV_RGB(255, 0, 0), 2, 8, 0);
            if(isCenter != nullptr)
                *isCenter = true;
            //在图上显示药品名
            if(labelBuffer != "")
            {
                char size = 5;//调节字体的y坐标
                std::string s_labelBuffer = (const char*)labelBuffer.toLocal8Bit();
                //printf("\n!!!!!!!%s\n", s_labelBuffer.c_str());
                cv::Point p = cv::Point(rect.x, rect.y - size);
                cv::putText(gFrame, s_labelBuffer, p, cv::FONT_HERSHEY_TRIPLEX, 0.7, cv::Scalar(0, 0, 255), 1);
            }
//            printf("rect");
        }
        else
        {
//            printf("no rect!");
        }
    }
    else //只有背景
    {
        res.setX(0);
        res.setY(0);
        res.setWidth(0);
        res.setHeight(0);
    }
//    qDebug() << res;
    if(resultFrame != nullptr)
        gFrame.copyTo(*resultFrame);
    return res;
}

void Camera::callOCR()
{
    ocrTimer->start();
    qDebug() << "OCR Started";
    pyProcess->start("python", {"/home/hdu/chineseocr_lite-onnx/detect_mine.py"});
}
