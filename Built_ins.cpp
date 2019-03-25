#include "Built_ins.hpp"
#include "Scheduler.hpp"
#include "Interpreter.hpp"

#include <iostream>

void alias(Interpreter &s, const char *src, const char *dst) {
    s.dict.emplace(std::pair(s.symtab.intern(dst), s.dict.find(s.symtab.intern(src))->second));
}

void print_value(Interpreter &s, const Value &v);

void print_array(Interpreter &s, const std::vector<Value> &arr) {
    if (arr.empty()) {
        std::cout << "a{}";
    } else {
        std::cout << "a{";
        for (const Value &v : arr) {
            std::cout << ' ';
            print_value(s, v);
        }
        std::cout << " }";
    }
}

void print_object(Interpreter &s, const std::unordered_map<Value, Value> map) {
    if (map.empty()) {
        std::cout << "o{}";
    } else {
        std::cout << "o{";
        for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
            std::cout << ' ';
            print_value(s, it->first);
            std::cout << " : ";
            print_value(s, it->second);
        }
        std::cout << " }";
    }
}

void print_value(Interpreter &s, const Value &v) {
    switch (v.tag()) {
    case VALUE_NIL:
        std::cout << '$';
        return;
    case VALUE_NUMBER:
        std::cout << v.asDouble();
        return;
    case VALUE_SYMBOL:
        std::cout << '$' << s.symtab.symbol_string(v.asSymbol());
        return;
    case VALUE_BUILT_IN:
        std::cout << s.symtab.symbol_string(*v.funcName());
        return;
    case VALUE_DEFINED: {
          const Symbol *sym = v.funcName();
          if (sym == nullptr) {
              std::cout << "<CODE>";
          } else {
              std::cout << s.symtab.symbol_string(*sym);
          }
        } break;
    case VALUE_ARRAY:
        print_array(s, v.array_elems());
        break;
    case VALUE_OBJECT:
        print_object(s, v.obj_fields());
        break;
    }
}

