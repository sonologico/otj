#ifndef EVAL_HPP_INCLUDED
#define EVAL_HPP_INCLUDED

#include "Result.hpp"
#include "Term.hpp"
#include "Value.hpp"

#include <string>
#include <unordered_map>

using Evalres = Result<Value, std::string>;

Evalres eval(const std::unordered_map<Symbol, Value> &dict, const Term &t) {
    switch (t.tag()) {
    case TERM_LITERAL_DOUBLE:
        return Evalres::ok(Value::fromDouble(t.as_double()));
    case TERM_LITERAL_SYMBOL:
        return Evalres::ok(Value::fromSymbol(t.as_symbol()));
    case TERM_VAR: {
        auto it = dict.find(t.as_var().symbol);
        if (it == dict.cend())
            return Evalres::error("bad var");
        else
            return Evalres::ok(it->second);
    }
    case TERM_ABS: {
        const std::shared_ptr<Abs<Term>> abs = t.as_abs();
        throw -1;
    }
    case TERM_APPL: {
        const std::shared_ptr<Appl<Term>> appl = t.as_appl();
        throw -1;
    }
    case TERM_LET: {
        const std::shared_ptr<Let<Term>> let = t.as_let();
        throw -1;
    }
    }
}

#endif
