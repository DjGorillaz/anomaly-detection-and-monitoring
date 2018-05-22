#include "filedialog.h"
#include "ui_filedialog.h"

FileDialog::FileDialog(QWidget *parent) :
    QDialog{parent},
    mask{0},
    ui{std::make_unique<Ui::FileDialog>()}
{
    ui->setupUi(this);
    ui->chromePassChBox->setVisible(false);
    ui->lineEdit->setVisible(false);
    QSize size = sizeHint();
    resize(size);
}

FileDialog::~FileDialog()
{ }

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
