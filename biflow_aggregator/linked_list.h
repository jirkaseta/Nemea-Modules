#ifndef ABC
#define ABC

#include<iostream>
#include<cstdio>
#include<cstdlib>
using namespace std;
#include <assert.h>


template<typename T>
struct node
{
    T value;
    struct node<T>* next;
    struct node<T>* prev;
};

template<typename T>
class Dll
{
    struct node<T>* head;
    struct node<T>* tail;

public:
int rrd= 0;

    Dll()
    {
        head = nullptr;
        tail = nullptr;
    }

    inline struct node<T> *begin() noexcept
    {
        return head;
    }

    void insert(struct node<T> *node)
    {
        if (head == nullptr) {
            head = node;
            node->prev = nullptr;
            node->next = nullptr;
            tail = node;
        } else {
            node->prev = tail;
            tail->next = node;
            node->next = nullptr;
            tail = node;
        }
    }

    void deleteFirst() 
    {
        if (head == nullptr)
            return;
    
        if (head == tail) {
            head = nullptr;
            tail = nullptr;
            return;
        } else {
            head = head->next;
            head->prev = nullptr;
        }
    }

    void swap(node<T> *node)
    {
        struct node<T>* prev;
        struct node<T>* next;
        if (tail == node)
            return;
        else if (head == node) {
            head = head->next;
            tail->next = node;
            node->next = nullptr;
            node->prev = tail;
            head->prev = nullptr;
            tail = node;
        } else {
            prev = node->prev;
            next = node->next;
            prev->next = next;
            next->prev = prev;
            tail->next = node;
            node->prev = tail;
            node->next = nullptr;
            tail = node;
        }
    }

};

#endif