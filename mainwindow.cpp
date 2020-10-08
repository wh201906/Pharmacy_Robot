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
    connect(this, &MainWindow::setLabelBuffer, camera, &Camera::setLabelBuffer);
    qDebug() << getSimilarity("ABCDEFG", "ABCHIJK");
    ui->cameraGroupBox->setVisible(false);
    tableFont = ui->drugTableWidget->font();
    tableFont.setPixelSize(32);
    ui->drugTableWidget->setFont(tableFont);
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
        QMap<QString, QString> requiredDrugInfo;
        ui->drugTableWidget->clear();
        ui->drugTableWidget->setFont(tableFont);
        ui->drugTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("ID"));
        ui->drugTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Name"));
        QString userID = reader->get14aUID().toUpper();
        if(userID == "")
        {
            ui->idLabel->setText("No Card");
            ui->nameLabel->setText("Patient Name:");
            delay(1000);
            continue;
        }
        ui->idLabel->setText("Patient ID:" + userID);
        QList<QByteArray> patientInfo = file2list("/home/hdu/Pharmacy_Robot_RAM/" + userID + ".txt");
        if(patientInfo.size() == 0)
        {
            ui->nameLabel->setText("Name:Not Found");
            continue;
        }
        ui->nameLabel->setText("Patient Name:" + patientInfo[0]);
        for(int i = 1; i < patientInfo.size(); i++)
        {
            if(patientInfo[i].size() < 2)
                continue;
            QList<QByteArray> requiredRawDrugInfo = patientInfo[i].split(',');
            if(requiredRawDrugInfo[0].startsWith('#'))
                continue;
            ui->drugTableWidget->setRowCount(ui->drugTableWidget->rowCount() + 1);
            qDebug() << QString(requiredRawDrugInfo[0]) << QString(requiredRawDrugInfo[1]);
            ui->drugTableWidget->setItem(ui->drugTableWidget->rowCount() - 1, 0, new QTableWidgetItem(QString(requiredRawDrugInfo[0])));
            ui->drugTableWidget->setItem(ui->drugTableWidget->rowCount() - 1, 1, new QTableWidgetItem(QString(requiredRawDrugInfo[1])));
            requiredDrugInfo.insert(requiredRawDrugInfo[0], requiredRawDrugInfo[1]);
        }

        QMap<QString, QString> errorDrugInfo;
        for(int retry = 0; retry < 3; retry++)
        {
            if(!isProcessing)
                break;
            for(QMap<QString, QString>::iterator it = requiredDrugInfo.begin(); it != requiredDrugInfo.end(); it++)
            {
                if(!isProcessing)
                    break;
                QPointF vPoint = gotoPos(it.key());
                if((!callOCR()) || (!getOCRMatchState(it.value())))
                {
                    errorDrugInfo.insert(it.key(), it.value());
                    emit setLabelBuffer("Not Match");
                    continue;
                }
                emit setLabelBuffer(it.key());
                QPointF catchPoint = linearTransform(vPoint, visualRect);
                if(!isProcessing)
                    break;
                //            qDebug() << ui->cameraLabel->pixmap(Qt::ReturnByValue).save("/home/hdu/img/" + ID + ".jpg");
                servoDriver->fetchDrug(catchPoint.x(), catchPoint.y(), 75);
                delay(500);
                emit setLabelBuffer("");
                qDebug() << QString("error info after %1 try").arg(retry + 1) + "\n" << errorDrugInfo;
            }
            if(errorDrugInfo.isEmpty())
                break;
            if(retry < 2)
            {
                requiredDrugInfo = errorDrugInfo;
                errorDrugInfo.clear();
            }
        }
        qDebug() << errorDrugInfo;
        if(!errorDrugInfo.isEmpty())
        {
            requiredDrugInfo = errorDrugInfo;
            errorDrugInfo.clear();
            for(QMap<QString, QPointF>::iterator totalIt = totalDrugInfo.begin(); totalIt != totalDrugInfo.end(); totalIt++)
            {
                if(requiredDrugInfo.isEmpty())
                {
                    emit setLabelBuffer("");
                    break;
                }
                qDebug() << "error info:" << errorDrugInfo << requiredDrugInfo;
                if(!isProcessing)
                    break;
                QPointF vPoint = gotoPos(totalIt.key());
                QMap<QString, QString>::iterator reqIt;
                if(!callOCR())
                {
                    emit setLabelBuffer("Not Match");
                    continue;
                }
                for(reqIt = requiredDrugInfo.begin(); reqIt != requiredDrugInfo.end(); reqIt++)
                {
                    qDebug() << "currDrug:" << reqIt.value() << totalIt.key();
                    if(callOCR() && getOCRMatchState(reqIt.value()))
                    {
                        emit setLabelBuffer(reqIt.key());
                        break;
                    }
                    emit setLabelBuffer("Not Match");
                }
                if(reqIt == requiredDrugInfo.end())
                {
                    continue;
                }
                requiredDrugInfo.remove(reqIt.key());
                emit setLabelBuffer(reqIt.key());
                QPointF catchPoint = linearTransform(vPoint, visualRect);
                if(!isProcessing)
                    break;
                servoDriver->fetchDrug(catchPoint.x(), catchPoint.y(), 75);
                delay(500);
                emit setLabelBuffer("");
            }
        }
        ui->drugTableWidget->clear();
        ui->drugTableWidget->setFont(tableFont);
        ui->drugTableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("ID"));
        ui->drugTableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Name"));
        ui->drugTableWidget->setRowCount(100);
        if(!errorDrugInfo.isEmpty())
            isProcessing = false;
        for(QMap<QString, QString>::iterator it = errorDrugInfo.begin(); it != errorDrugInfo.end(); it++)
        {
            ui->drugTableWidget->setRowCount(ui->drugTableWidget->rowCount() + 1);
            ui->drugTableWidget->setItem(ui->drugTableWidget->rowCount() - 1, 0, new QTableWidgetItem(QString(it.key())));
            ui->drugTableWidget->setItem(ui->drugTableWidget->rowCount() - 1, 1, new QTableWidgetItem(QString(it.value())));
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

bool MainWindow::callOCR()
{
    ocrResult = "";
    emit getOCRResult();
    for(int i = 0; i < 25; i++) // wait for 2500ms
    {
        delay(100);
        if(ocrResult != "")
            break;
    }
    return ocrResult != "";
}

bool MainWindow::getOCRMatchState(const QString& str)
{
    double threshold = 0.6;
    QStringList resultList = ocrResult.split('\n');
    qDebug() << resultList;
    int maxMatchPos = 0;
    double matchVal = 0, maxMatchVal = 0;
    for(int i = 0; i < resultList.size(); i++)
    {
        matchVal = getSimilarity(resultList[i], str);
        if(matchVal > maxMatchVal)
        {
            maxMatchPos = i;
            maxMatchVal = matchVal;
        }
    }
    qDebug() << "match index:" << maxMatchVal;
    qDebug() << "match state: ori:" << str << " res:" << resultList[maxMatchPos];
    return maxMatchVal >= threshold;
}

QPointF MainWindow::gotoPos(const QString& ID)
{
    QPointF vPoint = totalDrugInfo[ID];
    servoDriver->move_goto(vPoint.x(), vPoint.y(), 200);
    servoDriver->move_waitMotionSent();
    servoDriver->move_waitMotionFinished();
    delay(500);
    return vPoint;
}
