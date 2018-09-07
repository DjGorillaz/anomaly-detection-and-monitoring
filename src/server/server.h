#pragma once

#include <QMainWindow>
#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QDirIterator>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"
#include "filedialog.h"
#include "user.h"
#include "enums.h"

namespace std
{
    template<>
    struct hash<QString>
    {
        std::size_t operator()(const QString& str) const noexcept
        {
            return qHash(str);
        }
    };
}

namespace AnomalyDetection
{
    namespace Ui {
    class Server;
    }

    class Server : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit Server(QWidget *parent = nullptr, const quint16& port = 12345, const QString& path = QDir::currentPath());
        ~Server();

    private slots:
        void getString(const QString str, const QString ip);
        void configSendClicked();
        void configSaveClicked();
        void fileDialogClicked();
        void fileDialogAccepted();
        void calculateClicked();

    private:
        void setConfig(Config& cfg);
        void setData(User& user);
        void setupModels();
        bool saveUsers();
        bool loadUsers();
        void addUserToModel(const User& user, const State& st);
        void setStatus(const State &status, const QString& ip);
        void setEnabledUi(bool b);
        void loadCombobox(int& row);
        void setupUserConnections(User &user);
        void showFeatures(const QVector<double>& features);
        void showResults(const QVector<double>& results);

        std::unique_ptr<QStandardItemModel> treeModel;
        std::unique_ptr<QDataWidgetMapper> uiMapper;

        QString path;
        quint16 localPort;
        std::unique_ptr<AnomalyDetection::FileLib::FileServer> fileServer;
        std::unique_ptr<AnomalyDetection::FileLib::FileClient> fileClient;
        std::unordered_map<QString, std::unique_ptr<User>> users;
        std::unique_ptr<FileDialog> fileDialog;
        std::unique_ptr<Ui::Server> ui;
    };
}
