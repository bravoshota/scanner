#pragma once

#include <manager.h>
#include <QtCore/QObject>

/**
 * @brief The ManagerDBusInterface is DBus interface for Manager class.
 */
class ManagerDBusInterface: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DBUS_INTERFACE_NAME)

public:
    ManagerDBusInterface(Manager &manager)
        : manager(manager)
    {}

public slots:
    ScannerResults scanBytes(const QByteArray byteArray)
    {
        return manager.scanBytes(byteArray.data(), byteArray.size());
    }

    ScannerResults scanFile(const QString &filename)
    {
        return manager.scanFile(filename.toStdString());
    }

private:
    Manager &manager;
};
