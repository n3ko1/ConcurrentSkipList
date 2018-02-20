#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <string>
#include <iostream>
#include <limits>
#include <random>
#include <atomic>
#include <cstdint>

#define OUT_PARAM

template <class T>
struct MarkableReference
{
  T *val = nullptr;
  bool marked = false;
  MarkableReference(MarkableReference &other) : val(other.val), marked(other.marked) {}
  MarkableReference(T *value, bool mark) : val(value), marked(mark) {}
};

// Atomic wrapper for MarkableReference type, implementing copy constructor for use in container types
// ATTENTION: copy construction is NOT atomic!
template <typename T>
struct AtomicMarkableReference
{
  std::atomic<MarkableReference<T> *> ref;

  AtomicMarkableReference()
  {
    ref.store(new MarkableReference<T>(nullptr, false));
  }

  AtomicMarkableReference(T *val, bool marked)
  {
    ref.store(new MarkableReference<T>(val, marked));
  }

  T *get_reference() { return ref.load()->val; }

  // Stores the value of this references marked flag in reference
  T *get(OUT_PARAM bool &mark)
  {
    MarkableReference<T> *temp = ref.load();
    mark = temp->marked;
    return temp->val;
  }

  void set(T *value, bool mark)
  {
    MarkableReference<T> *curr = ref.load();
    if (value != curr->val || mark != curr->marked)
    {
      ref.store(new MarkableReference<T>(value, mark));
    }
  }

  // Atomically sets the value of both the reference and mark to the given
  // update values if the current reference is equal to the expected reference
  // and the current mark is equal to the expected mark. returns true on success
  bool compare_and_swap(T *expected_value, bool expected_mark, T *new_value, bool new_mark)
  {
    MarkableReference<T> *curr = ref.load();
    return (expected_value == curr->val && expected_mark == curr->marked &&
            ((new_value == curr->val && new_mark == curr->marked) ||                             // if already equal, return true by shortcircuiting
             ref.compare_exchange_strong(curr, new MarkableReference<T>(new_value, new_mark)))); // otherwise, attempt compare and swap
  }
};

// BEGIN SKIPLIST IMPLEMENTATION
#define SKIPLIST_TEMPLATE_ARGS \
  template <typename KeyType, typename ValueType>

#define SKIPLIST_TYPE \
  SkipList<KeyType, ValueType>

SKIPLIST_TEMPLATE_ARGS
class SkipList
{
private:
  struct SkipNode
  {
    // Constructor for standard nodes
    SkipNode(const KeyType k, const ValueType &v, const int forward_size) : key(k), value(v), top_level(forward_size)
    {
      intialize_forward(forward_size, nullptr);
    }

    // Constructor for sentinel nodes head and NIL
    SkipNode(const KeyType k, const int forward_size, SkipNode *forward_target) : key(k), top_level(forward_size)
    {
      intialize_forward(forward_size, forward_target);
    }
    ~SkipNode() {}

    void intialize_forward(const int forward_size, SkipNode *forward_target)
    {
      forward = std::vector<AtomicMarkableReference<SkipNode>>(forward_size);
      for (auto i = 0; i != forward_size; ++i)
      {
        forward[i].set(forward_target, false);
      }
    }

    KeyType key;
    ValueType value;

    int top_level;

    // Vector of atomic, markable (logical delete) forward nodes
    std::vector<AtomicMarkableReference<SkipNode>> forward;

    int node_level() const;
  };

public:
  SkipList();
  ~SkipList()
  {
    delete head;
    delete NIL;
  };

  ValueType *search(const KeyType search_key) const;
  bool find(const KeyType search_key, SkipNode **preds, SkipNode **succs); // not const, will delete marked nodes

  void insert(const KeyType key, const ValueType &val);
  void remove(const KeyType key);

  void print(std::ostream &os) const;
  uint32_t size() const;

private:
  int random_level() const;
  SkipNode *head;
  SkipNode *NIL;

  static const int max_levels = 16;
  const float probability;
};

SKIPLIST_TEMPLATE_ARGS
SKIPLIST_TYPE::SkipList() : probability(0.5)
{
  NIL = new SkipNode(std::numeric_limits<KeyType>::max(), max_levels + 1, nullptr);
  head = new SkipNode(std::numeric_limits<KeyType>::min(), max_levels + 1, NIL);
}

