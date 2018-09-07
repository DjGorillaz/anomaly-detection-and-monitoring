#include "filedialog.h"
#include "ui_filedialog.h"

namespace AnomalyDetection
{
    FileDialog::FileDialog(QWidget *parent) :
        QDialog{parent},
        mask{0},
        ui{std::make_unique<Ui::FileDialog>()}
    {
        ui->setupUi(this);
        ui->lineEdit->setVisible(false);
        QSize size = sizeHint();
        resize(size);
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

    void FileDialog::reset()
    {
        ui->ScreenChBox->setChecked(false);
        ui->LogChBox->setChecked(false);
    }
}
