#include "array_queue.h"
#include <cstddef>
#include <cassert>
#include <iostream>

ArrayQueue<T>::ArrayQueue(size_t max_items)
	: m_max_items(max_items)
	, m_num_items(0)
	, m_items(NULL)
	, m_producer_cnt(0)
	, m_consumer_cnt(0)
{
	assert(m_max_items > 0);

	m_items = new T[m_max_items];
}

ArrayQueue::~ArrayQueue() {
	delete [] m_items;
}

int ArrayQueue::push(T* item) {
	m_items[m_producer_cnt.fetch_and_add(1) % m_max_items] = *items;
	m_num_items += 1;
	return 0;
}

int ArrayQueue::pop(T* item) {
	int num_items = m_num_items.fetch_and_sub(1);
	if (num_items <= 0) {
		m_num_items += 1;
		return -1;
	}
	*item = m_items[m_consumer_cnt.fetch_and_add(1) % m_max_items];
	return 0;
}

