#pragma once
/* ------------------------------------------------------------------- *
 *  FlatVectorMap<K,V,Compare>
 *  ---------------------------
 *  A minimal drop-in replacement for std::map used by the random-events
 *  library.  Internally stores sorted unique std::vector< pair<K,V> >.
 *
 *  Complexity
 *      find / operator[]     : O(log N) binary search
 *      insert                : O(N) worst-case (shift right)
 *      iteration             : O(1) contiguous
 * ------------------------------------------------------------------- */
#include <vector>
#include <algorithm>
#include <utility>

template<class K, class V, class Compare = std::less<K>>
class FlatVectorMap {
    using value_type = std::pair<K const, V>;       // matches std::map
    using storage_type = std::vector< std::pair<K, V> >;

public:
    using iterator               = typename storage_type::iterator;
    using const_iterator         = typename storage_type::const_iterator;
    using size_type              = std::size_t;
    using key_type               = K;
    using mapped_type            = V;
    using key_compare            = Compare;

    /* ---------------------------------------------------------------- */
    FlatVectorMap() = default;

    template<class InputIt>
    FlatVectorMap(InputIt first, InputIt last) { insert(first, last); }

    /* capacity / modifiers */
    [[nodiscard]] size_type size()  const noexcept { return data_.size(); }
    [[nodiscard]] bool      empty() const noexcept { return data_.empty(); }
    void clear() noexcept { data_.clear(); }

    std::pair<iterator,bool> insert(const std::pair<K,V>& kv) {
        auto [pos, absent] = lower(kv.first);     // <iterator, bool notFound>
        if (!absent) {                            // key already present
            return {pos, false};
        }
        pos = data_.insert(pos, kv);              // insert & get iterator to new elem
        return {pos, true};
    }


    template<class InputIt>
    void insert(InputIt first, InputIt last) { for (; first!=last; ++first) insert(*first); }

    iterator       begin()  noexcept { return data_.begin();  }
    const_iterator begin()  const noexcept { return data_.begin(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    iterator       end()    noexcept { return data_.end(); }
    const_iterator end()    const noexcept { return data_.end(); }
    const_iterator cend()   const noexcept { return data_.cend(); }

    const_iterator find(const K& k) const { return lower(k).second ? data_.end() : lower(k).first; }
          iterator find(const K& k)       { return lower(k).second ? data_.end() : lower(k).first; }

    V& operator[](const K& k) {
        auto pr = insert({k, V{}});
        return pr.first->second;
    }

    const V& at(const K& k) const { return const_cast<FlatVectorMap*>(this)->at(k); }
          V& at(const K& k) {
              auto it = find(k);
              if (it == data_.end()) throw std::out_of_range("FlatVectorMap::at");
              return it->second;
          }

    /* expose raw data for fast two-pointer merges */
    storage_type const& raw() const noexcept { return data_; }
    storage_type      & raw()       noexcept { return data_; }   // <<< ADD THIS

private:
    /* lower_bound returning {it, not_found_bool} */
    std::pair<iterator,bool> lower(const K& k) {
        auto it = std::lower_bound(data_.begin(), data_.end(), k,
                                   [this](auto const& kv, auto const& key){
                                      return comp_(kv.first, key); });
        bool notfound = (it == data_.end() || comp_(k, it->first) || comp_(it->first, k));
        return {it, notfound};
    }
    std::pair<const_iterator,bool> lower(const K& k) const {
        auto it = std::lower_bound(data_.begin(), data_.end(), k,
                                   [this](auto const& kv, auto const& key){
                                      return comp_(kv.first, key); });
        bool notfound = (it == data_.end() || comp_(k, it->first) || comp_(it->first, k));
        return {it, notfound};
    }

    storage_type data_;
    Compare      comp_;
};
