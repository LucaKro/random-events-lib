#pragma once
/* -------------------------------------------------------------------------
 *  BitsetSimpleSet
 *  ----------------
 *  A simple set whose universe size U is known at construction time.
 *  Internally stored as boost::dynamic_bitset<U>.
 *
 *  Complexity
 *    intersection / difference / complement :  O(U / word_size)
 *    equality / ordering                     :  O(U / word_size)
 *
 *  NOTE:  No code outside this header is modified yet; nothing includes this
 *  file automatically.  You may #include it where you start migrating from
 *  SetElement to bitset-based discrete domains.
 * ------------------------------------------------------------------------ */

#include <boost/dynamic_bitset.hpp>
#include <sstream>
#include <memory>
#include "sigma_algebra.h"

class BitsetSimpleSet : public AbstractSimpleSet {
public:
    using Bitset           = boost::dynamic_bitset<>;
    using BitsetPtr_t      = std::shared_ptr<BitsetSimpleSet>;

    /* ------------------------------------------------------------------ */
    /*                        Construction helpers                        */
    /* ------------------------------------------------------------------ */
    explicit BitsetSimpleSet(Bitset bits) : bits_(std::move(bits)) {}

    static BitsetPtr_t make_shared(Bitset bits) {
        return std::make_shared<BitsetSimpleSet>(std::move(bits));
    }

    /* Build a size-U empty set */
    static BitsetPtr_t make_empty(std::size_t universe_size) {
        return make_shared(Bitset(universe_size));
    }

    /* Build a singleton {idx} within universe size U */
    static BitsetPtr_t make_singleton(std::size_t idx, std::size_t universe_size) {
        Bitset b(universe_size);
        b.set(idx);
        return make_shared(std::move(b));
    }

    /* ------------------------------------------------------------------ */
    /*                    AbstractSimpleSet overrides                     */
    /* ------------------------------------------------------------------ */
    AbstractSimpleSetPtr_t intersection_with(const AbstractSimpleSetPtr_t &other) override {
        const auto &rhs = static_cast<const BitsetSimpleSet&>(*other);
        Bitset tmp = bits_;
        tmp &= rhs.bits_;
        return make_shared(std::move(tmp));
    }

    SimpleSetSetPtr_t complement() override {
        Bitset tmp = bits_;
        tmp.flip();
        auto result = make_shared_simple_set_set();
        result->insert(make_shared(std::move(tmp)));
        return result;
    }

    SimpleSetSetPtr_t difference_with(const AbstractSimpleSetPtr_t &other) {
        const auto &rhs = static_cast<const BitsetSimpleSet&>(*other);
        Bitset tmp = bits_;
        tmp &= ~(rhs.bits_);
        auto result = make_shared_simple_set_set();
        result->insert(make_shared(std::move(tmp)));
        return result;
    }

    bool contains(const ElementaryVariant *) override { return false; /* not used */ }

    bool is_empty() override { return bits_.none(); }

    /* ---------------- Equality / ordering for pointer sets ------------ */
    bool operator==(const AbstractSimpleSet &other) override {
        return bits_ == static_cast<const BitsetSimpleSet&>(other).bits_;
    }

    bool operator<(const AbstractSimpleSet &other) override {
        /* Lexicographic comparison on bits (MSB … LSB) */
        auto const &rhs = static_cast<const BitsetSimpleSet&>(other);
        std::size_t n = std::max(bits_.size(), rhs.bits_.size());
        for (std::size_t i = n; i-- > 0;) {          // iterate descending
            bool a = i < bits_.size() ? bits_.test(i) : false;
            bool b = i < rhs.bits_.size() ? rhs.bits_.test(i) : false;
            if (a != b) return a < b;
        }
        return false;                               // equal
    }

    /* ------------------------------ I/O ------------------------------- */
    std::string *non_empty_to_string() override {
        auto out = new std::string();
        out->push_back('{');
        bool first = true;
        for (std::size_t i = bits_.find_first(); i != Bitset::npos;
            i = bits_.find_next(i)) {
            if (!first) out->append(", ");
            first = false;
            out->append(std::to_string(i));
        }
        out->push_back('}');
        return out;
    }

    /* ------------------------------------------------------------------ */
    /*                         Access to raw bits                         */
    /* ------------------------------------------------------------------ */
    const Bitset &bits() const noexcept { return bits_; }
          Bitset &bits()       noexcept { return bits_; }

private:
    Bitset bits_;
};
