#ifndef SYMBOL_HPP_INCLUDED
#define SYMBOL_HPP_INCLUDED

#include <vector>
#include <unordered_map>

class Symbol {
public:
    Symbol();
    Symbol(unsigned long _id);
    std::size_t hash() const;

    bool operator==(const Symbol &other) const;
    bool operator!=(const Symbol &other) const;

    unsigned long id;
};

class Symbol_table {
public:
    Symbol_table();

    Symbol intern(const std::string &s);
    std::string symbol_string(const Symbol &s);

private:
    std::unordered_map<std::string, Symbol> by_name;
    std::vector<std::string> by_id;
};

struct Symbol_hash {
    std::size_t operator()(const Symbol &s) const;
};

#endif
