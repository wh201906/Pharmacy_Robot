#include "camera.h"

Camera::Camera(QThread* thread, QObject *parent) : QObject(parent)
{
    this->thread = thread;
    moveToThread(thread);
    cam = new cv::VideoCapture;
    rawFrame = new cv::Mat;
    roiFrame = new cv::Mat;
    roiOfRawFrame = new cv::Mat;
    ocrResultFile = new QFile("/home/hdu/Pharmacy_Robot/ocr.txt");
    refreshTimer = new QTimer();
    pyProcess = new QProcess;
    pyProcess->moveToThread(thread);
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
    if(cv::imwrite("/home/hdu/Pharmacy_Robot/roi.jpg", *roiFrame))
        emit OCRResult(callOCR());
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
    cv::Mat open, close, open_2;

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
    threshold(edgeX_Y_Mat_out, g_edgeX_Y_Mat_out, 250, 255, THRESH_BINARY);//160//180
    //开运算
    cv::Mat element = getStructuringElement(MORPH_RECT, Size(2, 1), Point(-1, -1));
    morphologyEx(g_edgeX_Y_Mat_out, open, 2, element, Point(-1, -1));
    //闭运算
    element = getStructuringElement(MORPH_RECT, Size(4, 5), Point(-1, -1));
    morphologyEx(open, close, 3, element, Point(-1, -1));
    //连通域标记
    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(close, labels, stats, centroids);
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

        cv::Rect rect;
        rect.x = x_drug;
        rect.y = y_drug;
        rect.width = width_drug;
        rect.height = height_drug;
//        if(max_Perimeter > 500 && center_x_drug > center_x_gFrame - tolerance_x && center_x_drug < center_x_gFrame + tolerance_x && center_y_drug > center_y_gFrame - tolerance_y && center_y_drug < center_y_gFrame + tolerance_y)
        if(max_Perimeter > 100)
        {
            Point p1 = Point(x_drug, y_drug);
            Point p2 = Point(x_drug + width_drug, y_drug + height_drug);
            Rect roi = Rect(p1, p2);
            if(roi.width && roi.height && roiFrame != nullptr)
                *roiFrame = gFrame(roi);
            rectangle(gFrame, rect, CV_RGB(255, 0, 0), 2, 8, 0);
            if(isCenter != nullptr)
                *isCenter = true;
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

QString Camera::callOCR()
{
    QElapsedTimer timer;
    timer.start();
    qDebug() << "OCR Started";
    pyProcess->start("python", {"/home/hdu/chineseocr_lite-onnx/detect_mine.py"});
    pyProcess->waitForFinished(10000);
    qDebug() << "OCR Stoped" << timer.elapsed();

    ocrResultFile->open(QFile::Text | QFile::ReadOnly);
    QString result = ocrResultFile->readAll();
    ocrResultFile->close();
    return result;
}
