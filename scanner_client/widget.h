#pragma once

#include <QWidget>
#include <../common.h>
#include <QTime>

namespace Ui
{
class ScannerMain;
}

class QDBusPendingCallWatcher;
class QFileSystemModel;
class QStringListModel;
class QDBusInterface;

class ScannerMain : public QWidget
{
    Q_OBJECT
public:
    explicit ScannerMain(QWidget *parent = 0);
    ~ScannerMain();

private:
    Ui::ScannerMain *ui;
    QFileSystemModel *fileSystemModel;
    QStringListModel *resultListModel;
    QDBusInterface *scannerIface;
    QStringList allFiles;
    QStringList filesOutput;
    uint32_t numberInfectedFiles;
    QTime startTime;
    int replyCounter;
    bool busy;

private slots:
    // files tab
    void onScanPushed(bool);
    void scanFileFinished(QDBusPendingCallWatcher *reply);

    // bytes tab
    void onImportFromFilePushed(bool);
    void onScanBytes(bool);
    void scanBytesFinished(QDBusPendingCallWatcher *reply);

private:
    void asyncScanFile();
    void scanRecursivelly(const QString &root);
};
