#include "skiplist.h"
#include "mingw.thread.h"
#include <iostream>
#include <list>
#include <assert.h>
#include <algorithm>

constexpr int THREAD_COUNT = 128;
constexpr int ROUNDS = 100;
constexpr int ITERATIONS = 100;

std::atomic<int> id(0);
std::atomic<int> deletion_count(0);

int create_id()
{
    return id++;
}

void runInsert(SkipList<int, std::string> &l)
{
    for (auto j = 0; j != ITERATIONS; ++j)
    {
        l.insert(create_id(), "");
    }
}

void runRemove(SkipList<int, std::string> &l)
{
    for (auto j = 0; j != ITERATIONS * ITERATIONS; ++j)
    {
        if (l.remove(rand()))
        {
            deletion_count++;
        }
    }
}

void run_threaded_test()
{
    for (auto x = 0; x != ROUNDS; ++x)
    {
        SkipList<int, std::string> l = SkipList<int, std::string>();
        std::thread tAdd[THREAD_COUNT];
        std::thread tRemove[THREAD_COUNT];
        deletion_count.store(0);
        id.store(0);

        for (auto i = 0; i != THREAD_COUNT; ++i)
        {

            tAdd[i] = std::thread(runInsert, l);
            tRemove[i] = std::thread(runRemove, l);
        }

        for (auto i = 0; i != THREAD_COUNT; ++i)
        {
            tAdd[i].join();
            tRemove[i].join();
        }

        // at this point, size should be insertions - deletions
        int size = l.size();
        std::cout << "Size: " << size << std::endl;
        std::cout << "Insertions: " << THREAD_COUNT * ITERATIONS << std::endl;
        std::cout << "Deletions: " << deletion_count << std::endl;
        assert(size == ((THREAD_COUNT * ITERATIONS) - deletion_count));
    }
}

void run_simple_test()
{

    SkipList<int, std::string> l = SkipList<int, std::string>();
    l.insert(1, "Hello");
    l.insert(2, "World.");
    l.insert(3, "This");
    l.insert(4, "is");
    l.insert(5, "a");
    l.insert(6, "test");
    assert(l.size() == 6);
    l.remove(5);
    assert(l.size() == 5);
    l.insert(7, "this triggered GC.");
    assert(l.size() == 6);
    assert(*l.find_wait_free(3) == "This");
    assert(l.find_wait_free(1337) == nullptr);
}

int main()
{
    run_simple_test();
    run_threaded_test();
    return 0;
}
