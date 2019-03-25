#include "Value.hpp"
#include "Interpreter.hpp"

struct Built_in {
    Symbol name;
    unsigned args;
    Native_f func;

    Built_in(Symbol _name, unsigned _args, Native_f _f)
        : name(_name), args(_args), func(_f) {}
};

struct Block_t {
    unsigned long refcount;
    Symbol *name;
    std::vector<Instr> code;

    Block_t(Symbol *_name, std::vector<Instr> _code)
        : refcount(0), name(_name), code(_code) {}
};

struct Object {
    unsigned long refcount;
    Value::Field_map fields;

    Object():refcount(0), fields() {}
    Object(Value::Field_map map): refcount(0), fields(std::move(map)) {}
};

struct Array {
    std::vector<Value> elems;
    unsigned long refcount;

    Array():elems(), refcount(0) {}
    Array(std::vector<Value> _elems): elems(std::move(_elems)), refcount(0) {}
};

std::size_t Value::tag() const {
    return var.index();
}

Value Value::nil() {
    return Value();
}

Value Value::fromDouble(double d) {
    return Value(d);
}

Value Value::fromSymbol(Symbol s) {
    return Value(s);
}

Value Value::object() {
    return Value(new Object);
}

Value Value::from_map(Value::Field_map map) {
    return Value(new Object(std::move(map)));
}

Value Value::array() {
    return Value(new Array);
}

Value Value::from_vector(std::vector<Value> vec) {
    return Value(new Array(std::move(vec)));
}

Value Value::func(Symbol *s, std::vector<Instr> code) {
    return Value(new Block_t(s, code));
}

Value Value::built_in(Symbol s, unsigned args, Native_f f) {
    return Value(new Built_in(s, args, f));
}

const Symbol* Value::funcName() const {
    switch (tag()) {
    case VALUE_DEFINED:
        return std::get<VALUE_DEFINED>(var)->name;
    case VALUE_BUILT_IN:
        return &std::get<VALUE_BUILT_IN>(var)->name;
    default:
        throw "funcName illegal argument";
    }
}

double Value::asDouble() const {
    return std::get<VALUE_NUMBER>(var);
}

Symbol Value::asSymbol() const {
    return std::get<VALUE_SYMBOL>(var);
}

std::vector<Instr>& Value::definedFunc() {
    return std::get<VALUE_DEFINED>(var)->code;
}

unsigned Value::nativeFuncArgs() const {
    return std::get<VALUE_BUILT_IN>(var)->args;
}

Native_f Value::nativeFunc() const {
    return std::get<VALUE_BUILT_IN>(var)->func;
}

std::vector<Value> &Value::array_elems() {
    return std::get<VALUE_ARRAY>(var)->elems;
}

const std::vector<Value> &Value::array_elems() const {
    return std::get<VALUE_ARRAY>(var)->elems;
}

Value::Field_map &Value::obj_fields() {
    return std::get<VALUE_OBJECT>(var)->fields;
}

const Value::Field_map &Value::obj_fields() const {
    return std::get<VALUE_OBJECT>(var)->fields;
}

Value::Value(): var() {}

Value::Value(double d): var(d) {}

Value::Value(Symbol s): var(s) {}

Value::Value(Built_in *p): var(p) {}

Value::Value(Block_t *p): var(std::in_place_index<VALUE_DEFINED>, boost::intrusive_ptr(p, true)) {}

Value::Value(Object *p): var(std::in_place_index<VALUE_OBJECT>, boost::intrusive_ptr(p, true)) {}

Value::Value(Array *p): var(std::in_place_index<VALUE_ARRAY>, boost::intrusive_ptr(p, true)) {}


std::size_t Value::hash() const {
    switch (tag()) {
    case VALUE_NIL:
        return 0;
    case VALUE_NUMBER:
        return static_cast<std::size_t>(asDouble());
    case VALUE_SYMBOL:
        return asSymbol().hash();
    case VALUE_BUILT_IN:
        return reinterpret_cast<std::size_t>(std::get<VALUE_BUILT_IN>(var));
    case VALUE_DEFINED:
        return reinterpret_cast<std::size_t>(std::get<VALUE_DEFINED>(var).get());
    case VALUE_ARRAY:
        return reinterpret_cast<std::size_t>(std::get<VALUE_ARRAY>(var).get());
    case VALUE_OBJECT:
        return reinterpret_cast<std::size_t>(std::get<VALUE_OBJECT>(var).get());
    default:
        throw -1;
    }
}

bool Value::operator==(const Value &other) const {
    if (tag() != other.tag())
        return false;
    switch (tag()) {
    case VALUE_NIL:
        return true;
    case VALUE_NUMBER:
        return asDouble() == other.asDouble();
    case VALUE_SYMBOL:
        return asSymbol() == other.asSymbol();
    case VALUE_BUILT_IN:
        return std::get<VALUE_BUILT_IN>(var) == std::get<VALUE_BUILT_IN>(other.var);
    case VALUE_DEFINED:
        return std::get<VALUE_DEFINED>(var) == std::get<VALUE_DEFINED>(other.var);
    case VALUE_ARRAY:
        return std::get<VALUE_ARRAY>(var) == std::get<VALUE_ARRAY>(other.var);
    case VALUE_OBJECT:
        return std::get<VALUE_OBJECT>(var) == std::get<VALUE_OBJECT>(other.var);
    default:
        throw -1;
    }
}

bool Value::operator!=(const Value &other) const {
    return !(*this == other);
}

void intrusive_ptr_add_ref(Block_t *p) {
    ++p->refcount;
}

void intrusive_ptr_release(Block_t *p) {
    if (p->refcount == 1)
        delete p;
    else
        --p->refcount;
}

void intrusive_ptr_add_ref(Object *p) {
    ++p->refcount;
}

void intrusive_ptr_release(Object *p) {
    if (p->refcount == 1)
        delete p;
    else
        --p->refcount;
}

void intrusive_ptr_add_ref(Array *p) {
    ++p->refcount;
}

void intrusive_ptr_release(Array *p) {
    if (p->refcount == 1)
        delete p;
    else
        --p->refcount;
}

