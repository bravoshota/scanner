/**
  * @file common.h
  * @brief common types for both server and client projects.
  */

#pragma once

#include <set>
#include <string>
#include <functional>
#include <QString>
#include <QMetaType>
#include <QDBusArgument>
#include <QDataStream>

// DBus connection strings
#define DBUS_SERVICE_NAME "test.comodo.scanner"
#define DBUS_PATH "/"
#define DBUS_INTERFACE_NAME "test.comodo.scanner.main"

typedef std::string Bytes;
typedef std::string Guid;

enum class ResultError : uint8_t
{
    SUCCESS = 0,
    CAN_NOT_OPEN_FILE = 1,
    SEEK_ERROR = 2,
};

inline const char* asString(const ResultError val)
{
    switch (val)
    {
    case ResultError::SUCCESS:
        return "SUCCESS";
    case ResultError::CAN_NOT_OPEN_FILE:
        return "CAN_NOT_OPEN_FILE";
    case ResultError::SEEK_ERROR:
        return "SEEK_ERROR";
    }
    return "";
}

/**
 * @brief The ScannerResults struct is used for returning results
 * from scanner server.
 *
 * It is registered as DBus meta type with its serialize/deserialize.
 */
struct ScannerResults
{
    ScannerResults()
        : error(ResultError::SUCCESS)
    {}
    ScannerResults(ResultError error, std::set<Guid> &&results)
        : error(error)
        , results(results)
    {}
    ResultError error;
    std::set<Guid> results;
};

Q_DECLARE_METATYPE(ScannerResults)

inline QDBusArgument &operator<<(QDBusArgument &argument, const ScannerResults &val)
{
    argument.beginStructure();

    argument << static_cast<uint8_t>(val.error);

    argument << val.results.size();

    QByteArray byteArray;
    QDataStream dataStream(&byteArray, QIODevice::WriteOnly);
    for (const auto &guid : val.results)
    {
        dataStream << QString::fromStdString(guid);
    }
    argument << byteArray;

    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, ScannerResults &val)
{
    argument.beginStructure();

    uint8_t error;
    argument >> error;
    val.error = static_cast<ResultError>(error);

    size_t size;
    argument >> size;

    QByteArray byteArray;
    argument >> byteArray;
    QDataStream dataStream(byteArray);
    for (size_t i = 0; i < size; i ++)
    {
        QString guid;
        dataStream >> guid;
        val.results.insert(guid.toStdString());
    }

    argument.endStructure();

    return argument;
}
