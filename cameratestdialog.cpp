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
    connect(camera, &Camera::frameRefreshed, this, &CameraTestDialog::onFrameRefreshed);
    connect(camera, &Camera::frameAddr, this, &CameraTestDialog::onFrameAddrFetched);
}

CameraTestDialog::~CameraTestDialog()
{
    delete ui;
}

void CameraTestDialog::onFrameRefreshed()
{
    if(rawFrame != nullptr)
    {
        ui->rawFrameLabel->setPixmap(QPixmap::fromImage(QImage((const unsigned char*)rawFrame->data, rawFrame->cols, rawFrame->rows, rawFrame->step, QImage::Format_RGB888).rgbSwapped()));
    }
}

void CameraTestDialog::onFrameAddrFetched(cv::Mat* addr)
{
    this->rawFrame = addr;
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
