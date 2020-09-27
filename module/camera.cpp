#include "camera.h"

Camera::Camera(QThread* thread, QObject *parent) : QObject(parent)
{
    this->thread = thread;
    moveToThread(thread);
    cam = new cv::VideoCapture;
    rawFrame = new cv::Mat;
    roiFrame = new cv::Mat;
    roiOfRawFrame = new cv::Mat;
    roiFile = new QFile("/home/hdu/Pharmacy_Robot/roi.jpg");
    ocrResultFile = new QFile("/home/hdu/Pharmacy_Robot/ocr.txt");
    refreshTimer = new QTimer();
    refreshTimer->moveToThread(thread);
    refreshTimer->setInterval(200);
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
    if(ocrState)
        getOCRResult();
    emit frameRefreshed();
}

void Camera::closeCam()
{
    refreshTimer->stop();
    cam->release();
}

void Camera::getFrameAddr()
{
    if(ocrState)
        emit frameAddr(rawFrame, roiFrame, roiOfRawFrame);
    else
        emit frameAddr(rawFrame, nullptr, nullptr);
}

void Camera::getOCRResult()
{
    bool openFileResult = false;
    bool saveImageResult = false;
    bool isCenter = false;
    QRect res = drug_positioning(rawFrame, roiFrame, roiOfRawFrame, &isCenter);
    if(!isCenter)
        return;
    openFileResult = roiFile->open(QFile::WriteOnly | QFile::Unbuffered | QFile::Truncate);
    if(!openFileResult)
        return;
    saveImageResult = QImage((const unsigned char*)rawFrame->data, rawFrame->cols, rawFrame->rows, rawFrame->step, QImage::Format_RGB888).rgbSwapped().save(roiFile);
    if(!saveImageResult)
        return;
    roiFile->close();
    qDebug() << res.width() << isCenter << openFileResult << saveImageResult;
    emit OCRResult(callOCR());
}

QRect Camera::drug_positioning(cv::Mat* frame, cv::Mat* roiFrame, cv::Mat* resultFrame, bool* isCenter)
{
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

        int center_x_drug = x_drug + width_drug / 2;
        int center_y_drug = y_drug + height_drug / 2;
        int center_x_gFrame = gFrame.cols / 2;
        int center_y_gFrame = gFrame.rows / 2;
        char tolerance_x = 60, tolerance_y = 50;

        cv::Rect rect;
        rect.x = x_drug;
        rect.y = y_drug;
        rect.width = width_drug;
        rect.height = height_drug;
        if(max_Perimeter > 100 && center_x_drug > center_x_gFrame - tolerance_x && center_x_drug < center_x_gFrame + tolerance_x && center_y_drug > center_y_gFrame - tolerance_y && center_y_drug < center_y_gFrame + tolerance_y)
//            if(max_Perimeter > 100)
        {
            Point p1 = Point(x_drug, y_drug);
            Point p2 = Point(x_drug + width_drug, y_drug + height_drug);
            Rect roi = Rect(p1, p2);
            if(roi.width && roi.height && roiFrame != nullptr)
                *roiFrame = gFrame(roi);
            rectangle(gFrame, rect, CV_RGB(255, 0, 0), 2, 8, 0);
            *isCenter = true;
        }
    }
    else //只有背景
    {
        res.setX(0);
        res.setY(0);
        res.setWidth(0);
        res.setHeight(0);
    }
    qDebug() << res;
    if(resultFrame != nullptr)
        gFrame.copyTo(*resultFrame);
    return res;
}

QString Camera::callOCR()
{
    Py_Initialize();
    if(!Py_IsInitialized())
    {
        qDebug() << "cannot open python!";
    }
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/home/hdu/chineseocr_lite-onnx')");

    PyObject *pModule, *pFunc, *pArgs, *pDict;
    pModule = PyImport_ImportModule("detect_mine");
    if(!pModule)
    {
        qDebug() << "cannot open python file!";
    }
    pDict = PyModule_GetDict(pModule);
    if(!pDict)
    {
        qDebug() << "cannot find dictionary!";
    }
    pFunc = PyDict_GetItemString(pDict, "detect_ocr");
    if(!pFunc || !PyCallable_Check(pFunc))
    {
        qDebug() << "cannot find function!";
    }
    pFunc = PyObject_GetAttrString(pModule, "detect_ocr");

    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", "/home/hdu/Pharmacy_Robot/roi.jpg"));
    PyEval_CallObject(pFunc, pArgs);

//    Py_DecRef(pModule);
//    Py_DecRef(pFunc);
    Py_Finalize();

    ocrResultFile->open(QFile::Text | QFile::ReadOnly);
    QString result = ocrResultFile->readAll();
    ocrResultFile->close();
    return result;
}

void Camera::setOCRState(bool enabled)
{
    ocrState = enabled;
}
