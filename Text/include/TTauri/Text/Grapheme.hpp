// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/hash.hpp"
#include <array>

namespace TTauri::Text {

// "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
// https://unicode.org/reports/tr44/ (TR44 5.7.3)
using long_Grapheme = std::array<char32_t,18>;

/*! A Grapheme, what a user thinks a character is.
 * This will exclude ligatures, because a user would see those as separate characters.
 */
class Grapheme {
    /*! This value contains up to 3 code-points, or a pointer+length to an array
     * of code-points located on the heap.
     *
     * The code-points inside the Grapheme are in NFC.
     *
     * If bit 0 is '1' the value contains up to 3 code-points as follows:
     *    - 63:43   3rd code-point, or zero
     *    - 42:22   2nd code-point, or zero
     *    - 21:1    1st code-point, or zero
     *    - 0       '1'
     *
     * if bit 0 is '0' the value contains a length+pointer as follows:
     *    - 63:48   Length
     *    - 47:0    Pointer to long_Grapheme on the heap;
     *              bottom two bits are zero, due to alignment.
     */
    uint64_t value;

public:
    force_inline Grapheme() noexcept : value(1) {}

    force_inline ~Grapheme() {
        delete_pointer();
    }

    force_inline Grapheme(const Grapheme& other) noexcept {
        value = other.value;
        if (other.has_pointer()) {
            value = create_pointer(other.get_pointer()->data(), other.size());
        }
    }

    force_inline Grapheme& operator=(const Grapheme& other) noexcept {
        delete_pointer();
        value = other.value;
        if (other.has_pointer()) {
            value = create_pointer(other.get_pointer()->data(), other.size());
        }
        return *this;
    }

    force_inline Grapheme(Grapheme&& other) noexcept {
        value = other.value;
        other.value = 1;
    }

    force_inline Grapheme& operator=(Grapheme&& other) noexcept {
        delete_pointer();
        value = other.value;
        other.value = 1;
        return *this;
    }

    explicit Grapheme(std::u32string_view codePoints) noexcept;

    force_inline explicit Grapheme(char32_t codePoint) noexcept :
        Grapheme(std::u32string_view{&codePoint, 1}) {}

    explicit operator std::u32string () const noexcept {
        if (has_pointer()) {
            return {get_pointer()->data(), size()};
        } else {
            auto r = std::u32string{};
            auto tmp = value >> 1;
            for (size_t i = 0; i < 3; i++, tmp >>= 21) {
                if (auto codePoint = static_cast<char32_t>(tmp & 0x1f'ffff)) {
                    r += codePoint;
                } else {
                    return r;
                }
            }
            return r;
        }
    }

    operator bool () const noexcept {
        return value != 1;
    }

    [[nodiscard]] size_t hash() const noexcept {
        size_t r = 0;
        for (ssize_t i = 0; i != ssize(*this); ++i) {
            r = hash_mix_two(r, std::hash<char32_t>{}((*this)[i]));
        }
        return r;
    }

    [[nodiscard]] force_inline size_t size() const noexcept {
        if (has_pointer()) {
            return value >> 48;
        } else {
            auto tmp = value >> 1;
            size_t i;
            for (i = 0; i < 3; i++, tmp >>= 21) {
                if ((tmp & 0x1f'ffff) == 0) {
                    return i;
                }
            }
            return i;
        }
    }

    [[nodiscard]] char32_t front() const noexcept {
        if (size() == 0) {
            return 0;
        } else {
            return (*this)[0];
        }
    }

    [[nodiscard]] char32_t operator[](size_t i) const noexcept {
        if (has_pointer()) {
            ttauri_assume(i < std::tuple_size_v<long_Grapheme>); 
            return (*get_pointer())[i];

        } else {
            ttauri_assume(i < 3);
            return (value >> ((i * 21) + 1)) & 0x1f'ffff;
        }
    }

    [[nodiscard]] std::u32string NFC() const noexcept {
        std::u32string r;
        r.reserve(ssize(*this));
        for (ssize_t i = 0; i != ssize(*this); ++i) {
            r += (*this)[i];
        }
        return r;
    }

    [[nodiscard]] std::u32string NFD() const noexcept;

    [[nodiscard]] std::u32string NFKC() const noexcept;

    [[nodiscard]] std::u32string NFKD() const noexcept;

private:
    [[nodiscard]] force_inline bool has_pointer() const noexcept {
        return (value & 1) == 0;
    }

    [[nodiscard]] static uint64_t create_pointer(char32_t const *data, size_t size) noexcept {
        ttauri_assert(size <= std::tuple_size<long_Grapheme>::value);

        auto ptr = new long_Grapheme();
        memcpy(ptr->data(), data, size);

        auto iptr = reinterpret_cast<ptrdiff_t>(ptr);
        auto uptr = static_cast<uint64_t>(iptr << 16) >> 16;
        return (size << 48) | uptr;
    }

    [[nodiscard]] force_inline long_Grapheme *get_pointer() const noexcept {
        auto uptr = (value << 16);
        auto iptr = static_cast<ptrdiff_t>(uptr) >> 16;
        return std::launder(reinterpret_cast<long_Grapheme *>(iptr));
    }

    force_inline void delete_pointer() noexcept {
        if (has_pointer()) {
            delete get_pointer();
        }
    }

    [[nodiscard]] friend bool operator<(Grapheme const& a, Grapheme const& b) noexcept {
        let length = std::min(ssize(a), ssize(b));

        for (ssize_t i = 0; i != length; ++i) {
            if (a[i] < b[i]) {
                return true;
            }
        }
        return ssize(a) < ssize(b);
    }

    [[nodiscard]] friend bool operator==(Grapheme const& a, Grapheme const& b) noexcept {
        if (a.value == b.value) {
            return true;
        }

        if (ssize(a) != ssize(b)) {
            return false;
        }

        for (ssize_t i = 0; i != ssize(a); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }
};

}

namespace std {

template<>
struct hash<TTauri::Text::Grapheme> {
    [[nodiscard]] size_t operator() (TTauri::Text::Grapheme const &rhs) const noexcept {
        return rhs.hash();
    }
};

}