#ifndef KOK_STATIC_VECTOR_HPP
#define KOK_STATIC_VECTOR_HPP

#include "types.hpp"
#include <new>
#include <utility>

namespace kok {

// ============================================================================
// SABİT KAPASİTELİ HEAP-SIZ VEKTÖR
// (Zero Dynamic Allocation - Deterministik Taktik Bellek)
// ============================================================================

template <typename T, usize Capacity>
class static_vector {
public:
    static_assert(Capacity > 0, "Kapasite sifirdan buyuk olmalidir");

    constexpr static_vector() : m_size(0) {}

    ~static_vector() {
        clear();
    }

    static_vector(const static_vector& other) : m_size(0) {
        for (usize i = 0; i < other.m_size; ++i) {
            push_back(other[i]);
        }
    }

    static_vector& operator=(const static_vector& other) {
        if (this != &other) {
            clear();
            for (usize i = 0; i < other.m_size; ++i) {
                push_back(other[i]);
            }
        }
        return *this;
    }

    static_vector(static_vector&& other) noexcept : m_size(0) {
        for (usize i = 0; i < other.m_size; ++i) {
            push_back(std::move(other[i]));
        }
        other.clear();
    }

    static_vector& operator=(static_vector&& other) noexcept {
        if (this != &other) {
            clear();
            for (usize i = 0; i < other.m_size; ++i) {
                push_back(std::move(other[i]));
            }
            other.clear();
        }
        return *this;
    }

    bool push_back(const T& val) {
        if (m_size >= Capacity) return false;
        new (&m_data[m_size]) T(val);
        ++m_size;
        return true;
    }

    bool push_back(T&& val) {
        if (m_size >= Capacity) return false;
        new (&m_data[m_size]) T(std::move(val));
        ++m_size;
        return true;
    }

    template <typename... Args>
    bool emplace_back(Args&&... args) {
        if (m_size >= Capacity) return false;
        new (&m_data[m_size]) T(std::forward<Args>(args)...);
        ++m_size;
        return true;
    }

    bool pop_back() {
        if (m_size == 0) return false;
        --m_size;
        reinterpret_cast<T*>(&m_data[m_size])->~T();
        return true;
    }

    void clear() {
        for (usize i = 0; i < m_size; ++i) {
            reinterpret_cast<T*>(&m_data[i])->~T();
        }
        m_size = 0;
    }

    T& operator[](usize idx) {
        return *reinterpret_cast<T*>(&m_data[idx]);
    }

    const T& operator[](usize idx) const {
        return *reinterpret_cast<const T*>(&m_data[idx]);
    }

    T* data() { return reinterpret_cast<T*>(&m_data[0]); }
    const T* data() const { return reinterpret_cast<const T*>(&m_data[0]); }

    usize size() const { return m_size; }
    constexpr usize capacity() const { return Capacity; }
    bool empty() const { return m_size == 0; }
    bool full() const { return m_size >= Capacity; }

    T* begin() { return reinterpret_cast<T*>(&m_data[0]); }
    const T* begin() const { return reinterpret_cast<const T*>(&m_data[0]); }
    T* end() { return reinterpret_cast<T*>(&m_data[m_size]); }
    const T* end() const { return reinterpret_cast<const T*>(&m_data[m_size]); }

private:
    alignas(T) u8 m_data[Capacity * sizeof(T)];
    usize m_size;
};

} // namespace kok

#endif // KOK_STATIC_VECTOR_HPP
