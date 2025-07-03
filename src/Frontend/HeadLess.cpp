#include "Frontend/HeadLess.hpp"

#include <queue>

HeadLess::HeadLess() {

}
HeadLess::~HeadLess() {

}
auto HeadLess::tick(Backend *backend, std::queue<addr_t> &addrs) -> void {
    backend->process(addrs.front());
};

auto HeadLess::halted() -> bool {
    return running;
}

