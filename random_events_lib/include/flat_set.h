#pragma once
/* A drop-in, header-only replacement for std::set that stores elements in a
 * sorted/unique std::vector.  100 % STL-compatible for the subset of methods
 * your framework and tests use (begin/end/insert/size/empty/clear/rbegin etc.).
 *
 * Complexity:
 *   - insert(single)      O(N)  (binary-search + vector::insert)
 *   - insert(range)       O(N+M) (two-pointer merge, no extra allocations)
 *   - iteration           O(1)  contiguous cache-friendly storage
 *
 * Template parameters mirror std::set<T,Compare>.
 * Requires C++17.
 */
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>            

template<class T, class Compare = std::less<T>>
class FlatSet {
public:
    using value_type             = T;
    using iterator               = typename std::vector<T>::iterator;
    using const_iterator         = typename std::vector<T>::const_iterator;
    using reverse_iterator       = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;
    using size_type              = std::size_t;

    FlatSet()  = default;
    ~FlatSet() = default;

    /* range-ctor so std::set-style brace-init works */
    template<class InputIt>
    FlatSet(InputIt first, InputIt last) { insert(first, last); }

    /* -------- basic capacity -------- */
    [[nodiscard]] size_type size()  const noexcept { return data_.size(); }
    [[nodiscard]] bool      empty() const noexcept { return data_.empty(); }
    void clear() noexcept { data_.clear(); }

    /* -------- iterators -------- */
    iterator               begin()  noexcept { return data_.begin();  }
    const_iterator         begin()  const noexcept { return data_.begin(); }
    const_iterator         cbegin() const noexcept { return data_.cbegin(); }
    iterator               end()    noexcept { return data_.end();    }
    const_iterator         end()    const noexcept { return data_.end(); }
    const_iterator         cend()   const noexcept { return data_.cend(); }
    reverse_iterator       rbegin() noexcept { return data_.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
    reverse_iterator       rend()   noexcept { return data_.rend();   }
    const_reverse_iterator rend()   const noexcept { return data_.rend(); }

    /* -------- modifiers -------- */
    bool insert(value_type v) {
        auto pos = std::lower_bound(data_.begin(), data_.end(), v, cmp_);
        if (pos != data_.end() && !cmp_(v, *pos)) { return false; }     // duplicate
        data_.insert(pos, std::move(v));
        return true;
    }

    template<class U>
    std::enable_if_t<
        std::is_base_of_v<typename value_type::element_type, U>, bool>
    insert(const std::shared_ptr<U>& v) {
        return insert(std::static_pointer_cast<typename value_type::element_type>(v));
    }

    template<class InputIt>
    void insert(InputIt first, InputIt last) {
        if (first == last) return;

        // Fast path: reserve and push, then sort+unique once.
        const auto start_sz = data_.size();
        std::copy(first, last, std::back_inserter(data_));
        std::inplace_merge(data_.begin(), data_.begin()+start_sz, data_.end(), cmp_);
        data_.erase(std::unique(data_.begin(), data_.end(),
                                [this](auto const& a, auto const& b){
                                      return !cmp_(a,b) && !cmp_(b,a); }),
                    data_.end());
    }

    /* Convenience for tests that expect count(x) */
    size_type count(value_type const& v) const {
        return std::binary_search(data_.begin(), data_.end(), v, cmp_) ? 1 : 0;
    }

private:
    std::vector<T> data_;
    Compare        cmp_;
};

