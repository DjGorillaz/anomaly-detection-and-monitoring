#include "filedialog.h"
#include "ui_filedialog.h"

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    mask(0),
    ui(new Ui::FileDialog)
{
    ui->setupUi(this);
    ui->chromePassChBox->setVisible(false);
    ui->lineEdit->setVisible(false);
    QSize size = sizeHint();
    resize(size);
}

FileDialog::~FileDialog()
{
    delete ui;
}

uint& FileDialog::getFileMask()
{
    mask |= ui->ScreenChBox->checkState() ? uint(Files::Screen) : 0 ;
    mask |= ui->LogChBox->checkState() ? uint(Files::Log) : 0 ;
    return mask;
}

QString& FileDialog::getFileString()
{
   files = ui->lineEdit->text();
   return files;
}
