#include "Frontend/HeadLess.hpp"

#include <queue>

HeadLess::HeadLess() {

}
HeadLess::~HeadLess() {

}
auto HeadLess::tick(Backend *backend, std::queue<addr_t> &addrs) -> void {
    addr_t addr = addrs.front();
    auto access = backend->process(addr);
    addrs.pop();
    if( addrs.empty() ) {
        running = false;
    }
};

auto HeadLess::halted() -> bool {
    return running;
}

