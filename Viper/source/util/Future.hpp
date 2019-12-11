#pragma once
#include <concurrentqueue.h>

template<typename T>
class Future
{
public:
	bool tryGet(T& get)
	{
		return queue.try_dequeue(get);
	}

	T get()
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