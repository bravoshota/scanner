#include <widget.h>
#include <ui_widget.h>
#include <QStringList>
#include <QDir>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QDBusPendingCallWatcher>
#include <QtDBus/QtDBus>
#include <QFileDialog>
#include <QMessageBox>

ScannerMain::ScannerMain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScannerMain)
    , fileSystemModel(new QFileSystemModel)
    , resultListModel(new QStringListModel)
    , busy(false)
{
    qDBusRegisterMetaType<ScannerResults>();

    ui->setupUi(this);

    // setup slots
    connect(ui->buttonScan, SIGNAL(clicked(bool)), SLOT(onScanPushed(bool)));
    connect(ui->buttonImportFromFile, SIGNAL(clicked(bool)), SLOT(onImportFromFilePushed(bool)));
    connect(ui->buttonScanBytes, SIGNAL(clicked(bool)), SLOT(onScanBytes(bool)));

    // setup file tree
    fileSystemModel->setRootPath(QDir::currentPath());
    ui->treeFiles->setModel(fileSystemModel);
    for (auto i = fileSystemModel->columnCount()-1; i > 0; i --)
    {
        ui->treeFiles->hideColumn(i);
    }
    QStringList currentDirectory = QDir::currentPath().split("/");
    QString str;
    for (auto &val : currentDirectory)
    {
        str += "/" + val;
        ui->treeFiles->setExpanded(fileSystemModel->index(str), true);
    }

    // setup list view for results
    ui->listResults->setModel(resultListModel);

    // setup dbus
    if(!QDBusConnection::sessionBus().isConnected())
    {
        return;
    }

    scannerIface = new QDBusInterface(DBUS_SERVICE_NAME,
                                      DBUS_PATH,
                                      DBUS_INTERFACE_NAME,
                                      QDBusConnection::sessionBus());
}

ScannerMain::~ScannerMain()
{
    delete scannerIface;
    delete ui;
    delete fileSystemModel;
    delete resultListModel;
}

void ScannerMain::onScanPushed(bool)
{
    if (busy)
    {
        QMessageBox::information(0, "", "The scanner is already busy!");
        return;
    }

    scanRecursivelly(fileSystemModel->filePath(ui->treeFiles->currentIndex()));
}

void ScannerMain::scanRecursivelly(const QString &root)
{
    filesOutput.clear();
    filesOutput.push_back("initializing..");
    resultListModel->setStringList(filesOutput);

    allFiles.clear();
    if (QDir(root).exists())
    {
        QDirIterator it(root, QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            allFiles.push_back(it.next());
        }
    }
    else
    {
        if (QFile(root).exists())
        {
            allFiles.push_back(root);
        }
    }

    if (allFiles.count() > 0)
    {
        numberInfectedFiles = 0;
        startTime.start();
        filesOutput[0].append(QString(" %1 files to scan..").arg(allFiles.count()));
        ui->progressBar->setMaximum(allFiles.count());

        if (scannerIface->isValid())
        {
            replyCounter = 0;
            filesOutput.push_back(allFiles[replyCounter]);
            busy = true;
            asyncScanFile();
        }
    }
    else
    {
        filesOutput[0].append(" nothing to scan..");
    }

    resultListModel->setStringList(filesOutput);
}

void ScannerMain::asyncScanFile()
{
    auto reply = scannerIface->asyncCall("scanFile", allFiles[replyCounter]);
    connect(new QDBusPendingCallWatcher(reply),
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(scanFileFinished(QDBusPendingCallWatcher*)));
}

