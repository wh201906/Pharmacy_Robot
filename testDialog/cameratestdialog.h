#ifndef CAMERATESTDIALOG_H
#define CAMERATESTDIALOG_H

#include "module/camera.h"
#include <QDialog>
#include <opencv.hpp>
#include <opencv2/imgproc/types_c.h>

namespace Ui
{
class CameraTestDialog;
}

class CameraTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraTestDialog(Camera* camera, QWidget *parent = nullptr);
    ~CameraTestDialog();

public slots:
    void onFrameRefreshed();
    void onFrameAddrFetched(cv::Mat* rawAddr, cv::Mat* roiAddr, cv::Mat* roiOfRawAddr);
    void onOCRResultFetched(QString result);
    void onDrugRectFetched(QRect rect);
private slots:
    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

    void on_OCRButton_clicked();

private:
    Ui::CameraTestDialog *ui;
    Camera* camera;
    cv::Mat* rawFrame = nullptr;
    cv::Mat* roiFrame = nullptr;
    cv::Mat* roiOfRawFrame = nullptr;
    QPixmap mat2Pixmap(cv::Mat *mat);
signals:
    void openCam(int id);
    void getFrameAddr();
    void closeCam();
    void getOCRResult();
};

#endif // CAMERATESTDIALOG_H
