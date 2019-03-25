#include "Symbol.hpp"

Symbol::Symbol(): id() {}

Symbol::Symbol(unsigned long _id): id(_id) {}

std::size_t Symbol::hash() const {
    return static_cast<std::size_t>(id);
}

bool Symbol::operator==(const Symbol &s) const {
    return id == s.id;
}

bool Symbol::operator!=(const Symbol &s) const {
    return id != s.id;
}

Symbol_table::Symbol_table(): by_name(), by_id() {}

Symbol Symbol_table::intern(const std::string &s) {
    auto iter = by_name.find(s);
    if (iter != by_name.end()) {
        return iter->second;
    } else {
        unsigned long id = by_id.size();
        by_id.push_back(s);
        Symbol sym(id);
        by_name.emplace(std::make_pair(s, sym));
        return sym;
    }
}

std::string Symbol_table::symbol_string(const Symbol &s) {
    return by_id.at(s.id);
}

std::size_t Symbol_hash::operator()(const Symbol &s) const {
    return s.hash();
}
