#pragma once

#include <memory>

#include <QDialog>

#include "enums.h"

namespace Ui {
class FileDialog;
}

class FileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileDialog(QWidget *parent = 0);
    ~FileDialog();

    uint& getFileMask();
    QString& getFileString();
    void reset();

private:
    uint mask;
    QString files;
    std::unique_ptr<Ui::FileDialog> ui;
};
