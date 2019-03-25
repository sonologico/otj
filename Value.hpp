#ifndef VALUE_HPP_INCLUDED
#define VALUE_HPP_INCLUDED

#include <functional>
#include <string>
#include <variant>
#include <vector>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include "Symbol.hpp"

#define VALUE_NIL 0
#define VALUE_NUMBER 1
#define VALUE_SYMBOL 2
#define VALUE_BUILT_IN 3
#define VALUE_DEFINED 4
#define VALUE_OBJECT 5
#define VALUE_ARRAY 6

class Interpreter;

struct Built_in;
struct Block_t;
struct Object;
struct Array;

using Native_f = std::function<void(Interpreter&)>;

template <class T>
struct Instruction {
    bool exec;
    T value;

    Instruction(bool _exec, T _value): exec(_exec), value(_value) {}
};

class Value {
public:
    using Field_map = std::unordered_map<Value, Value>;

    static Value nil();
    static Value fromDouble(double d);
    static Value fromSymbol(Symbol s);
    static Value func(Symbol *name, std::vector<Instruction<Value>> code);
    static Value built_in(Symbol name, unsigned args, Native_f f);
    static Value object();
    static Value array();
    static Value from_vector(std::vector<Value> vec);
    static Value from_map(Field_map map);

    std::size_t tag() const;

    double asDouble() const;
    Symbol asSymbol() const;
    const Symbol *funcName() const;
    std::vector<Instruction<Value>>& definedFunc();
    unsigned nativeFuncArgs() const;
    Native_f nativeFunc() const;
    std::vector<Value> &array_elems();
    const std::vector<Value> &array_elems() const;
    Field_map &obj_fields();
    const Field_map &obj_fields() const;

    std::size_t hash() const;
    bool operator==(const Value &other) const;
    bool operator!=(const Value &other) const;

private:
    Value();
    Value(double d);
    Value(Symbol s);
    Value(Built_in *p);
    Value(Block_t *p);
    Value(Object *p);
    Value(Array *p);
    std::variant<
        std::monostate,
        double, Symbol, Built_in*,
        boost::intrusive_ptr<Block_t>,
        boost::intrusive_ptr<Object>,
        boost::intrusive_ptr<Array>> var;
};

using Instr = Instruction<Value>;

void intrusive_ptr_add_ref(Block_t *p);
void intrusive_ptr_release(Block_t *p);
void intrusive_ptr_add_ref(Array *p);
void intrusive_ptr_release(Array *p);
void intrusive_ptr_add_ref(Object *p);
void intrusive_ptr_release(Object *p);

namespace std {
    template<>
    struct hash<Value> {
        using argument_type = Value;
        using result_type = std::size_t;
        result_type operator()(argument_type const& s) const noexcept {
            return s.hash();
        }
    };
}

#endif
