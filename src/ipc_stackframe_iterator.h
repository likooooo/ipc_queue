#pragma once
#include <iterator>
template <class T> struct reverse_like_iterator 
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = std::ptrdiff_t;
    using pointer           = T*;
    using reference         = T&;

    reverse_like_iterator() : ptr_(nullptr) {}
    explicit reverse_like_iterator(pointer p) : ptr_(p) {}

    reference operator*() const { return *ptr_; }
    reference operator[](difference_type n) const { return *(*this + n); }
    pointer operator->() const { return ptr_; }
    pointer base() const { return ptr_; }

    reverse_like_iterator& operator++() 
    {
        --ptr_;
        return *this;
    }
    reverse_like_iterator& operator--() 
    {
        ++ptr_;
        return *this;
    }
    reverse_like_iterator& operator+=(difference_type n) 
    {
        ptr_ -= n;
        return *this;
    }
    reverse_like_iterator& operator-=(difference_type n) 
    {
        ptr_ += n;
        return *this;
    }
    reverse_like_iterator operator++(int) 
    {
        reverse_like_iterator tmp = *this;
        --ptr_;
        return tmp;
    }
    reverse_like_iterator operator--(int) 
    {
        reverse_like_iterator tmp = *this;
        ++ptr_;
        return tmp;
    }
    reverse_like_iterator operator+(difference_type n) const 
    {
        return reverse_like_iterator(ptr_ - n);
    }
    reverse_like_iterator operator-(difference_type n) const 
    {
        return reverse_like_iterator(ptr_ + n);
    }
    difference_type operator-(const reverse_like_iterator& other) const 
    {
        return other.ptr_ - ptr_;
    }

    auto operator<=>(const reverse_like_iterator& other) const = default;
private:
    pointer ptr_;
};

template <typename T>
reverse_like_iterator<T> make_reverse_like_iterator(T* p) {
    return reverse_like_iterator<T>(p);
}

#include "ipc_stackframe.h"

struct ipc_stackframe_loop_adapter
{
    using iterator = reverse_like_iterator<ipc_stackframe>;
    using const_iterator = reverse_like_iterator<const ipc_stackframe>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    ipc_stackframe_loop_adapter(ipc_queue& queue) : q(queue) {}

    iterator begin() 
    {
        return iterator(&ipc_stackframe::base_pointer(q));
    }
    iterator end() 
    {
        return iterator(ipc_stackframe::move_stack_next(&ipc_stackframe::stack_pointer(q)));
    }
    const_iterator cbegin()const
    {
        return const_iterator(&ipc_stackframe::base_pointer(q));
    }
    const_iterator cend()const
    {
        return const_iterator(ipc_stackframe::move_stack_next(&ipc_stackframe::stack_pointer(q)));
    }
    reverse_iterator rbegin() 
    { 
        return reverse_iterator(end()); 
    }
    reverse_iterator rend()   
    { 
        return reverse_iterator(begin()); 
    }
    const_reverse_iterator crbegin() const 
    { 
        return const_reverse_iterator(cend()); 
    }
    const_reverse_iterator crend()   const 
    {
         return const_reverse_iterator(cbegin()); 
    }
private:
    ipc_queue& q;
};