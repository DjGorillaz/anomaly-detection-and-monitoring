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

quint16& FileDialog::getFileMask()
{
    mask |= ui->ScreenChBox->checkState() ? int(Files::Screen) : 0 ;
    mask |= ui->LogChBox->checkState() ? int(Files::Log) : 0 ;
    return mask;
}

QString& FileDialog::getFileString()
{
   files = ui->lineEdit->text();
   return files;
}
