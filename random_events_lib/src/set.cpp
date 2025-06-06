#include "set.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

/* ------------------------------------------------------------------ *
 *                       SetElement implementation                    *
 * ------------------------------------------------------------------ */
SetElement::SetElement(const AllSetElementsPtr_t& universe)
    : BitsetSimpleSet(BitsetSimpleSet::Bitset(universe->size())),
      all_elements(universe)
{ /* empty singleton (⊘) */ }

SetElement::SetElement(int idx, const AllSetElementsPtr_t& universe)
    : BitsetSimpleSet(BitsetSimpleSet::Bitset(universe->size())),
      element_index(idx),
      all_elements(universe)
{
    if (idx < 0 || idx >= static_cast<int>(universe->size()))
        throw std::invalid_argument("element_index out of range");
    bits().set(static_cast<std::size_t>(idx));
}

AbstractSimpleSetPtr_t
SetElement::intersection_with(const AbstractSimpleSetPtr_t& other)
{
    const auto& rhs = static_cast<const SetElement&>(*other);

    /* empty   OR different singleton value  →  empty SetElement            */
    if (bits().none() || rhs.bits().none() || element_index != rhs.element_index) {
        return make_shared_set_element(all_elements);      // element_index = -1 (empty)
    }
    /* same index ⇒ identical singleton */
    return make_shared_set_element(element_index, all_elements);
}

/* ------------------------------------------------------------------ *
 *                               Set                                  *
 * ------------------------------------------------------------------ */
Set::Set(const AllSetElementsPtr_t& universe)
    : all_elements(universe)
{
    simple_sets = make_shared_simple_set_set();
}

Set::Set(const SetElementPtr_t& element, const AllSetElementsPtr_t& universe)
    : all_elements(universe)
{
    simple_sets = make_shared_simple_set_set();
    simple_sets->insert(std::static_pointer_cast<AbstractSimpleSet>(element));
}

Set::Set(const SimpleSetSetPtr_t& elements, const AllSetElementsPtr_t& universe)
    : all_elements(universe)
{
    simple_sets = make_shared_simple_set_set(elements->begin(), elements->end());
}

Set::~Set() { simple_sets->clear(); }

/* keep legacy behaviour: no merging, just clone */
AbstractCompositeSetPtr_t Set::simplify() {
    return std::make_shared<Set>(simple_sets, all_elements);
}

AbstractCompositeSetPtr_t Set::make_new_empty() const {
    return std::make_shared<Set>(all_elements);
}

std::string *Set::to_string() {
    if (is_empty()) return &EMPTY_SET_SYMBOL;

    std::ostringstream oss;
    oss << '{';
    bool first = true;
    for (auto const& s : *simple_sets) {
        if (!first) oss << ", ";
        first = false;
        oss << *s->to_string();
    }
    oss << '}';
    return new std::string(std::move(oss).str());
}
