#include "cameratestdialog.h"
#include "ui_cameratestdialog.h"

CameraTestDialog::CameraTestDialog(Camera* camera, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraTestDialog)
{
    ui->setupUi(this);
    this->camera = camera;
    connect(this, &CameraTestDialog::openCam, camera, &Camera::openCam);
    connect(this, &CameraTestDialog::getFrameAddr, camera, &Camera::getFrameAddr);
    connect(this, &CameraTestDialog::closeCam, camera, &Camera::closeCam);
    connect(this, &CameraTestDialog::setOCRState, camera, &Camera::setOCRState);
    connect(camera, &Camera::frameRefreshed, this, &CameraTestDialog::onFrameRefreshed);
    connect(camera, &Camera::frameAddr, this, &CameraTestDialog::onFrameAddrFetched);
    connect(camera, &Camera::OCRResult, this, &CameraTestDialog::onOCRResultFetched);
    connect(camera, &Camera::drugRect, this, &CameraTestDialog::onDrugRectFetched);
}

CameraTestDialog::~CameraTestDialog()
{
    delete ui;
}

void CameraTestDialog::onDrugRectFetched(QRect rect)
{
    qDebug() << rect;
}

void CameraTestDialog::onOCRResultFetched(QString result)
{
    qDebug() << result;
}

void CameraTestDialog::onFrameRefreshed()
{
    if(rawFrame != nullptr)
    {
        ui->rawFrameLabel->setPixmap(mat2Pixmap(rawFrame));
    }
    if(roiFrame != nullptr)
    {
        ui->roiFrameLabel->setPixmap(mat2Pixmap(roiFrame));
    }
    if(roiOfRawFrame != nullptr)
    {
        ui->roiOfRawFrameLabel->setPixmap(mat2Pixmap(roiOfRawFrame));
    }
}

void CameraTestDialog::onFrameAddrFetched(cv::Mat* rawAddr, cv::Mat* roiAddr, cv::Mat* roiOfRawAddr)
{
    this->rawFrame = rawAddr;
    this->roiFrame = roiAddr;
    this->roiOfRawFrame = roiOfRawAddr;
}

void CameraTestDialog::on_connectButton_clicked()
{
    emit openCam(ui->camIDEdit->text().toInt());
    emit getFrameAddr();
}

void CameraTestDialog::on_disconnectButton_clicked()
{
    emit closeCam();
}

QPixmap CameraTestDialog::mat2Pixmap(cv::Mat* mat)
{
    return QPixmap::fromImage(QImage((const unsigned char*)mat->data, mat->cols, mat->rows, mat->step, QImage::Format_RGB888).rgbSwapped());
}

void CameraTestDialog::on_OCRCheckBox_stateChanged(int arg1)
{
    emit setOCRState(arg1 == Qt::Checked);
    emit getFrameAddr();
}
