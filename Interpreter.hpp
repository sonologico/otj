#ifndef STATE_HPP_INCLUDED
#define STATE_HPP_INCLUDED

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
#include "Scheduler.hpp"
#include "Symbol.hpp"
#include "Value.hpp"

class Interpreter {
public:
    Interpreter();

    void process();
    void add_built_in(std::string name, unsigned args, Native_f f);
    void read(const std::string &tok);
    void push(Value v);
    Value pop();

    void start(std::atomic_bool &run);

    Scheduler scheduler;
    Symbol_table symtab;
    std::unordered_map<Symbol, Value, Symbol_hash> dict;
    std::vector<Value> *stack;
    void exec_value(Value &v);

private:
    std::vector<Value> main_stack;
    std::vector<Value> callback_stack;

    boost::lockfree::spsc_queue<Symbol> callback_queue;
    boost::lockfree::spsc_queue<std::string> input_queue;

    std::vector<std::vector<Instr>> assembling;

    void process_read(const std::string &tok);
    void process_reference(bool exec, Symbol s);
    void process(bool exec, Value v);

    void execute_callback(Symbol s);
};


#endif
