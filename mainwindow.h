#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QString>
#include <QProcess>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_pushButton_clicked();

    void on_fileTree_itemClicked(QTreeWidgetItem *item, int column);
    void createProject(QString projectName,QStringList files);

    void on_saveButton_clicked();

    void on_buildButton_clicked();

    void on_runButton_clicked();

    void on_openFolderButton_clicked();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;

    void sendPrompt(
        QString prompt
        );

    void handleResponse(
        QNetworkReply* reply);
    QMap<QString, QString> generatedContents;
    QString savedProjectPath;
    QString lastSavedFolder;
    QString builtExecutable;
    QProcess* buildProcess;
    QProcess* runProcess;
    QDateTime buildStart;

    void addFileToTree(QString path);
};
#endif // MAINWINDOW_H
