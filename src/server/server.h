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
    template<> struct hash<QString>
    {
        std::size_t operator()(const QString& s) const noexcept
        {
            const QChar* str = s.data();
            std::size_t hash = 5381;

            for (int i = 0; i < s.size(); ++i)
                hash = ((hash << 5) + hash) + ((str->row() << 8) | (str++)->cell());

            return hash;
        }
    };
}

namespace Ui {
class Server;
}

class Server : public QMainWindow
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = 0, const QString& path = QDir::currentPath(), quint16 _port = 12345);
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
    std::unique_ptr<FileServer> fileServer;
    std::unique_ptr<FileClient> fileClient;
    std::unordered_map<QString, std::unique_ptr<User>> users;
    std::unique_ptr<FileDialog> fileDialog;
    std::unique_ptr<Ui::Server> ui;
};
