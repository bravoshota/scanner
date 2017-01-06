#pragma once

#include <../common.h>
#include <cstdint>
#include <vector>
#include <thread>

struct ByteSequence
{
    ByteSequence(const Bytes &bytes, const Guid &guid);

    uint64_t size() const;

    /**
     * @brief find Fast scanning method in memory block.
     * Uses comparing bytes as 64 bit values.
     * @return true if sequence is found in memory block.
     */
    bool find(const void *firstByte, uint64_t remainingSize) const;

    std::string bytes;
    Guid guid;
private:
    // properties for optimized comparing:
    const uint64_t *data64bit;
    uint64_t count64bit;
    const char *dataRemainingBytes;
    uint32_t countRemainingBytes;
    bool found;
};

struct MemoryBlock
{
    MemoryBlock(const void *firstByte, uint64_t sizeInBytes)
        : firstByte(reinterpret_cast<const char *>(firstByte))
        , sizeInBytes(sizeInBytes)
    {}
    const char *firstByte;
    uint64_t sizeInBytes;
};

/**
 * @brief cb_results used for return results from single scanner method.
 * ResultError: error occured or success.
 * std::set<Guid>: set of found GUIDs.
 */
typedef std::function<void(ResultError, std::set<Guid> &&)> cb_results;

struct Scanner
{
    /**
     * @brief scanMemoryBlock scans memoryBlock in current thread.
     * @param memoryBlock
     * @param cb return callback with parameters (@see cb_results).
     */
    void scanMemoryBlock(MemoryBlock memoryBlock, cb_results cb);

    /**
     * @brief scanMemoryBlockAsync calls scanMemoryBlock in new thread.
     * @see scanMemoryBlock.
     * @return newly created thread.
     */
    std::thread scanMemoryBlockAsync(MemoryBlock memoryBlock, cb_results cb);

    /**
     * @brief byteSequences stores sequences for current Scanner object.
     * All read sequences during Manager's construction is splat by groups
     * and stored in different scanner objects.
     * Single instance of Scanner stores sequences
     * which itself this instance is responsible to check in memory blocks.
     */
    std::vector<ByteSequence> byteSequences;
};
