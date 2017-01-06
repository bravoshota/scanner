#pragma once

#include <scanner.h>
#include <string>
#include <set>

class Manager
{
public:
    Manager(const std::string &sequencesFile);

    /**
     * @brief scanBytes scans bytes into memory block.
     */
    ScannerResults scanBytes(const void *firstByte, uint64_t sizeInBytes);

    /**
     * @brief scanFile scans file.
     */
    ScannerResults scanFile(const std::string &filename);

    /**
     * @brief setChunkSize (@see chunkSize).
     */
    void setChunkSize(uint64_t sizeInBytes);

protected:
    /**
     * @brief updateSequencesFromFile reads sequences data from file.
     * @param filename is received by scanner server proccess
     * through cmd parameter (e.g. in bash: "scanner somesequencesfile.txt")
     */
    void updateSequencesFromFile(const std::string &filename);

    /**
     * @brief scanMemoryBlock base function for both scanBytes and scanFile.
     * This method invokes threads using scanner pool (@see m_scannersPool).
     * @param memoryBlock The memory block object to scan.
     * @return Total results from all scanners in pool.
     */
    ScannerResults scanMemoryBlock(MemoryBlock memoryBlock);

private:
    std::vector<ByteSequence> m_byteSequences;

    /**
     * @brief m_scannersPool stores Scanner objects.
     * Size of this pool is set in constructor equal to CPU cores count.
     */
    std::vector<Scanner> m_scannersPool;

    /**
     * @brief chunkSize in bytes.
     * Used by scanFile method to read from file by chunks
     * less or equal to this value.
     */
    uint64_t chunkSize;

    /**
     * @brief readSize in bytes.
     * For algorithmic use: store actual read bytes size.
     */
    size_t readSize;
};
