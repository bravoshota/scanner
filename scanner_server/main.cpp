#include <interface.h>
#include <QCoreApplication>
#include <QtDBus/QtDBus>
#include <iostream>
#include <fstream>

void updateSequencesFromFile(const std::string &filename,
                             std::vector<ByteSequence> &byteSequences)
{
    byteSequences.clear();

    std::cout << "updating byte sequences from file: " << filename << std::endl;

    std::ifstream ifs(filename);
    while (ifs)
    {
        std::string line;
        if (!getline(ifs, line))
        {
            break;
        }

        size_t index = line.find(".{");
        if (index > 0 && index != std::string::npos && line.back() == '}')
        {
            line.pop_back();
            Guid guid = line.substr(index + 2);
            Bytes bytes = line.erase(index);
            byteSequences.push_back({bytes, guid});
        }
    }

    std::cout << "number of byte sequences = " << byteSequences.size() << std::endl;
}

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

    if(!QFile(argv[1]).exists())
    {
        std::cout << "File doesn't exist!" << std::endl;
        return 1;
    }

    std::vector<ByteSequence> byteSequences;
    updateSequencesFromFile(argv[1], byteSequences);
    if (byteSequences.empty())
    {
        std::cout << "Empty sequences or not supported file!" << std::endl;
        return 1;
    }

    Manager scannerManager(std::move(byteSequences));
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