// TODO optimize http://ticki.github.io/blog/skip-lists-done-right/
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
void SKIPLIST_TYPE::print(std::ostream &os) const
{
  auto x = head;
  while (x->forward[0].get_reference()->key != std::numeric_limits<KeyType>::max())
  {
    x = x->forward[0].get_reference(); // traverse to the right
    os << "Key: " << x->key << " Value: " << x->value << " Level: " << x->top_level << std::endl;
  }
}

SKIPLIST_TEMPLATE_ARGS
uint32_t SKIPLIST_TYPE::size() const
{
  auto x = head;
  uint32_t size = 0;
  while (x->forward[0].get_reference()->key != std::numeric_limits<KeyType>::max())
  {
    x = x->forward[0].get_reference(); // traverse to the right
    ++size;
  }
  return size;
}

SKIPLIST_TEMPLATE_ARGS
bool SKIPLIST_TYPE::find(const KeyType search_key, SkipNode **preds, SkipNode **succs)
{
  bool marked = false;
  bool snip;

  SkipNode *pred, *curr, *succ;

RETRY:
  pred = head;
  for (auto level = max_levels; level >= 0; --level)
  {
    curr = pred->forward[level].get_reference();
    while (true)
    {
      // delete marked nodes
      succ = curr->forward[level].get(marked);
      while (marked)
      {
        snip = pred->forward[level].compare_and_swap(curr, false, succ, false);
        if (!snip)
          goto RETRY; // CAS failed, try again
        curr = pred->forward[level].get_reference();
        succ = curr->forward[level].get(marked);
      }
      if (curr->key < search_key)
      {
        pred = curr;
        curr = succ;
      }
      else
      {
        break;
      }
    }
    preds[level] = pred;
    succs[level] = curr;
  }
  return (curr->key == search_key);
}

SKIPLIST_TEMPLATE_ARGS
ValueType *SKIPLIST_TYPE::search(const KeyType search_key) const
{
  auto x = head;
  // traverse from top of head. Forward size of head is list level
  for (int i = head.get_reference()->node_level() - 1; i >= 0; --i)
  {
    while (x->forward[i].get_reference() && x->forward[i].get_reference()->key < search_key)
    {
      x = x->forward[i]; // traverse to the right
    }
  }

  x = x->forward[0];
  return (x.get_reference()->key == search_key ? &x.get_reference()->value : nullptr);
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::insert(const KeyType key, const ValueType &val)
{
  int top_level = random_level();
  auto preds = new SkipNode *[max_levels + 1];
  auto succs = new SkipNode *[max_levels + 1];
  while (true)
  {
    bool found = find(key, preds, succs);
    if (found)
    {
      delete preds;
      delete succs;
      return;
    }
    else
    {
      auto new_node = new SkipNode(key, val, top_level);
      for (auto level = 0; level < top_level; ++level)
      {
        auto succ = succs[level];
        new_node->forward[level].set(succ, false);
      }
      auto pred = preds[0];
      auto succ = succs[0];
      new_node->forward[0].set(succ, false);
      if (!pred->forward[0].compare_and_swap(succ, false, new_node, false))
      {
        delete new_node;
        continue; // CAS failed, try again
      }
      for (auto level = 0; level < top_level; ++level)
      {
        while (true)
        {
          pred = preds[level];
          succ = succs[level];
          if (pred->forward[level].compare_and_swap(succ, false, new_node, false))
            break;
          find(key, preds, succs); // CAS failed for upper level, search node to update preds and succs
        }
      }
      delete preds;
      delete succs;
      return;
    }
  }
}

SKIPLIST_TEMPLATE_ARGS
void SKIPLIST_TYPE::remove(const KeyType key)
{
  // auto x = head;
  // auto update = new SkipNode *[max_levels];

  // // traverse from top of head. Forward size of head is list level
  // for (int i = head->node_level() - 1; i >= 0; --i)
  // {
  //   while (x->forward[i] && x->forward[i]->key < key)
  //   {
  //     x = x->forward[i]; // traverse to the right
  //   }
  //   update[i] = x; // store last forward pointer
  // }
  // x = x->forward[0];
  // if (x->key == key)
  // {
  //   for (int i = 0; i != head->node_level() - 1; ++i)
  //   {
  //     if (update[i]->forward[i] != x)
  //       break;
  //     update[i]->forward[i] = x->forward[i];
  //   }
  //   x = nullptr;
  // }
  // delete update;
  return;
}

#endif