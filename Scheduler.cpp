#include "Scheduler.hpp"

#include <memory>

#include <iostream>

struct Callback_event {
    Symbol func;
    boost::asio::high_resolution_timer timer;
    Scheduler *scheduler;

    Callback_event(Symbol _func, boost::asio::io_context &io, Scheduler *sched);

    void operator()(const boost::system::error_code &e) {
        switch (e.value()) {
        case boost::system::errc::success:
            scheduler->callback_executor(func);
            break;
        default:
            return;
        }
    }
};

Callback_event::Callback_event(Symbol _func, boost::asio::io_context &io, Scheduler *sched)
    : func(_func), timer(io), scheduler(sched) {}

Scheduler::Scheduler(std::function<void(Symbol)> _executor): callback_executor(_executor), io(), clocks() {}

void Scheduler::start() {
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
       guard = boost::asio::make_work_guard(io);
    io.run();
}

void Scheduler::make_clock(const Symbol &s, double tempo) {
    clocks.insert(std::pair(s, Clock(tempo)));
}

void Scheduler::schedule_callback(const Symbol *clock, const Symbol &s, double t) {
    io.post([this, clock, s, t](){
        if (clock == nullptr) {
            std::unique_ptr<Callback_event> ptr = std::make_unique<Callback_event>(s, io, this);
            ptr->timer.expires_after(std::chrono::duration_cast< std::chrono::duration<long int, std::ratio<1, 1000000000>> >(
                        std::chrono::duration<double>(t)));
            ptr->timer.async_wait([ptr = std::move(ptr)](const boost::system::error_code &e) {
                ptr->operator()(e);
            });
        } else {
            auto it = clocks.find(*clock);
            if (it == clocks.end()) {
                std::cerr << "Unknown clock\n";
                return;
            }
        }
    });
}

