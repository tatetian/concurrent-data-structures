#ifndef __ARRAY_QUEUE_H__
#define __ARRAY_QUEUE_H__

#include <atomic>
using namespace std;

template<typename T>
class ArrayQueue {
public:
	ArrayQueue(size_t max_items);
	~ArrayQueue();
	int push(T* item);
	int pop(T* item);
private:
	size_t		m_max_items;
	atomic<int>	m_num_items;
	atomic<size_t>	m_producer_cnt;
	atomic<size_t>	m_consumer_cnt;
	T*		m_items;
};

#endif
