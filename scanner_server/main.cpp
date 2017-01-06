#include <interface.h>
#include <QCoreApplication>
#include <QtDBus/QtDBus>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    qDBusRegisterMetaType<ScannerResults>();

    if(!QDBusConnection::sessionBus().isConnected())
    {
        std::cout << "Cannot connect to D_Bus session" << std::endl;
        return 1;
    }

    if (argc < 2)
    {
        std::cout << "please pass sequences file name in arguments" << std::endl;
        return 1;
    }

    Manager scannerManager(argv[1]);
    ManagerDBusInterface wrapper(scannerManager);
    if (QDBusConnection::sessionBus().registerObject(DBUS_PATH, &wrapper,
                                                     QDBusConnection::ExportAllSlots))
    {
        std::cout << "object registered successfully!.." << std::endl;
    }
    else
    {
        std::cout << "object already registered!.." << std::endl;
    }

    if (QDBusConnection::sessionBus().registerService(DBUS_SERVICE_NAME))
    {
        std::cout << "service registered successfully!.." << std::endl;
    }
    else
    {
        std::cout << "Cannot register service: " << DBUS_SERVICE_NAME << std::endl;
        std::cout << "DBus error: "
                  << QDBusConnection::sessionBus().lastError().message().toStdString()
                  << std::endl;
        return 1;
    }

    application.exec();
    return 0;
}

