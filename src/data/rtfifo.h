#ifndef CERBERUS_DATA_RTFIFO_H
#define CERBERUS_DATA_RTFIFO_H

/*  This is a FIFO ring buffer that implements SPSC data transfer and signaling.
 *  Please make sure there is just one producer and just one consumer, otherwise, this
 *  api is not thread-safe
 */

#include <atomic>
#include <cstdint>

#include "../Cerberus_global.h"
#include "../types.h"

namespace crb
{
    class CERBERUS_EXPORT RTFifo
    {
       public:
        struct Meta
        {
            uint32_t len;
            uint32_t _pad0;
            uint64_t seq;
        };

      private:
        alignas(64) std::atomic<uint32_t> m_head;

        uint8_t* m_blocks;
        Meta* m_meta;

        uint32_t m_cap;
        uint32_t m_mask;
        SIZE m_blockBytes;
        SIZE m_blocksSize;
        SIZE m_metaSize;

        int m_eventFd;
        std::atomic<bool> m_overflow;
        bool m_lockMemory;

        alignas(64) std::atomic<uint32_t> m_tail;

       public:
        RTFifo();
        RTFifo(uint32_t capPow2, SIZE blockBytes, SIZE align = 64, bool lockMemory = true);

        ~RTFifo();

        RTFifo(const RTFifo& other) = delete;
        RTFifo& operator=(const RTFifo& other) = delete;

        void init(uint32_t capPow2, SIZE blockBytes, SIZE align = 64, bool lockMemory = true);
        void shutdown();

        // Producer (RT): returns pointer to a writable block or nullptr on overflow (drop-newest).
        uint8_t* tryBeginWrite(uint32_t* outSlot = nullptr);

        // Producer (RT): publish the written block.
        void endWrite(uint32_t slot, SIZE len, uint64_t seq);

        // Consumer: returns pointer to a readable block or nullptr if empty.
        const uint8_t* tryBeginRead(uint32_t* outSlot = nullptr, SIZE* outLen = nullptr,
                                    uint64_t* outSeq = nullptr) const;

        // Consumer: release the block.
        void endRead();

        // Consumer: wait for data availability and drain eventfd.
        // timeoutMs < 0 blocks indefinitely. Returns OR_TimedOut on timeout.
        OpRes waitData(int timeoutMs = -1) const;

        // eventfd used to signal transitions from empty to non-empty.
        int eventFd() const { return m_eventFd; }

        SIZE blockSize() const { return m_blockBytes; }
        uint32_t capacity() const { return m_cap; }

        bool overflowed() const { return m_overflow.load(std::memory_order_relaxed); }
        void clearOverflow() { m_overflow.store(false, std::memory_order_relaxed); }
    };
}  // namespace crb

#endif  // CERBERUS_DATA_RTFIFO_H
