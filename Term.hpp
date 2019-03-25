#ifndef TERM_HPP_INCLUDED
#define TERM_HPP_INCLUDED

#include <memory>
#include <variant>

#include "Symbol.hpp"

#define TERM_LITERAL_DOUBLE 0
#define TERM_LITERAL_SYMBOL 1
#define TERM_VAR 2
#define TERM_ABS 3
#define TERM_APPL 4
#define TERM_LET 5

struct Var {
    Symbol symbol;

    Var(Symbol s): symbol(s) {}
};

template<class T>
struct Abs {
    Var var;
    T body;

    Abs(Var v, T b): var(v), body(b) {}
};

template<class T>
struct Appl {
    T func;
    T arg;

    Appl(T f, T a): func(f), arg(a) {}
};

template<class T>
struct Let {
    Var var;
    T bound_expr;
    T body;

    Let(Var v, T e, T b): var(v), bound_expr(e), body(b) {}
};

class Term {
public:
    using var_t = std::variant<
        double,
        Symbol,
        Var,
        std::shared_ptr<Abs<Term>>,
        std::shared_ptr<Appl<Term>>,
        std::shared_ptr<Let<Term>>
    >;

    static Term lit_double(double x);
    static Term lit_symbol(Symbol x);
    static Term var(Symbol x);
    static Term abs(Symbol s, Term body);
    static Term appl(Term func, Term arg);
    static Term let(Symbol s, Term bound_expr, Term body);

    unsigned tag() const;

    double as_double() const;
    Symbol as_symbol() const;
    const Var& as_var() const;

    const std::shared_ptr<Abs<Term>> as_abs() const;
    const std::shared_ptr<Appl<Term>> as_appl() const;
    const std::shared_ptr<Let<Term>> as_let() const;

    const var_t &variant() const;

private:
    Term(var_t &&x);

    var_t val;
};

#endif
