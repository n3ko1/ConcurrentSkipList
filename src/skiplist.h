#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <string>
#include <iostream>
#include <limits>
#include <random>

#define SKIPLIST_TEMPLATE_ARGS \
  template <typename KeyType, typename ValueType>

#define SKIPLIST_TYPE \
  SkipList<KeyType, ValueType>

SKIPLIST_TEMPLATE_ARGS
class SkipList
{
public:
  SkipList();
  ~SkipList(){};

  ValueType *search(const KeyType searchKey) const;
  void insert(const KeyType key, const ValueType &val);
  void remove(const KeyType key);

  void print(std::ostream &os) const;

private:
  struct SkipNode
  {
    SkipNode(const KeyType k, const ValueType &v, const int forward_size) : key(k), value(v) { intialize_forward(forward_size); }
    SkipNode(const KeyType k, const int forward_size) : key(k) { intialize_forward(forward_size); }
    ~SkipNode() { delete forward; }

    void intialize_forward(const int forward_size)
    {
      forward = new SkipNode *[forward_size];
      for (auto i = 0; i != forward_size; ++i)
      {
        forward[i] = nullptr;
      }
    }

    KeyType key;
    ValueType value;

    // Array of forward nodes
    SkipNode **forward;

    int node_level() const;
  };

  int random_level() const;
  SkipNode *find_node(const KeyType searchKey) const;
  SkipNode *head;
  SkipNode *NIL;

  static const int max_levels = 16;
  const float probability;
};

SKIPLIST_TEMPLATE_ARGS
SKIPLIST_TYPE::SkipList() : probability(0.5)
{
  head = new SkipNode(std::numeric_limits<KeyType>::min(), max_levels);
  NIL = new SkipNode(std::numeric_limits<KeyType>::max(), max_levels);
  head->forward[0] = NIL;
}

SKIPLIST_TEMPLATE_ARGS
int SKIPLIST_TYPE::SkipNode::node_level() const
{
  int level;
  for (level = 0; level != max_levels; ++level)
  {
    if (forward[level] == nullptr)
      break;
  }
  return level;
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::print(std::ostream &os) const
{
  auto x = head;
  while (x->forward[0]->key != std::numeric_limits<KeyType>::max())
  {
    x = x->forward[0]; // traverse to the right
    os << "Key: " << x->key << " Value: " << x->value << " Level: " << x->node_level() << std::endl;
  }
}

SKIPLIST_TEMPLATE_ARGS
ValueType *SKIPLIST_TYPE::search(const KeyType searchKey) const
{
  auto x = head;
  // traverse from top of head. Forward size of head is list level
  for (int i = head->node_level() - 1; i >= 0; --i)
  {
    while (x->forward[i] && x->forward[i]->key < searchKey)
    {
      x = x->forward[i]; // traverse to the right
    }
  }

  x = x->forward[0];
  return (x->key == searchKey ? &x->value : nullptr);
}

SKIPLIST_TEMPLATE_ARGS
int SKIPLIST_TYPE::random_level() const
{
  int new_level = 1;

  while (((double)rand() / (RAND_MAX)) < probability)
  {
    ++new_level;
  }
  return (new_level > max_levels ? max_levels : new_level);
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::insert(const KeyType key, const ValueType &val)
{
  auto x = head;
  auto update = new SkipNode *[max_levels];

  // traverse from top of head. Forward size of head is list level
  for (int i = head->node_level() - 1; i >= 0; --i)
  {
    while (x->forward[i] && x->forward[i]->key < key)
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

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::remove(const KeyType key)
{
  auto x = head;
  auto update = new SkipNode *[max_levels];

  // traverse from top of head. Forward size of head is list level
  for (int i = head->node_level() - 1; i >= 0; --i)
  {
    while (x->forward[i] && x->forward[i]->key < key)
    {
      x = x->forward[i]; // traverse to the right
    }
    update[i] = x; // store last forward pointer
  }
  x = x->forward[0];
  if (x->key == key)
  {
    for (int i = 0; i != head->node_level() - 1; ++i)
    {
      if (update[i]->forward[i] != x)
        break;
      update[i]->forward[i] = x->forward[i];
    }
    x = nullptr;
  }
  delete update;
  return;
}

#endif