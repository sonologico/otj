#ifndef RESULT_HPP_INCLUDED
#define RESULT_HPP_INCLUDED

#include <variant>

template <class T, class E>
class Result {
public:
    static Result ok(T &&x) {
        return Result(var_t(x));
    }

    static Result error(E &&x) {
        return Result(var_t(x));
    }

private:
    using var_t = std::variant<T, E>;

    Result(var_t &&x): val(x) {}

    var_t val;
};

#endif
