#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTreeWidgetItem>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    buildProcess =
        new QProcess(
            this
            );
    runProcess =
        new QProcess(
            this
            );

    ui->saveButton
        ->setEnabled(false);

    ui->buildButton
        ->setEnabled(false);

    ui->runButton
        ->setEnabled(false);

    ui->openFolderButton
        ->setEnabled(false);
    ui->statusLabel
        ->setText(
            "Ready • Generate a project"
            );

    manager =
        new QNetworkAccessManager(
            this
            );

    connect(
        manager,
        &QNetworkAccessManager::finished,
        this,
        &MainWindow::handleResponse
        );

    connect(
        buildProcess,

        &QProcess::readyReadStandardOutput,

        this,

        [=]()
        {
            ui->previewPane
                ->appendPlainText(

                    buildProcess
                        ->readAllStandardOutput()

                    );
            ui->previewPane
                ->moveCursor(
                    QTextCursor::End
                    );
        }
        );

    connect(
        buildProcess,

        &QProcess::readyReadStandardError,

        this,

        [=]()
        {
            ui->previewPane
                ->appendPlainText(

                    buildProcess
                        ->readAllStandardError()

                    );
            ui->previewPane
                ->moveCursor(
                    QTextCursor::End
                    );
        }
        );

    connect(
        buildProcess,

        &QProcess::finished,

        this,

        [=]()
        {
            qint64 ms =
                buildStart.msecsTo(
                    QDateTime::currentDateTime()
                    );

            double seconds =
                ms
                /
                1000.0;
            QDir buildDir(
                buildProcess
                    ->workingDirectory()
                +
                "/build"
                );

            builtExecutable.clear();

            QStringList exeList =
                buildDir.entryList(
                    QDir::Files
                    |
                    QDir::Executable
                    );

            for(QString file : exeList)
            {
                if(
                    !file.contains(".")
                    )
                {
                    builtExecutable =
                        buildDir.absoluteFilePath(
                            file
                            );

                    break;
                }
            }

            if(buildProcess->exitCode()==0 && !builtExecutable.isEmpty())
            {
                ui->statusLabel
                    ->setText(

                        QString(
                            "Build successful (%1s) • Ready to run"
                            )
                            .arg(
                                seconds,
                                0,
                                'f',
                                1
                                )

                        );

                ui->buildButton
                    ->setEnabled(
                        true
                        );

                ui->runButton
                    ->setEnabled(
                        true
                        );
            }
            else
            {
                ui->statusLabel
                    ->setText(

                        QString(
                            "Build failed (%1s)"
                            )
                            .arg(
                                seconds,
                                0,
                                'f',
                                1
                                )

                        );

                ui->buildButton
                    ->setEnabled(
                        true
                        );
            }
        }
        );
    connect(
        runProcess,

        &QProcess::finished,

        this,

        [=]()
        {
            ui->runButton
                ->setEnabled(
                    true
                    );
            ui->buildButton
                ->setEnabled(
                    true
                    );

            ui->statusLabel
                ->setText(
                   "Program closed • Ready to run"
                    );
        }
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString prompt =
        ui->promptInput
            ->toPlainText();

    if(
        prompt.isEmpty()
        )
        return;

    sendPrompt(
        prompt
        );
}


void MainWindow::on_fileTree_itemClicked(
    QTreeWidgetItem *item,
    int column
    )
{
    Q_UNUSED(column);

    QString path;

    while(item)
    {
        if(!path.isEmpty())
        {
            path =
                "/"
                +
                path;
        }

        path =
            item->text(0)
            +
            path;

        item =
            item->parent();
    }
    ui->projectLabel
        ->setText(
            path
            );
    ui->previewPane
        ->setPlainText(
            generatedContents
                .value(path)
            );
}
void MainWindow::createProject(
    QString projectName,
    QStringList files
    )
{
    QString basePath =
        QDir::currentPath()
        + "/Generated/"
        + projectName;

    QDir().mkpath(basePath);

    for(QString file : files)
    {
        QString fullPath =
            basePath
            + "/"
            + file;

        QFile output(fullPath);

        QDir().mkpath(
            QFileInfo(fullPath)
                .absolutePath()
            );

        if(output.open(
                QIODevice::WriteOnly
                ))
        {
            QTextStream stream(
                &output
                );

            stream
                << "// Generated by RepoPilot\n";

            output.close();
        }
    }
}
void MainWindow::sendPrompt(
    QString prompt
    )
{
    ui->pushButton->setEnabled(false);
    ui->saveButton
        ->setEnabled(false);

    ui->buildButton
        ->setEnabled(false);

    ui->runButton
        ->setEnabled(false);

    ui->openFolderButton
        ->setEnabled(false);

    ui->previewPane
        ->clear();

    ui->fileTree
        ->clear();

    ui->statusLabel
        ->setText("Generating project...");
    QApplication::setOverrideCursor(
        Qt::WaitCursor
        );
    QNetworkRequest request(
        QUrl(
            "http://127.0.0.1:8000/generate"
            )
        );

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
        );

    QJsonObject body;

    body["prompt"] =
        prompt;

    manager->post(
        request,
        QJsonDocument(body)
            .toJson()
        );
}
void MainWindow::handleResponse(
    QNetworkReply* reply
    )
{
    ui->pushButton
        ->setEnabled(
            true
            );

    if(reply->error())
    {
        ui->statusLabel
            ->setText(
                "REQUEST FAILED"
                );

        ui->previewPane
            ->setPlainText(
                reply->errorString()
                );

        reply->deleteLater();
        QApplication::restoreOverrideCursor();
        return;
    }

    auto json =
        QJsonDocument::fromJson(
            reply->readAll()
            );

    if(
        !json.isObject()
        )
    {
        ui->statusLabel
            ->setText(
                "INVALID RESPONSE"
                );

        reply->deleteLater();
        QApplication::restoreOverrideCursor();
        return;
    }

    ui->fileTree->clear();

    auto obj =
        json.object();

    ui->projectLabel
        ->setText(
            obj["project"]
                .toString()
            );

    generatedContents.clear();

    auto files =
        obj["files"]
            .toArray();

    auto contents =
        obj["contents"]
            .toObject();

    for(
        auto file :
        files
        )
    {
        QString name =
            file.toString();

        addFileToTree(name);

        generatedContents[name]
            =
            contents[name]
                .toString();
    }

    ui->fileTree
        ->expandAll();

    if(
        ui->fileTree
            ->topLevelItemCount()
        >
        0
        )
    {
        auto first =

            ui->fileTree
                ->topLevelItem(
                    0
                    );

        ui->fileTree
            ->setCurrentItem(
                first
                );

        on_fileTree_itemClicked(
            first,
            0
            );
    }

    ui->statusLabel
        ->setText(
            QString(
                "Generated (%1 files) • Save project"
                )
                .arg(
                    generatedContents.size()
                    )
            );
    ui->saveButton
        ->setEnabled(true);

    QApplication::restoreOverrideCursor();
    reply->deleteLater();
}

