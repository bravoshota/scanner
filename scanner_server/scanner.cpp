#include <scanner.h>

ByteSequence::ByteSequence(const Bytes &_bytes, const Guid &_guid)
    : bytes(_bytes)
    , guid(_guid)
    , found(false)
{
    data64bit = reinterpret_cast<const uint64_t *>(bytes.data());
    count64bit = bytes.size()/sizeof(uint64_t);
    dataRemainingBytes = reinterpret_cast<const char *>(bytes.data()) + count64bit*sizeof(uint64_t);
    countRemainingBytes = bytes.size()%sizeof(uint64_t);
}

uint64_t ByteSequence::size() const
{
    return count64bit*sizeof(uint64_t) + countRemainingBytes;
}

bool ByteSequence::find(const void *memoryStart, uint64_t remainingSize) const
{
    if (size() > remainingSize)
    {
        return false;
    }

    for (auto i = 0u; i < count64bit; i ++)
    {
        if (data64bit[i] != reinterpret_cast<const uint64_t *>(memoryStart)[i])
        {
            return false;
        }
    }

    if (countRemainingBytes > 0)
    {
        const char *remainingOffset =
                reinterpret_cast<const char *>(memoryStart) + count64bit*sizeof(uint64_t);
        for (auto i = 0u; i < countRemainingBytes; i ++)
        {
            if (dataRemainingBytes[i] != remainingOffset[i])
            {
                return false;
            }
        }
    }

    return true;
}

void Scanner::scanMemoryBlock(MemoryBlock memoryBlock, cb_results cb)
{
    std::set<Guid> results;
    if (byteSequences.size() > 0)
    {
        // last element stores minimal element because in Manager they are sorted by descending
        auto minimalArraySize = byteSequences.back().size();
        if (minimalArraySize <= memoryBlock.sizeInBytes)
        {
            auto remainingSize = memoryBlock.sizeInBytes;

            const char *ptrEnd = memoryBlock.firstByte + memoryBlock.sizeInBytes - minimalArraySize;
            for (char *ptrIterator = const_cast<char *>(memoryBlock.firstByte); ptrIterator < ptrEnd; ptrIterator ++)
            {
                for (auto i = 0u; i < byteSequences.size(); i ++)
                {
                    if (byteSequences[i].find(ptrIterator, remainingSize))
                    {
                        results.insert(byteSequences[i].guid);
                    }
                }

                remainingSize --;
            }
        }
    }

    cb(ResultError::SUCCESS, std::move(results));
}

std::thread Scanner::scanMemoryBlockAsync(MemoryBlock memoryBlock, cb_results cb)
{
    return std::thread([this, memoryBlock, cb]()
    {
        scanMemoryBlock(memoryBlock, cb);
    });
}