void load_built_ins(Interpreter& s) {
    s.add_built_in("!", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value sym = s.pop();
        if (sym.tag() != VALUE_SYMBOL) {
            std::cout << "value is not a symbol in !\n";
            return;
        }
        s.dict.insert_or_assign(sym.asSymbol(), x);
    });
    s.add_built_in("@", 1, [](Interpreter &s) {
        Value x = s.pop();
        if (x.tag() != VALUE_SYMBOL) {
            std::cout << "value is not a symbol in @\n";
            return;
        }
        Symbol sym = x.asSymbol();
        auto iter = s.dict.find(sym);
        if (iter == s.dict.end()) {
            std::cerr << "Unknown word: " << s.symtab.symbol_string(sym) << std::endl;
            return;
        }
        s.push(iter->second);
    });
    s.add_built_in("a{}", 0, [](Interpreter &s) {
        s.push(Value::array());
    });
    s.add_built_in("o{}", 0, [](Interpreter &s) {
        s.push(Value::object());
    });
    s.add_built_in("size", 1, [](Interpreter &s) {
        Value x = s.pop();
        if (x.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in size\n";
            return;
        }
        s.push(Value::fromDouble(x.array_elems().size()));
    });
    s.add_built_in(",", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value arr = s.pop();
        if (arr.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in push\n";
            return;
        }
        std::vector<Value> new_arr = arr.array_elems();
        new_arr.push_back(x);
        s.push(Value::from_vector(std::move(new_arr)));
    });
    s.add_built_in("pop", 1, [](Interpreter &s) {
        Value x = s.pop();
        if (x.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in pop\n";
            return;
        }
        if (x.array_elems().empty()) {
            std::cerr << "can't pop an empty array\n";
            return;
        }
        std::vector<Value> elems = x.array_elems();
        Value e = elems[elems.size() - 1];
        elems.pop_back();
        s.push(e);
        s.push(Value::from_vector(std::move(elems)));
    });
    s.add_built_in("@i", 2, [](Interpreter &s) {
        Value i = s.pop();
        Value arr = s.pop();
        if (arr.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in @i\n";
            return;
        }
        if (i.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number in @i\n";
            return;
        }
        double index = i.asDouble();
        if (index < 0) {
            std::cerr << "index out of range\n";
            return;
        }
        std::vector<Value>::size_type index_i = index;
        std::vector<Value> &elems = arr.array_elems();
        if (index_i >= elems.size()) {
            std::cerr << "index out of range\n";
            return;
        }
        s.push(elems[index_i]);
    });
    s.add_built_in("!i", 3, [](Interpreter &s) {
        Value v = s.pop();
        Value i = s.pop();
        Value arr = s.pop();
        if (i.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number in !i\n";
            return;
        }
        if (arr.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in !i\n";
            return;
        }
        double index = i.asDouble();
        if (index < 0) {
            std::cerr << "index out of range in !i\n";
            return;
        }
        std::vector<Value>::size_type index_i = index;
        std::vector<Value> elems = arr.array_elems();
        if (index_i >= elems.size()) {
            std::cerr << "index out of range in !i\n";
            return;
        }
        elems[index_i] = v;
        s.push(Value::from_vector(elems));
    });
    s.add_built_in("save", 0, [](Interpreter &s) {
        Value v = Value::array();
        v.array_elems() = *s.stack;
        s.push(v);
    });
    s.add_built_in("restore", 1, [](Interpreter &s) {
        Value v = s.pop();
        if (v.tag() != VALUE_ARRAY) {
            std::cerr << "value is not array in restore\n";
            return;
        }
        *s.stack = v.array_elems();
    });
    s.add_built_in("@f", 2, [](Interpreter &s) {
        Value key = s.pop();
        Value obj = s.pop();
        if (obj.tag() != VALUE_OBJECT) {
            std::cerr << "value is not an object in @f\n";
            return;
        }
        Value::Field_map &fields = obj.obj_fields();
        auto it = fields.find(key);
        if (it == fields.end()) {
            std::cerr << "invalid key in @f\n";
            return;
        }
        s.push(it->second);
    });
    s.add_built_in("!f", 3, [](Interpreter &s) {
        Value val = s.pop();
        Value key = s.pop();
        Value obj = s.pop();
        if (obj.tag() != VALUE_OBJECT) {
            std::cerr << "value is not an object in !f\n";
            return;
        }
        Value::Field_map map = obj.obj_fields();
        map.emplace(std::pair(key, val));
        s.push(Value::from_map(map));
    });
    s.add_built_in("delete", 2, [](Interpreter &s) {
        Value key = s.pop();
        Value obj = s.pop();
        if (obj.tag() != VALUE_OBJECT) {
            std::cerr << "value is not an object in !f\n";
            return;
        }
        Value::Field_map &fields = obj.obj_fields();
        auto it = fields.find(key);
        if (it == fields.end()) {
            std::cerr << "invalid key in @f\n";
            return;
        }
        Value::Field_map new_fields = fields;
        new_fields.erase(new_fields.find(key));
        s.push(it->second);
        s.push(Value::from_map(new_fields));
    });
    s.add_built_in("dup", 1, [](Interpreter &s) {
        s.push(s.stack->operator[](s.stack->size() - 1));
    });
    s.add_built_in("drop", 1, [](Interpreter &s) {
        s.pop();
    });
    s.add_built_in("swap", 2, [](Interpreter &s) {
        Value x = s.stack->operator[](s.stack->size() - 1);
        s.stack->operator[](s.stack->size() - 1) = s.stack->operator[](s.stack->size() - 2);
        s.stack->operator[](s.stack->size() - 2) = std::move(x);
    });
    s.add_built_in("exec", 1, [](Interpreter &s) {
        Value v = s.pop();
        s.exec_value(v);
    });
    s.add_built_in("+", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(Value::fromDouble(x.asDouble() + y.asDouble()));
    });
    s.add_built_in("-", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(Value::fromDouble(x.asDouble() - y.asDouble()));
    });
    s.add_built_in("*", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(Value::fromDouble(x.asDouble() * y.asDouble()));
    });
    s.add_built_in("/", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        } else if (x.asDouble() == 0.0) {
            std::cerr << "division by zero\n";
            return;
        }
        s.push(Value::fromDouble(x.asDouble() / y.asDouble()));
    });
    s.add_built_in("<", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(y.asDouble() < x.asDouble() ? Value::fromDouble(1) : Value::nil());
    });
    s.add_built_in(">", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(y.asDouble() > x.asDouble() ? Value::fromDouble(1) : Value::nil());
    });
    s.add_built_in("<=", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(y.asDouble() <= x.asDouble() ? Value::fromDouble(1) : Value::nil());
    });
    s.add_built_in(">=", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        if (x.tag() != VALUE_NUMBER || y.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number\n";
            return;
        }
        s.push(y.asDouble() >= x.asDouble() ? Value::fromDouble(1) : Value::nil());
    });
    s.add_built_in("=", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        s.push(x == y ? Value::fromDouble(1) : Value::nil());
    });
    s.add_built_in("/=", 2, [](Interpreter &s) {
        Value x = s.pop();
        Value y = s.pop();
        s.push(x != y ? Value::fromDouble(1) : Value::nil());
    });
    s.add_built_in("if", 3, [](Interpreter &s) {
        Value boolean = s.pop();
        Value thenb = s.pop();
        Value elseb = s.pop();
        s.exec_value(boolean.tag() == VALUE_NIL ? elseb : thenb);
    });
    s.add_built_in(".s", 0, [](Interpreter &s) {
        if (s.stack->empty()) {
            std::cout << "[]\n";
        } else {
            std::cout << "[\n";
            for (long i = s.stack->size() - 1; i >= 0; --i) {
                Value &v = s.stack->operator[](i);
                std::cout << "  ";
                print_value(s, v);
                std::cout << std::endl;
            }
            std::cout << "]\n";
        }
    });
    s.add_built_in("times", 0, [](Interpreter &s) {
        Value k = s.pop();
        Value action = s.pop();
        if (k.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number in times\n";
            return;
        }
        double count = k.asDouble();
        for (double i = 0; i < count; ++i) {
            s.push(Value::fromDouble(i));
            s.exec_value(action);
        }
    });
    s.add_built_in("iter", 2, [](Interpreter &s) {
        Value arr = s.pop();
        Value action = s.pop();
        if (arr.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in iter\n";
            return;
        }
        std::vector<Value> &elems = arr.array_elems();
        for (Value &v :  elems) {
            s.push(v);
            s.exec_value(action);
        }
    });
    s.add_built_in("map", 2, [](Interpreter &s) {
        Value arr = s.pop();
        Value action = s.pop();
        if (arr.tag() != VALUE_ARRAY) {
            std::cerr << "value is not an array in map\n";
            return;
        }
        std::vector<Value> &elems = arr.array_elems();
        std::vector<Value> new_array;
        for (Value &v :  elems) {
            s.push(v);
            s.exec_value(action);
            new_array.push_back(s.pop());
        }
        s.push(Value::from_vector(new_array));
    });
    s.add_built_in("schedule", 2, [](Interpreter &s) {
        Value time = s.pop();
        Value action = s.pop();

        if (time.tag() != VALUE_NUMBER) {
            std::cerr << "value is not a number in schedule\n";
            return;
        }
        if (action.tag() != VALUE_SYMBOL) {
            std::cerr << "value is not a symbol in schedule\n";
            return;
        }
        s.scheduler.schedule_callback(nullptr, action.asSymbol(), time.asDouble());
    });
    std::string at("at");
    std::string freq("freq");
    s.add_built_in("beep", 1, [at, freq](Interpreter &s) {
        Value args = s.pop();
        if (args.tag() != VALUE_OBJECT) {
            std::cerr << "value is not an object in beep\n";
            return;
        }
        const Value::Field_map &fields = args.obj_fields();
        const auto at_it = fields.find(Value::fromSymbol(s.symtab.intern(at)));
        const auto freq_it = fields.find(Value::fromSymbol(s.symtab.intern(freq)));
        if (at_it == fields.cend()) {
            std::cerr << "field at is missing in beep\n";
            return;
        }
        if (at_it->second.tag() != VALUE_NUMBER) {
            std::cerr << "field at is not a number in beep\n";
            return;
        }
        if (freq_it == fields.cend()) {
            std::cerr << "field freq is missing in beep\n";
            return;
        }
        if (freq_it->second.tag() != VALUE_NUMBER) {
            std::cerr << "field freq is not a number in beep\n";
            return;
        }
    });
    alias(s, ",", "push");
}
