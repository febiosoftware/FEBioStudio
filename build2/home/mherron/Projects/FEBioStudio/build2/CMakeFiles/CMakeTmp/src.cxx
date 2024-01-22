#include <atomic>
#include <cstdint>

void test(volatile std::atomic<std::int64_t> &a)
{
    std::int64_t v = a.load(std::memory_order_acquire);
    while (!a.compare_exchange_strong(v, v + 1,
                                      std::memory_order_acq_rel,
                                      std::memory_order_acquire)) {
        v = a.exchange(v - 1);
    }
    a.store(v + 1, std::memory_order_release);
}

int main(int, char **)
{
    void *ptr = (void*)0xffffffc0; // any random pointer
    test(*reinterpret_cast<std::atomic<std::int64_t> *>(ptr));
    return 0;
}
