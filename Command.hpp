#ifndef COMMAND_HPP_INCLUDED
#define COMMAND_HPP_INCLUDED

#include <variant>

#include "Term.hpp"

#define COMMAND_DEF 0
#define COMMAND_DO 1

struct Def {
    Var var;
    Term expr;

    Def(Var v, Term e);
};

struct Do {
    Term expr;

    Do(Term e);
};

class Command {
public:
    static Command def(Var v, Term e);
    static Command do_(Term e);

private:
    using var_t = std::variant<Def, Do>;

    Command(var_t &&x);

    unsigned tag();

    var_t val;
};

#endif
