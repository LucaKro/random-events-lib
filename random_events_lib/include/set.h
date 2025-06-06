#pragma once

#include "sigma_algebra.h"
#include "bitset_simple_set.h"
#include <utility>
#include <memory>

/* ------------------------------------------------------------------ *
 *        Forward declarations + smart-pointer aliases                *
 * ------------------------------------------------------------------ */
class SetElement;   using SetElementPtr_t = std::shared_ptr<SetElement>;
class Set;          using SetPtr_t        = std::shared_ptr<Set>;
using AllSetElementsPtr_t = std::shared_ptr<std::vector<long long>>;

/* helper: build universe vector from any iterable container */
template<class Container>
inline AllSetElementsPtr_t make_shared_all_elements(const Container& c) {
    return std::make_shared<std::vector<long long>>(c.begin(), c.end());
}

template<typename... Args>
inline SetElementPtr_t make_shared_set_element(Args&&... args) {
    return std::make_shared<SetElement>(std::forward<Args>(args)...);
}

template<typename... Args>
inline SetPtr_t make_shared_set(Args&&... args) {
    return std::make_shared<Set>(std::forward<Args>(args)...);
}

/* ------------------------------------------------------------------ *
 *                           SetElement                               *
 * ------------------------------------------------------------------ */
class SetElement : public BitsetSimpleSet {
public:
    int                 element_index{-1};
    AllSetElementsPtr_t all_elements;

    explicit SetElement(const AllSetElementsPtr_t& universe);
    SetElement(int element_index, const AllSetElementsPtr_t& universe);

    /* Override so tests can still cast to SetElement and check element_index */
    AbstractSimpleSetPtr_t intersection_with(const AbstractSimpleSetPtr_t& other) override;
};

/* ------------------------------------------------------------------ *
 *                                Set                                 *
 * ------------------------------------------------------------------ */
class Set : public AbstractCompositeSet {
public:
    AllSetElementsPtr_t all_elements;

    explicit Set(const AllSetElementsPtr_t& universe);
    Set(const SetElementPtr_t& element, const AllSetElementsPtr_t& universe);
    Set(const SimpleSetSetPtr_t& elements, const AllSetElementsPtr_t& universe);

    ~Set() override;

    AbstractCompositeSetPtr_t simplify()             override;  // keeps cardinality
    AbstractCompositeSetPtr_t make_new_empty() const override;
    std::string               *to_string()           override;
};
