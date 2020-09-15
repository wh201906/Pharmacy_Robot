#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QDateTime>

#include "module/rfid.h"
#include "module/facerecognizer.h"

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

    void on_readCardButton_clicked();

    void on_saveImageButton_clicked();

private:
    Ui::MainWindow *ui;
    FaceRecognizer* faceRecognizer;
    RFID* reader;
};
#endif // MAINWINDOW_H