void ScannerMain::scanFileFinished(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError())
    {
        filesOutput.push_back("ABORTING because of error:");
        filesOutput.push_back(watcher->error().message());
        busy = false;
    }
    else
    {
        replyCounter ++;
        if (replyCounter > 0)
        {
            QDBusPendingReply<ScannerResults> error = *watcher;
            ScannerResults scannerResults = error.value();
            QString &lastString = filesOutput[filesOutput.size() - 1];

            if (scannerResults.error == ResultError::SUCCESS)
            {
                if (scannerResults.results.empty())
                {
                    lastString.append(".. [OK]");
                }
                else
                {
                    numberInfectedFiles ++;
                    lastString.append(".. [INFECTED!]");
                    size_t counter = 0;
                    for (auto &val : scannerResults.results)
                    {
                        filesOutput.push_back(QString(">>>> %1. Found sequence with guid = %2")
                                              .arg(++counter)
                                              .arg(QString::fromStdString(val)));
                    }
                }
            }
            else
            {
                lastString.append(QString(".. internal error: %1").arg(asString(scannerResults.error)));
                busy = false;
            }
        }

        if (replyCounter < allFiles.count())
        {
            if (scannerIface->isValid())
            {
                asyncScanFile();
                filesOutput.push_back(allFiles[replyCounter]);
            }
            else
            {
                filesOutput.push_back("The server has stopped - aborted!..");
                busy = false;
            }
        }
        else
        {
            auto totalSeconds = startTime.elapsed()/1000;
            int64_t totalSizeScanned = 0;
            for (auto &val : allFiles)
            {
                totalSizeScanned += QFile(val).size();
            }
            filesOutput.push_back(QString("*** Scan of %1 files finished!..")
                                  .arg(allFiles.size()));
            filesOutput.push_back(QString("*** Number of infected files: %1")
                                  .arg(numberInfectedFiles));
            filesOutput.push_back(QString("*** Total size scanned: %1 MB")
                                  .arg(totalSizeScanned/1024/1024));
            filesOutput.push_back(QString("*** Elapsed time: %1 sec")
                                  .arg(totalSeconds));
            busy = false;
        }
    }

    watcher->deleteLater();

    resultListModel->setStringList(filesOutput);
    ui->listResults->scrollToBottom();
    ui->progressBar->setValue(replyCounter);
}

void ScannerMain::onImportFromFilePushed(bool)
{
    QString filename = QFileDialog::getOpenFileName(this, "Open bytes file", "", "All Files (*.*)");
    QFile file(filename);
    if (file.open(QFile::ReadOnly))
    {
        ui->textBytes->setText(QString::fromLatin1(file.readAll()));
    }
    else
    {
        QMessageBox::information(0, "error", file.errorString());
    }
}

void ScannerMain::onScanBytes(bool)
{
    if (ui->textBytes->toPlainText().isEmpty())
    {
        QMessageBox::information(0, "", "Nothing to check - empty text edit!");
        return;
    }

    if (busy)
    {
        QMessageBox::information(0, "", "The scanner is already busy!");
        return;
    }
    busy = true;

    auto reply = scannerIface->asyncCall("scanBytes", ui->textBytes->toPlainText().toLatin1());
    connect(new QDBusPendingCallWatcher(reply),
            SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(scanBytesFinished(QDBusPendingCallWatcher*)));
}

void ScannerMain::scanBytesFinished(QDBusPendingCallWatcher *watcher)
{
    if (watcher->isError())
    {
        ui->labelBytes->setText(QString("Error: ") + watcher->error().message());
    }
    else
    {
        QDBusPendingReply<ScannerResults> error = *watcher;
        ScannerResults scannerResults = error.value();

        if (scannerResults.error == ResultError::SUCCESS)
        {
            if (scannerResults.results.empty())
            {
                ui->labelBytes->setText("Clean!");
            }
            else
            {
                QString stringCollected;
                stringCollected += "Found following sequences: ";
                for (auto &val : scannerResults.results)
                {
                    stringCollected += QString::fromStdString(val) + "; ";
                }
                ui->labelBytes->setText(stringCollected);
            }
        }
        else
        {
            ui->labelBytes->setText(QString("Internal error: %1").arg(asString(scannerResults.error)));
        }
    }

    watcher->deleteLater();
    busy = false;
}
