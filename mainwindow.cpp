#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    reader = new RFID;
    servoDriver = new ServoDriver;
    cameraThread = new QThread;
    camera = new Camera(cameraThread);
    cameraThread->start();
    servoTestDialog = new ServoTestDialog(servoDriver);
    servoTestDialog->setModal(false);
    myRFIDTestDialog = new RFIDTestDialog(reader);
    myRFIDTestDialog->setModal(false);
    cameraTestDialog = new CameraTestDialog(camera);
    cameraTestDialog->setModal(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_camOpenButton_clicked()
{
}

void MainWindow::on_getFrameButton_clicked()
{
}

void MainWindow::on_camCloseButton_clicked()
{
}

void MainWindow::on_saveImageButton_clicked()
{

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
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", "/home/hdu/roi.jpg"));
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

void MainWindow::on_testGroupBox_clicked(bool checked)
{
    ui->servoTestButton->setVisible(checked);
}

void MainWindow::on_servoTestButton_clicked()
{
    servoTestDialog->show();
}

void MainWindow::on_RFIDTestButton_clicked()
{
    myRFIDTestDialog->show();
}

void MainWindow::on_cameraTestButton_clicked()
{
    cameraTestDialog->show();
}
