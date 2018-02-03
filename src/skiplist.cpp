#include "skiplist.h"
#include <iostream>
#include <limits>
#include <random>

SkipList::SkipList() : max_levels(16), probability(0.5)
{
    std::cout << "in const" << std::endl;
    head = new SkipNode(std::numeric_limits<int>::min(), "head", max_levels);
    NIL = new SkipNode(std::numeric_limits<int>::max(), "NIL", max_levels);
    head->forward.push_back(NIL);
    std::cout << NIL->value;
};

void SkipList::print(std::ostream &os) const
{
    auto x = head;
    for (int i = head->forward.size(); i >= 1; --i)
    {
        while (x->forward[i]->key != std::numeric_limits<int>::max())
        {
            x = x->forward[i]; // traverse to the right
            os << "Key: " << x->key << " Value: " << x->value << std::endl;
        }
    }
}

std::string *SkipList::search(const int searchKey) const
{
    auto x = head;
    // traverse from top of head. Forward size of head is list level
    for (int i = head->forward.size(); i >= 1; --i)
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
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    while (dis(gen) < probability)
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
    for (int i = head->forward.size(); i >= 1; --i)
    {
        while (x->forward[i]->key < key)
        {
            x = x->forward[i]; // traverse to the right
        }
        update[i] = x; // store last forward pointer
    }
    if (x)
    {
        x->value = val; // update value
    }
    else
    {
        // insert new node
        int new_level = random_level();
        int list_level = head->forward.size();

        if (new_level > list_level)
        {
            for (auto i = list_level + 1; i <= new_level; ++i)
            {
                update[i] = head;
            }
        }
        x = new SkipNode(key, val, max_levels);
        for (auto i = 1; i <= new_level; ++i)
        {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
    }

    delete update[max_levels];
    return;
}