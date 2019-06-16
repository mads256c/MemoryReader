#pragma once
template <typename T>
struct ListNode
{
	explicit ListNode(T data, ListNode<T>* prev, ListNode<T>* next) : data(data), prev(prev), next(next)
	{

	}
	~ListNode()
	{
		delete data;
	}

	T data;
	ListNode<T>* prev = nullptr;
	ListNode<T>* next = nullptr;
};

