#include "Frontend/HeadLess.hpp"

#include <queue>

HeadLess::HeadLess() {

}
HeadLess::~HeadLess() {

}
auto HeadLess::tick(Backend *backend, addr_t addr) -> void {
    backend->process(addr);
};

auto HeadLess::halted() -> bool {
    return running;
}

