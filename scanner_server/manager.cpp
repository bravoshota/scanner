#include <manager.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <cassert>
#include <vector>

namespace
{
std::string generateOutput(const ScannerResults &scannerResults)
{
    std::stringstream resultString;
    if (scannerResults.results.empty())
    {
        resultString << "OK";
    }
    else
    {
        resultString << "NOT CLEAN!";
        size_t counter = 0;
        for (auto &val : scannerResults.results)
        {
            resultString << std::endl << " " << ++counter << ". Found sequence with guid = " << val;
        }
    }
    return resultString.str();
}
}

Manager::Manager(std::vector<ByteSequence> &&byteSequences)
    : m_byteSequences(byteSequences)
{
    if (m_byteSequences.size() == 0)
    {
        assert(false);
        std::cout << "zero sequences - initialization finished!" << std::endl;
        return;
    }

    unsigned cores = std::min(std::thread::hardware_concurrency(),
                              m_byteSequences.size());
    assert(cores > 0);
    std::cout << "number of cores = " << cores << std::endl;

    //sort array such that first was the longest bytes array
    std::sort(m_byteSequences.begin(), m_byteSequences.end(),
              [](const ByteSequence &a, const ByteSequence &b)->bool
    {
        return a.size() > b.size();
    });
    std::cout << "byte arrays initialized" << std::endl;

    // create groups of byte arrays such way that total size of array sums
    // in different groups were more or less equal.
    m_scannersPool.resize(cores);
    for (size_t index = 0; index < m_byteSequences.size(); index ++)
    {
        unsigned minimalGroup = std::numeric_limits<unsigned>::max();
        unsigned minimalSize = std::numeric_limits<unsigned>::max();
        for (unsigned i = 0; i < cores; i ++)
        {
            std::vector<ByteSequence> &currentGroup = m_scannersPool[i].byteSequences;
            unsigned totalSize = 0;
            for (auto it : currentGroup)
            {
                totalSize += it.size();
            }

            if (minimalSize > totalSize)
            {
                minimalSize = totalSize;
                minimalGroup = i;
            }
        }

        assert(minimalGroup != std::numeric_limits<unsigned>::max());
        m_scannersPool[minimalGroup].byteSequences.push_back(m_byteSequences[index]);
    }

    // printout grouping results
    std::cout << "created scanner pool in size = " << cores << ": " << std::endl;
    for (size_t i = 0; i < cores; i ++)
    {
        std::vector<ByteSequence> &current = m_scannersPool[i].byteSequences;
        uint64_t totalSize = 0;
        for (const auto &val : current)
        {
            totalSize += val.size();
        }
        std::cout << i << ": arrays = " << current.size() << ", total size = " << totalSize << std::endl;
    }

    setChunkSize(16*1024*1024); // default: 16 MB
}

ScannerResults Manager::scanBytes(const void *firstByte, uint64_t sizeInBytes)
{
    std::cout << "scanning memory block of size " << sizeInBytes << " bytes.. ";
    ScannerResults results = scanMemoryBlock({firstByte, sizeInBytes});
    std::cout << generateOutput(results) << std::endl;
    return results;
}

ScannerResults Manager::scanFile(const std::string &filename)
{
    std::cout << "scanning file: " << filename << ".. ";

    ScannerResults resultsCollector;
    char *buffer = new char[readSize];
    bool readMore = true;
    FILE *file = nullptr;
    uint32_t counter = 0;

    auto destroyAndExit = [&](const std::string &outputString)
    {
        if (file != nullptr)
        {
            fclose(file);
        }
        if (buffer != nullptr)
        {
            delete [] buffer;
        }
        std::cout << outputString << std::endl;
    };

    file = fopen(filename.c_str(), "rb");
    if (file == nullptr)
    {
        destroyAndExit("can't open file!");
        return resultsCollector;
    }

    do
    {
        if (fseek(file, counter*chunkSize, SEEK_SET) != 0)
        {
            destroyAndExit(std::string("SEAK ERROR on teration No.") + std::to_string(counter));
            return resultsCollector;
        }
        counter ++;

        size_t actuallyRead = fread(buffer, 1, readSize, file);
        if (actuallyRead < readSize)
        {
            readMore = false;
        }

        ScannerResults tempResults = scanMemoryBlock({buffer, static_cast<uint64_t>(actuallyRead)});
        resultsCollector.error = tempResults.error;
        if (resultsCollector.error != ResultError::SUCCESS)
        {
            destroyAndExit("EMPTY");
            return resultsCollector;
        }

        resultsCollector.results.insert(tempResults.results.begin(), tempResults.results.end());
    }
    while (readMore);

    destroyAndExit(generateOutput(resultsCollector));
    return resultsCollector;
}

void Manager::setChunkSize(uint64_t sizeInBytes)
{
    std::cout << "Set chunk size to " << sizeInBytes << " bytes" << std::endl;
    chunkSize = sizeInBytes;
    readSize = chunkSize + m_byteSequences[0].size() - 1;
}

ScannerResults Manager::scanMemoryBlock(MemoryBlock memoryBlock)
{
    std::set<Guid> collectedResults;
    collectedResults.clear();

    std::vector<std::thread> threads;
    threads.reserve(m_scannersPool.size());

    for (auto &val : m_scannersPool)
    {
        auto scannerResult = [this, &collectedResults](ResultError error, std::set<Guid> &&results)
        {
            if (error == ResultError::SUCCESS)
            {
                collectedResults.insert(results.begin(), results.end());
            }
        };

        threads.push_back(val.scanMemoryBlockAsync(memoryBlock, scannerResult));
    }

    // wait for all threads finish
    for (auto &val : threads)
    {
        val.join();
    }

    // return collected results
    return ScannerResults(ResultError::SUCCESS, std::move(collectedResults));
}
