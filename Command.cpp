#include "Command.hpp"


Def::Def(Var v, Term e): var(v), expr(e) {}

Do::Do(Term e): expr(e) {}

Command Command::def(Var v, Term e) {
    return Command(var_t(Def(v, e)));
}

Command Command::do_(Term e) {
    return Command(var_t(Do(e)));
}

Command::Command(Command::var_t &&v): val(v) {}
