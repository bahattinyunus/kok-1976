#ifndef KOK_MEMORY_POOL_HPP
#define KOK_MEMORY_POOL_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// SABİT BLOKLU BELLEK HAVUZU (FIXED-BLOCK MEMORY POOL)
// Sıfır Parçalanma (Zero Fragmentation), O(1) Tahsis ve Serbest Bırakma
// ============================================================================

template <usize BlockSize, usize BlockCount>
class memory_pool {
public:
    static_assert(BlockSize >= sizeof(usize), "Blok boyutu en az pointer/indeks kadar olmalidir");
    static_assert(BlockCount > 0, "Blok sayisi sifirdan buyuk olmalidir");

    memory_pool() {
        reset();
    }

    void reset() {
        m_free_head = 0;
        m_allocated_count = 0;
        for (usize i = 0; i < BlockCount; ++i) {
            usize* next_ptr = reinterpret_cast<usize*>(&m_storage[i * BlockSize]);
            *next_ptr = (i + 1 < BlockCount) ? (i + 1) : INVALID_INDEX;
        }
    }

    void* allocate() {
        if (m_free_head == INVALID_INDEX) {
            return nullptr; // Havuz tamamen dolu
        }

        usize allocated_idx = m_free_head;
        usize* next_ptr = reinterpret_cast<usize*>(&m_storage[allocated_idx * BlockSize]);
        m_free_head = *next_ptr;
        ++m_allocated_count;

        return static_cast<void*>(&m_storage[allocated_idx * BlockSize]);
    }

    bool deallocate(void* ptr) {
        if (!ptr) return false;

        u8* byte_ptr = static_cast<u8*>(ptr);
        if (byte_ptr < m_storage || byte_ptr >= (m_storage + BlockCount * BlockSize)) {
            return false; // Bu havuza ait degil
        }

        usize offset = static_cast<usize>(byte_ptr - m_storage);
        if (offset % BlockSize != 0) {
            return false; // Yanlis hizalama
        }

        usize block_idx = offset / BlockSize;
        usize* next_ptr = reinterpret_cast<usize*>(ptr);
        *next_ptr = m_free_head;
        m_free_head = block_idx;
        --m_allocated_count;

        return true;
    }

    usize allocated_count() const { return m_allocated_count; }
    usize free_count() const { return BlockCount - m_allocated_count; }
    constexpr usize total_blocks() const { return BlockCount; }
    constexpr usize block_size() const { return BlockSize; }

private:
    static constexpr usize INVALID_INDEX = static_cast<usize>(-1);

    alignas(void*) u8 m_storage[BlockCount * BlockSize];
    usize m_free_head;
    usize m_allocated_count;
};

} // namespace kok

#endif // KOK_MEMORY_POOL_HPP
