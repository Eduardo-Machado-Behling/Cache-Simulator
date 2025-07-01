#include "Frontend/Frontend.hpp"

#include <queue>

class HeadLess : public Frontend
{
private:
    bool running = true;
public:
    HeadLess();
    ~HeadLess();
    auto tick(Backend *backend, std::queue<addr_t> &addrs) -> void;
    auto halted() -> bool;
};

