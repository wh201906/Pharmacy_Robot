#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QThread>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_camOpenButton_clicked();

    void on_getFrameButton_clicked();

    void on_camCloseButton_clicked();

private:
    Ui::MainWindow *ui;
    cv::VideoCapture* cap;
    cv::Mat* videoFrame;
    QImage* imgFrame;
};
#endif // MAINWINDOW_H
