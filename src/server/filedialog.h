#pragma once

#include <memory>

#include <QDialog>

#include "enums.h"

namespace AnomalyDetection
{
    namespace Ui {
    class FileDialog;
    }

    class FileDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit FileDialog(QWidget *parent = nullptr);
        ~FileDialog() = default;

        uint& getFileMask();
        QString& getFileString();
        void reset();

    private:
        uint mask;
        QString files;
        std::unique_ptr<Ui::FileDialog> ui;
    };
}
