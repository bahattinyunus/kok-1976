#ifndef KOK_RING_BUFFER_HPP
#define KOK_RING_BUFFER_HPP

#include "types.hpp"

namespace kok {

// ============================================================================
// DAİRESEL TAMPON (RING BUFFER)
// Kesinti ve Telsiz Örnekleme Hatları için Sıfır-Kopya, Sıfır-Tahsis Tampon
// ============================================================================

template <typename T, usize Capacity>
class ring_buffer {
public:
    static_assert(Capacity > 0, "Tampon boyutu sifirdan buyuk olmalidir");
    // Verim icin 2'nin kuvveti kontrolu
    static constexpr bool is_power_of_two = (Capacity & (Capacity - 1)) == 0;

    constexpr ring_buffer() : m_head(0), m_tail(0), m_count(0) {}

    void reset() {
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    bool push(const T& item) {
        if (full()) return false;
        m_buffer[m_head] = item;
        m_head = next_index(m_head);
        ++m_count;
        return true;
    }

    bool pop(T& out_item) {
        if (empty()) return false;
        out_item = m_buffer[m_tail];
        m_tail = next_index(m_tail);
        --m_count;
        return true;
    }

    bool peek(T& out_item) const {
        if (empty()) return false;
        out_item = m_buffer[m_tail];
        return true;
    }

    usize size() const { return m_count; }
    constexpr usize capacity() const { return Capacity; }
    bool empty() const { return m_count == 0; }
    bool full() const { return m_count >= Capacity; }

private:
    constexpr usize next_index(usize current) const {
        if constexpr (is_power_of_two) {
            return (current + 1) & (Capacity - 1);
        } else {
            return (current + 1) % Capacity;
        }
    }

    T m_buffer[Capacity];
    usize m_head;
    usize m_tail;
    usize m_count;
};

} // namespace kok

#endif // KOK_RING_BUFFER_HPP
