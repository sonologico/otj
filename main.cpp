#include <atomic>
#include <csound/csound.hpp>
#include <iostream>
#include <string>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>

#include "Built_ins.hpp"
#include "Term.hpp"
#include "Scheduler.hpp"
#include "Interpreter.hpp"

const char *orc_text =
 "instr 1 \n"
 " out(linen(oscili(p4,p5),0.1,p3,0.1)) \n"
 "endin \n";

const char *sco_text = "i1 0 5 1000 440 \n";

int main()
{
    Term t = Term::lit_double(0);
    std::atomic_bool run(true);
    Csound csd;
    csd.SetOption("-odac");
    csd.Start();
    csd.CompileOrc(orc_text);
    std::thread csd_thread([&run, &csd]() {
        while (run.load()) {
            int result = csd.PerformKsmps();
            if (result != 0) {
                std::cerr << "csound error\n";
                run.store(false);
                break;
            }
        }
        csd.Stop();
        csd.Cleanup();
    });

    Interpreter st;
    load_built_ins(st);

    std::thread inp_thread = std::thread([&csd, &run, &st](){
        std::string tok;
        while (run.load() && std::cin >> tok) {
            if (tok == "#quit") {
                break;
            }
            st.read(tok);
        }
        run.store(false);
    });

    st.start(run);

    return 0;
}
