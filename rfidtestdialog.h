#ifndef RFIDTESTDIALOG_H
#define RFIDTESTDIALOG_H

#include <QDialog>
#include <QDebug>
#include "module/rfid.h"

namespace Ui
{
class RFIDTestDialog;
}

class RFIDTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RFIDTestDialog(RFID *reader, QWidget *parent = nullptr);
    ~RFIDTestDialog();

private slots:
    void on_connectButton_clicked();

    void on_readButton_clicked();

private:
    Ui::RFIDTestDialog *ui;
    RFID* reader;
};

#endif // RFIDTESTDIALOG_H
