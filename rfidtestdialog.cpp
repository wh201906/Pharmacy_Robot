#include "rfidtestdialog.h"
#include "ui_rfidtestdialog.h"

RFIDTestDialog::RFIDTestDialog(RFID* reader, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RFIDTestDialog)
{
    ui->setupUi(this);
    this->reader = reader;
}

RFIDTestDialog::~RFIDTestDialog()
{
    delete ui;
}

void RFIDTestDialog::on_connectButton_clicked()
{
    qDebug() << reader->init(ui->portEdit->text());
}

void RFIDTestDialog::on_readButton_clicked()
{
    ui->IDLabel->setText(reader->get14aUID());
}
