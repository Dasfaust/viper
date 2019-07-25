#pragma once
#include <concurrentqueue.h>

template<typename T>
class Future
{
public:
	T& tryGet()
	{
		T get;
		queue.try_dequeue(get);
		return t;
	}

	T& get()
	{
		T get;
		while(!queue.try_dequeue(get)) { }
		return get;
	}

	void post(T& what)
	{
		queue.enqueue(what);
	}
private:
	moodycamel::ConcurrentQueue<T> queue;
};