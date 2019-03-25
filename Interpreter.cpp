#include "Interpreter.hpp"
#include <iostream>
#include <string>
#include <thread>

Interpreter::Interpreter()
    : scheduler(std::bind(&Interpreter::execute_callback, this, std::placeholders::_1)),
      symtab(), dict(), stack(), main_stack(), callback_stack(), callback_queue(100), input_queue(200), assembling() {
          stack = &main_stack;
      }

void Interpreter::start(std::atomic_bool &run) {
    std::thread sched_thread = std::thread([this]() {
        scheduler.start();
    });

    while (run.load()) {
        callback_queue.consume_all([this](Symbol &s) {
            auto iter = dict.find(s);
            if (iter == dict.end()) {
                return;
            }
            stack = &callback_stack;
            exec_value(iter->second);
            callback_stack.clear();
            stack = &main_stack;
        });
        input_queue.consume_one([this](std::string &tok){
            process_read(tok);
        });
    }
}

void Interpreter::execute_callback(Symbol s) {
    callback_queue.push(s);
}

void Interpreter::process_reference(bool exec, Symbol s) {
    auto iter = dict.find(s);
    if (iter == dict.end()) {
        std::cerr << "Unknown word: " << symtab.symbol_string(s) << std::endl;
        return;
    }
    process(exec, iter->second);
}

void Interpreter::process(bool exec, Value v) {
    if (assembling.empty()) {
        if (exec)
            exec_value(v);
        else
            push(std::move(v));
    } else {
        assembling[assembling.size() - 1].emplace_back(exec, std::move(v));
    }
}

void Interpreter::read(const std::string &str) {
    input_queue.push(str);
}

void Interpreter::process_read(const std::string &tok) {
    if (tok.size() == 0)
        return;
    try {
        double d = std::stod(tok);
        process(false, Value::fromDouble(d));
        return;
    } catch (const std::invalid_argument &) {
        if (tok.size() == 1) {
            switch (tok[0]) {
            case '[':
                assembling.emplace_back();
                return;
            case ']':
                if (assembling.empty()) {
                    std::cerr << "[ with no matching ]\n";
                    return;
                } else {
                    Value v = Value::func(nullptr,
                                          std::move(assembling[assembling.size() - 1]));
                    assembling.pop_back();
                    process(false, std::move(v));
                    return;
                }
            case '&':
            case '$':
                std::cerr << "Invalid word: " << tok << std::endl;
                return;
            default:
                process_reference(true, symtab.intern(tok));
                return;
            }
        } else {
            switch (tok[0]) {
            case '$':
                process(false, Value::fromSymbol(symtab.intern(tok.substr(1))));
                break;
            case '&':
                process_reference(false, symtab.intern(tok.substr(1)));
                break;
            default:
                process_reference(true, symtab.intern(tok));
                break;
            }
        }
    }
}

void Interpreter::exec_value(Value &v) {
    switch (v.tag()) {
    case VALUE_BUILT_IN:
        if (v.nativeFuncArgs() > stack->size()) {
            std::cerr << "stack too small for "
                << symtab.symbol_string(*(v.funcName()))
                << std::endl;
            return;
        }
        v.nativeFunc()(*this);
        return;
    case VALUE_DEFINED:
        for (Instr &sub: v.definedFunc()) {
            if (sub.exec)
                exec_value(sub.value);
            else
                push(sub.value);
        }
        return;
    default:
        push(v);
        return;
    }
}

void Interpreter::add_built_in(std::string name, unsigned args, Native_f f) {
    Symbol s = symtab.intern(name);
    dict.emplace(std::pair(s, Value::built_in(s, args, f)));
}

void Interpreter::push(Value v) {
    stack->emplace_back(std::move(v));
}

Value Interpreter::pop() {
    Value v = stack->operator[](stack->size() - 1);
    stack->pop_back();
    return v;
}
