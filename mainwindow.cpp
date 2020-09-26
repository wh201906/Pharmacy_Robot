#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    faceRecognizer = new FaceRecognizer(this);
    reader = new RFID;
    servoDriver = new ServoDriver;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_camOpenButton_clicked()
{
    faceRecognizer->init({"CPU", "CPU"}, {"/home/hdu/test/models/intel/face-detection-adas-0001/FP32/face-detection-adas-0001.xml", "/home/hdu/test/models/intel/facial-landmarks-35-adas-0002/FP32/facial-landmarks-35-adas-0002.xml"});
}

void MainWindow::on_getFrameButton_clicked()
{
    while(true)
    {
        ui->imgLabel->setPixmap(QPixmap::fromImage(faceRecognizer->getResult()));
        QApplication::processEvents();
    }
}

void MainWindow::on_camCloseButton_clicked()
{
    QApplication::exit();
}

void MainWindow::on_readCardButton_clicked()
{
    qDebug() << reader->get14aUID();
    //RFID::getIDCard_CNUID();
}

void MainWindow::on_saveImageButton_clicked()
{
    qDebug() << "./img/" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".jpg";
    qDebug() << faceRecognizer->getFrame().save("./img/" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".jpg");
}

void MainWindow::on_testButton_clicked()
{
//    cv::Mat res;
//    drug_positioning(faceRecognizer->getRawFrame(), &res);
//    ui->imgLabel->setPixmap(QPixmap::fromImage(faceRecognizer->frame2image(res)));
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
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", "/home/hdu/1.jpg"));
    /*
        PyObject *pReturn = NULL;
        pReturn = PyEval_CallObject(pFunc, pArgs);
    */
    PyEval_CallObject(pFunc, pArgs);
    /*
        QString result;
        PyArg_Parse(pReturn, "s", &result);
        //long result = PyLong_AsLong(pReturn);

    int result;
    PyArg_Parse(pReturn, "i", &result);

    char result[5][50] = {'\0'};
    PyArg_Parse(pReturn, "s", result);
    */
    /*
        PyObject* objectsRepresentation = PyObject_Repr(pReturn);
        const char* result = PyBytes_AsString(objectsRepresentation);
    */
    Py_DecRef(pModule);
    Py_DecRef(pFunc);
    Py_Finalize();

    QFile file("name.txt");
    file.open(QFile::Text | QFile::ReadOnly);
    QString result = file.readAll();
    file.close();
    qDebug() << result;

}

int* MainWindow::drug_positioning(cv::Mat frame, cv::Mat* resultFrame)
{
    double scale = 1;//0.5

    using namespace cv;
    cv::Size ResImgSiz = cv::Size(frame.cols * scale, frame.rows * scale);
    cv::Mat gFrame = cv::Mat(ResImgSiz, frame.type());
    cv::resize(frame, gFrame, ResImgSiz);

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
    ////开运算
    //element = getStructuringElement(MORPH_RECT, Size(5, 4), Point(-1, -1));
    //morphologyEx(close, open_2, 2, element, Point(-1, -1));
    //连通域标记
    cv::Mat labels, stats, centroids;
    int num = cv::connectedComponentsWithStats(close, labels, stats, centroids);
    static int output_rect[4];
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
        //画框
        int x_drug = output_rect[0] = stats.at<int>(max_i, 0);
        int y_drug = output_rect[1] = stats.at<int>(max_i, 1);
        int width_drug = output_rect[2] = stats.at<int>(max_i, 2);
        int height_drug = output_rect[3] = stats.at<int>(max_i, 3);
        //			int area_drug = stats.at<int>(max_i, 4);
        int center_x_drug = x_drug + width_drug / 2;
        int center_y_drug = y_drug + height_drug / 2;
        int center_x_gFrame = gFrame.cols / 2;
        int center_y_gFrame = gFrame.rows / 2;
        char tolerance_x = 60, tolerance_y = 50;
        //			char tolerance_min = 5;
        cv::Rect rect;
        rect.x = x_drug;
        rect.y = y_drug;
        rect.width = width_drug;
        rect.height = height_drug;
        //周长与中心点判断
        //printf(" %d   %d\n", gframe.cols, gframe.rows);
        //printf(" center_x_gframe - tolerance=%d\n", center_x_gframe - tolerance);
        printf(" max_perimeter=%d \n center_x_drug=%d (%d-%d) \n center_y_drug=%d (%d-%d)\n", max_Perimeter, center_x_drug, center_x_gFrame - tolerance_x, center_x_gFrame + tolerance_x, center_y_drug, center_y_gFrame - tolerance_y, center_y_gFrame + tolerance_y);
//        if(max_Perimeter > 100 && center_x_drug > center_x_gFrame - tolerance_x && center_x_drug < center_x_gFrame + tolerance_x && center_y_drug > center_y_gFrame - tolerance_y && center_y_drug < center_y_gFrame + tolerance_y)
        if(max_Perimeter > 100)
        {
            Point p1 = Point(x_drug, y_drug);
            Point p2 = Point(x_drug + width_drug, y_drug + height_drug);
            Rect roi = Rect(p1, p2);
            if(roi.width && roi.height)
            {
                cv::Mat roiImg = gFrame(roi);
                imshow("roi", roiImg);
                imwrite("D:/H D U/VS/007VS/43_drug_detection/chineseocr_lite-onnx/test_imgs/1.jpg", roiImg);
            }

            rectangle(gFrame, rect, CV_RGB(255, 0, 0), 2, 8, 0);
            //printf("rectangle\n");
        }
        //cv::imshow("edgeX_Y_Mat", edgeX_Y_Mat_out);
        //cv::imshow("g_edgeX_Y_Mat", g_edgeX_Y_Mat_out);
        //cv::imshow("close", close);
        //		cv::imshow("edgeX_Mat", edgeX_Mat_out);
        //		cv::imshow("edgeY_Mat", edgeY_Mat_out);
        //cv::imshow("rFrame", rFrame);
        cv::imshow("frame", gFrame);

        //waitKey(1);
    }
    else //只有背景
    {
        output_rect[3] = output_rect[2] = output_rect[1] = output_rect[0] = 0;
    }
    qDebug() << output_rect[3] << output_rect[2] << output_rect[1] << output_rect[0];
    gFrame.copyTo(*resultFrame);
    return output_rect;
}

void MainWindow::on_servoTestButton_clicked()
{
    ServoTestDialog dialog(servoDriver);
    dialog.exec();
}

void MainWindow::on_testGroupBox_clicked(bool checked)
{
    ui->servoTestButton->setVisible(checked);
}

void MainWindow::on_RFIDTestButton_clicked()
{
    RFIDTestDialog dialog(reader);
    dialog.exec();
}
