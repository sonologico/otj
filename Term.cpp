#include "Term.hpp"

Term Term::lit_double(double x) {
    return Term(Term::var_t(x));
}

Term Term::lit_symbol(Symbol x) {
    return Term(Term::var_t(x));
}

Term Term::var(Symbol x) {
    return Term(Term::var_t(Var(x)));
}

Term Term::abs(Symbol s, Term body) {
    return Term(Term::var_t(std::make_shared<Abs<Term>>(Var(s), body)));
}

Term Term::appl(Term func, Term arg) {
    return Term(Term::var_t(std::make_shared<Appl<Term>>(func, arg)));
}

Term Term::let(Symbol s, Term bound_expr, Term body) {
    return Term(Term::var_t(std::make_shared<Let<Term>>(Var(s), bound_expr, body)));
}

Term::Term(Term::var_t &&x): val(x) {}

unsigned Term::tag() const {
    return val.index();
}

double Term::as_double() const {
    return std::get<TERM_LITERAL_DOUBLE>(val);
}

Symbol Term::as_symbol() const {
    return std::get<TERM_LITERAL_SYMBOL>(val);
}

const Var &Term::as_var() const {
    return std::get<TERM_VAR>(val);
}

const Term::var_t &Term::variant() const {
    return val;
}
