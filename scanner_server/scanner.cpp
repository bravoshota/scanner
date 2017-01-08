#include <scanner.h>

ByteSequence::ByteSequence(const Bytes &_bytes, const Guid &_guid)
    : m_bytes(_bytes)
    , m_guid(_guid)
    , found(false)
{
    count64bit = m_bytes.size()/sizeof(uint64_t);
    countRemainingBytes = m_bytes.size()%sizeof(uint64_t);
}

bool ByteSequence::find(const void *memoryStart, uint64_t remainingSize) const
{
    if (size() > remainingSize)
    {
        return false;
    }

    if (count64bit > 0)
    {
        const uint64_t *data64bit = reinterpret_cast<const uint64_t *>(m_bytes.data());
        const uint64_t *memory64bit = reinterpret_cast<const uint64_t *>(memoryStart);
        for (auto i = 0u; i < count64bit; i ++)
        {
            if (data64bit[i] != memory64bit[i])
            {
                return false;
            }
        }
    }

    if (countRemainingBytes > 0)
    {
        const char *dataRemainingBytes = m_bytes.data() + count64bit*sizeof(uint64_t);
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
            for (char *ptrIterator = const_cast<char *>(memoryBlock.firstByte); ptrIterator <= ptrEnd; ptrIterator ++)
            {
                for (auto i = 0u; i < byteSequences.size(); i ++)
                {
                    if (byteSequences[i].find(ptrIterator, remainingSize))
                    {
                        results.insert(byteSequences[i].guid());
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
