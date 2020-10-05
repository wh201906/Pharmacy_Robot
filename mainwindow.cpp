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
    cameraFrame = new cv::Mat;
    connect(camera, &Camera::drugRect, this, &MainWindow::onDrugRectFetched);
    connect(this, &MainWindow::getFrameAddr, camera, &Camera::getFrameAddr);
    connect(camera, &Camera::frameRefreshed, this, &MainWindow::onFrameRefreshed);
    connect(camera, &Camera::frameAddr, this, &MainWindow::onFrameAddrFetched);
    connect(camera, &Camera::OCRResult, this, &MainWindow::onOCRResultFetched);
    connect(this, &MainWindow::getOCRResult, camera, &Camera::getOCRResult);
    qDebug() << getSimilarity("ABCDEFG", "ABCHIJK");
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOCRResultFetched(QString result)
{
    ocrResult = result;
}

void MainWindow::onFrameRefreshed()
{
    if(cameraFrame != nullptr)
    {
        ui->cameraLabel->setPixmap(mat2Pixmap(cameraFrame));
    }
}

QPixmap MainWindow::mat2Pixmap(cv::Mat* mat)
{
    return QPixmap::fromImage(QImage((const unsigned char*)mat->data, mat->cols, mat->rows, mat->step, QImage::Format_RGB888).rgbSwapped());
}

void MainWindow::onFrameAddrFetched(cv::Mat* rawAddr, cv::Mat* roiAddr, cv::Mat* roiOfRawAddr)
{
    this->cameraFrame = rawAddr;
}

void MainWindow::onDrugRectFetched(QRect rect)
{
    visualRect = rect;
}

void MainWindow::on_testGroupBox_clicked(bool checked)
{
    ui->servoTestButton->setVisible(checked);
    ui->RFIDTestButton->setVisible(checked);
    ui->cameraTestButton->setVisible(checked);
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

void MainWindow::on_startButton_clicked()
{
    isProcessing = true;
    while(isProcessing)
    {
        totalDrugInfo = file2drugInfo("/home/hdu/Pharmacy_Robot_RAM/drugInfo.txt");
        QStringList requiredDrugID;
        ui->drugListWidget->clear();
        QString userID = reader->get14aUID().toUpper();
        if(userID == "")
        {
            ui->idLabel->setText("No Card");
            ui->nameLabel->setText("Name:");
            delay(1000);
            continue;
        }
        ui->idLabel->setText("ID:" + userID);
        QList<QByteArray> patientInfo = file2list("/home/hdu/Pharmacy_Robot_RAM/" + userID + ".txt");
        if(patientInfo.size() == 0)
        {
            ui->nameLabel->setText("Name:Not Found");
            continue;
        }
        ui->nameLabel->setText("Name:" + patientInfo[0]);
        for(int i = 1; i < patientInfo.size(); i++)
        {
            if(patientInfo[i].size() == 0)
                continue;
            QList<QByteArray> requiredDrugInfo = patientInfo[i].split(',');
            if(requiredDrugInfo[0].startsWith('#'))
                continue;
            ui->drugListWidget->addItem(requiredDrugInfo[1]);
            requiredDrugID.append(requiredDrugInfo[0]);
        }
        for(QString ID : requiredDrugID)
        {
            if(!isProcessing)
                break;
            QPointF vPoint = totalDrugInfo[ID];
            servoDriver->move_goto(vPoint.x(), vPoint.y(), 200);
            servoDriver->move_waitMotionSent();
            servoDriver->move_waitMotionFinished();
            delay(500);
            ocrResult = "";
            emit getOCRResult();
            for(int i = 0; i < 15; i++)
            {
                delay(100);
                if(ocrResult != "")
                    break;
            }
            if(ocrResult == "")
                continue;
            else
            {
                QStringList resultList = ocrResult.split('\n');
                qDebug() << resultList;
            }
            QPointF catchPoint = linearTransform(vPoint, visualRect);
            if(!isProcessing)
                break;
//            qDebug() << ui->cameraLabel->pixmap(Qt::ReturnByValue).save("/home/hdu/img/" + ID + ".jpg");
//            servoDriver->fetchDrug(catchPoint.x(), catchPoint.y(), 65);
//            delay(500);
        }
    }
}

QList<QByteArray> MainWindow::file2list(QString path)
{
    QList<QByteArray> result;
    QFile file(path);
    if(!file.open(QFile::Text | QFile::ReadOnly))
        return result;
    result = file.readAll().split('\n');
    return result;
}

QMap<QString, QPointF> MainWindow::file2drugInfo(QString path)
{
    QStringList currItem;
    QMap<QString, QPointF> result;
    QList<QByteArray> lineList = file2list(path);
    if(lineList.size() == 0)
        return result;
    for(QString line : lineList)
    {
        if(line.length() == 0)
            continue;
        currItem = line.split(',');
        result.insert(currItem[0], QPointF(currItem[2].toDouble(), currItem[3].toDouble()));
    }
    return result;
}

QPointF MainWindow::linearTransform(QPointF vPoint, QRect vRect)
{
    const double coe1[7] = {-0.693556148, 1.0010192, -0.000878111, 0.006579999, 0.061800669, 0.009361325, -0.114309532};
    const double coe2[7] = {57.33135, 0.000437, 1.000207, -0.36999, 0.006473, -0.21877, 0.027652};
    QPointF result;
    result.setX(coe1[0] + vPoint.x()*coe1[1] + vPoint.y()*coe1[2] + vRect.x()*coe1[3] + vRect.y()*coe1[4] + vRect.width()*coe1[5] + vRect.height()*coe1[6]);
    result.setY(coe2[0] + vPoint.x()*coe2[1] + vPoint.y()*coe2[2] + vRect.x()*coe2[3] + vRect.y()*coe2[4] + vRect.width()*coe2[5] + vRect.height()*coe2[6]);
    qDebug() << vPoint << vRect << result;
    return result;
}

void MainWindow::delay(int ms)
{
    QTime targetTime = QTime::currentTime().addMSecs(ms);
    QTime currTime = QTime::currentTime();
    while(currTime < targetTime)
    {
        QApplication::processEvents();
        currTime = QTime::currentTime();
    }
}

void MainWindow::on_cameraGroupBox_clicked(bool checked)
{
    ui->cameraLabel->setVisible(checked);
}

void MainWindow::on_stopButton_clicked()
{
    isProcessing = false;
    servoDriver->move_stop();
    servoDriver->rotate_stopSuck();
}


//double MainWindow::on

double MainWindow::getSimilarity(const QString& str1, const QString& str2)
{
    quint64 multiply = 0, freq1 = 0, freq2 = 0;
    QSet<QChar> allCharSet;
    for(QChar ch : str1)
        allCharSet.insert(ch);
    for(QChar ch : str2)
        allCharSet.insert(ch);

    for(QChar ch : allCharSet)
    {
        int c1 = str1.count(ch);
        int c2 = str2.count(ch);
        multiply += c1 * c2;
        freq1 += c1 * c1;
        freq2 += c2 * c2;
    }
    double res = multiply / sqrt(freq1) / sqrt(freq2);
    return res;
}
