#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QDirIterator>

#include "config.h"
#include "fileserver.h"
#include "fileclient.h"
#include "filedialog.h"

enum state {
    OFFLINE,
    ONLINE
};

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

private:
    void setConfig(Config& cfg);
    void setupModels();
    bool saveUsers();
    bool loadUsers();
    void initTreeModel(QList<QStandardItem*> &items, const QString &ip, const QString &username, const quint16 port, const Config *cfg, const state& st);

    QStandardItemModel* treeModel;
    QDataWidgetMapper* uiMapper;

    QString path;
    quint16 localPort;
    FileServer* fileServer;
    FileClient* fileClient;
    QHash<QString, Config*> usersConfig;
    QHash<QString, QPair<QString, quint16>> usernames;
    FileDialog* fileDialog;
    Ui::Server *ui;
};

#endif // SERVER_H
