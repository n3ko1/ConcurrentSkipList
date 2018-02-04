#include "skiplist.h"
#include <iostream>
#include <limits>
#include <random>

int main()
{
    SkipList l = SkipList();
    l.print(std::cout);
    l.insert(1, "Hello");
    l.insert(2, "Hello");
    l.insert(3, "Hello");
    l.insert(4, "Hello");
    l.insert(5, "Hello");
    l.insert(6, "Hello");
    l.insert(7, "Hello");
    l.insert(8, "Hello");
    l.insert(9, "Hello");
    l.print(std::cout);
    return 0;
}

SkipList::SkipList() : probability(0.5)
{
    head = new SkipNode(std::numeric_limits<int>::min(), "head", max_levels);
    NIL = new SkipNode(std::numeric_limits<int>::max(), "NIL", max_levels);
    head->forward[0] = NIL;
}

int SkipList::SkipNode::node_level() const
{
    int level;
    for (level = 0; level != max_levels; ++level)
    {
        if (forward[level] == nullptr)
            break;
    }
    return level;
}

void SkipList::print(std::ostream &os) const
{
    auto x = head;
    for (int i = head->node_level() - 1; i >= 0; --i)
    {
        while (x->forward[i]->key != std::numeric_limits<int>::max())
        {
            x = x->forward[i]; // traverse to the right
            os << "Key: " << x->key << " Value: " << x->value << " Level: " << x->node_level() << std::endl;
        }
    }
}

std::string *SkipList::search(const int searchKey) const
{
    auto x = head;
    // traverse from top of head. Forward size of head is list level
    for (int i = head->node_level() - 1; i >= 0; --i)
    {
        while (x->forward[i]->key < searchKey)
        {
            x = x->forward[i]; // traverse to the right
        }
    }

    x = x->forward[0];
    return (x->key == searchKey ? &x->value : nullptr);
}

int SkipList::random_level() const
{
    int new_level = 1;
    
    while (((double) rand() / (RAND_MAX)) < probability)
    {
        ++new_level;
    }
    return (new_level > max_levels ? max_levels : new_level);
}

void SkipList::insert(const int key, const std::string &val)
{
    auto x = head;
    auto update = new SkipNode *[max_levels];

    // traverse from top of head. Forward size of head is list level
    for (int i = head->node_level() - 1; i >= 0; --i)
    {
        while (x->forward[i]->key < key)
        {
            x = x->forward[i]; // traverse to the right
        }
        update[i] = x; // store last forward pointer
    }
    if (x->forward[0]->key == key)
    {
        x->forward[0]->value = val; // update value
    }
    else
    {
        // insert new node
        int new_level = random_level();
        int list_level = head->node_level();

        if (new_level > list_level)
        {
            for (auto i = list_level; i < new_level; ++i)
            {
                update[i] = head;
            }
        }
        x = new SkipNode(key, val, max_levels);
        for (auto i = 0; i < new_level; ++i)
        {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
    }

    delete update;
    return;
}