void MainWindow::on_saveButton_clicked()
{
    QString folder =

        QFileDialog
        ::getExistingDirectory(
            this,
            "Select Save Location"
            );


    if(
        folder.isEmpty()
        )
        return;

    for(
        auto it =
        generatedContents.begin();

        it !=
        generatedContents.end();

        ++it
        )
    {
        QString projectFolder =
            folder
            + "/GeneratedProject";

        QDir().mkpath(projectFolder);

        QString fullPath =
            projectFolder
            + "/"
            + it.key();

        QDir()
            .mkpath(

                QFileInfo(
                    fullPath
                    )
                    .absolutePath()

                );

        QFile file(
            fullPath
            );

        if(
            file.open(
                QIODevice::WriteOnly
                )
            )
        {
            QTextStream out(
                &file
                );

            out
                << it.value();

            file.close();
        }
    }
    QMessageBox::information(
        this,
        "Saved",
        "Project exported successfully."
        );
    savedProjectPath =folder+ "/GeneratedProject";
    lastSavedFolder =savedProjectPath;

    ui->statusLabel
        ->setText(
            QString(
                "Project saved (%1 files) • Ready to build"
                )
                .arg(
                    generatedContents.size()
                    )
            );
    ui->buildButton
        ->setEnabled(true);

    ui->openFolderButton
        ->setEnabled(true);
}


void MainWindow::on_buildButton_clicked()
{

    QString folder =
        QFileDialog
        ::getExistingDirectory(
            this,
            "Select Project Folder"
            );

    if(folder.isEmpty())
        return;

    if(
        buildProcess->state()
        !=
        QProcess::NotRunning
        )
    {
        return;
    }
    buildStart =
        QDateTime::currentDateTime();

    buildProcess
        ->setWorkingDirectory(
            folder
            );

    ui->previewPane
        ->clear();

    ui->statusLabel
        ->setText(
            "Building project..."
            );
    ui->buildButton
        ->setEnabled(
            false
            );

    ui->runButton
        ->setEnabled(
            false
            );

    buildProcess->start(
        "bash",

        QStringList()

            << "-c"

            << R"(

if [ -f CMakeLists.txt ]
then
    mkdir -p build
    cd build
    cmake ..
    make
else
    echo "ERROR: No CMakeLists.txt found"
fi

)"
        );
}


void MainWindow::on_runButton_clicked()
{
    if(
        runProcess->state()
        !=
        QProcess::NotRunning
        )
    {
        ui->statusLabel
            ->setText(
                "Program already running"
                );

        return;
    }
    if(
        builtExecutable.isEmpty()
        )
    {
        ui->statusLabel
            ->setText(
                "BUILD FIRST"
                );

        return;
    }

    if(
        QFile::exists(
            builtExecutable
            )
        )
    {
        ui->runButton
            ->setEnabled(
                false
                );
        ui->buildButton
            ->setEnabled(
                false
                );

        runProcess
            ->start(
                builtExecutable
                );

        ui->statusLabel
            ->setText(
                "Program running..."
                );
    }
    else
    {
        ui->statusLabel
            ->setText(
                "EXECUTABLE NOT FOUND"
                );
    }
}


void MainWindow::on_openFolderButton_clicked()
{
    if(
        lastSavedFolder
            .isEmpty()
        )
        return;

    QDesktopServices::openUrl(

        QUrl::fromLocalFile(
            lastSavedFolder
            )

        );
}

void MainWindow::addFileToTree(QString path)
{
    QStringList parts =
        path.split("/");

    QTreeWidgetItem* parent =
        nullptr;

    QString current;

    for(QString part : parts)
    {
        if(!current.isEmpty())
        {
            current += "/";
        }

        current += part;

        QTreeWidgetItem* found =
            nullptr;

        int count =

                parent

                ?

                parent->childCount()

                :

                ui->fileTree
                    ->topLevelItemCount();

        for(
            int i = 0;
            i < count;
            i++
            )
        {
            QTreeWidgetItem* item =

                    parent

                    ?

                    parent->child(i)

                    :

                    ui->fileTree
                        ->topLevelItem(i);

            if(
                item->text(0)
                ==
                part
                )
            {
                found =
                    item;

                break;
            }
        }

        if(!found)
        {
            found =
                new QTreeWidgetItem(
                    QStringList(part)
                    );

            if(parent)
            {
                parent->addChild(found);
            }
            else
            {
                ui->fileTree
                    ->addTopLevelItem(found);
            }
        }

        parent =
            found;
    }
}