#ifndef CAMERATESTDIALOG_H
#define CAMERATESTDIALOG_H

#include <QDialog>
#include "module/camera.h"
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
    void onFrameAddrFetched(cv::Mat* addr);
private slots:
    void on_connectButton_clicked();

    void on_disconnectButton_clicked();

private:
    Ui::CameraTestDialog *ui;
    Camera* camera;
    cv::Mat* rawFrame = nullptr;
signals:
    void openCam(int id);
    void getFrameAddr();
    void closeCam();
};

#endif // CAMERATESTDIALOG_H
