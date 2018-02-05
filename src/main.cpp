#include "skiplist.h"
#include <iostream>

int main()
{
    SkipList l = SkipList();
    l.insert(1, "Hello");
    l.insert(2, "world.");
    l.insert(4, "Whatever.");
    l.print(std::cout);
    l.remove(2);
    l.print(std::cout);
    return 0;
}
