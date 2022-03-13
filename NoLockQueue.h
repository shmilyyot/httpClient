#pragma once
#include <atomic>

template<class T>
class NoLockQueue
{
    struct Node
    {
        T _value;
        std::atomic<Node*> _next;
        Node(const T&& value) : _value(std::move(value)), _next(nullptr) {};
        Node(const T value) : _value(value), _next(nullptr) {};
    };

    public:
    NoLockQueue()
    {
        _head = _tail = new Node(T());
    }

    ~NoLockQueue()
    {
        Node *cur = _head;
        while(cur)
        {
            Node *next = cur->_next;
            delete cur;
            cur = next;
        }
    }

    void push(const T x)
    {
        Node *newnode = new Node(x);
        Node *oldtail = nullptr;
        Node *nullnode = nullptr;
        do
        {
            oldtail = _tail.load();
        } 
        while (oldtail->_next.compare_exchange_weak(nullnode, newnode) != true);
        _tail.compare_exchange_weak(oldtail, newnode);
    }

    T pop()
    {
        Node *oldhead = _head.load();
        T ret;
        do
        {
            Node *next = oldhead->_next;
            if(next == nullptr)
            {
                return T();
            }
            else
            {
                ret = next->_value;
            }

        } 
        while (_head.compare_exchange_weak(oldhead, oldhead->_next) != true);
        delete oldhead;
        return ret;
    }

    bool empty()
    {
        return _head.load() == _tail.load();
    }



    private:
        std::atomic<Node*> _head;
        std::atomic<Node*> _tail;
};