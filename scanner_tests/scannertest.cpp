#include <manager.h>
#include <QString>
#include <QtTest>
#include <fstream>
#include <cstdio>

class ScannerTest : public QObject
{
    Q_OBJECT
public:
    ScannerTest();

private Q_SLOTS:
    void testScanFile();
};

ScannerTest::ScannerTest()
{
}

void ScannerTest::testScanFile()
{
    std::string filename = "sequences.tmp";
    uint32_t chunkSize = 50u;
    std::string bytes = "~some@ seq!ueNce12";
    std::string guid = "concrete_guid";
    std::vector<ByteSequence> byteSequences{{bytes, guid}};
    Manager manager(std::move(byteSequences));
    manager.setChunkSize(chunkSize);

    auto fileSize = 2*chunkSize + bytes.size();
    qDebug() << "File size will be " << fileSize << " bytes";

    for (auto i = 49u; i < 2*chunkSize; i ++)
    {
        qDebug() << "Iteration #" << i;

        std::string fileContent;
        fileContent.insert(0, fileSize, '.');

        for (auto j = 0u; j < bytes.size(); j ++)
        {
            fileContent[i+j] = bytes[j];
        }

        std::ofstream ofs(filename);
        ofs << fileContent;
        ofs.close();

        ScannerResults results = manager.scanFile(filename);
        QVERIFY2(std::remove(filename.c_str()) == 0, "File remove error!");
        QVERIFY2(results.error == ResultError::SUCCESS, "Not SUCCESS");
        QVERIFY2(results.results.size() == 1, "size should be 1");
    }
}

QTEST_APPLESS_MAIN(ScannerTest)

#include "scannertest.moc"
