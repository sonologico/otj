#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <unordered_map>
#include "Symbol.hpp"
#include "Clock.hpp"

class Scheduler {
public:
    Scheduler(std::function<void(Symbol)> _executor);

    void start();

    void make_clock(const Symbol &s, double tempo);
    void schedule_callback(const Symbol *clock, const Symbol &s, double t);

    friend class Callback_event;

private:
    std::function<void(Symbol)> callback_executor;
    boost::asio::io_context io;
    std::unordered_map<Symbol, Clock, Symbol_hash> clocks;
};
