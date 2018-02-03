#include "skiplist.h"
#include <iostream>

int main()
{
    auto l = SkipList();
    l.print(std::cout);
    return 0;
}