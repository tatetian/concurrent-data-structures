#include <iostream>
#include <bitset>
#include <thread>
#include <mutex>
#include <ctime>
#include <cstdlib>
#include "array_queue.h"
using namespace std;

#define MAX_COUNT		100000
#define MAX_NPRODUCERS		8
#define MAX_NCONSUMERS		8

atomic<size_t> sum_items;
atomic<size_t> total_items;

volatile int consumed[MAX_NPRODUCERS] = {0};

typedef struct _item {
	int	producer_id;
	size_t	item_val;
} item_t;

void produce(ArrayQueue<item_t> *aq, int producer_id, int count) {
	while (count--) {
		size_t item_val = total_items.fetch_and_add(1);
		item_t item = { producer_id, item_val };
		aq->push(&item);

		consumed[producer_id] = 0;
		while (consumed[producer_id] == 0);
	}
}

volatile int keep_consuming = 1;

void consume(ArrayQueue<item_t> *aq) {
	item_t item;
	while (1) {
		if (aq->pop(&item)) {
			if (keep_consuming) continue;
			if (aq->pop(&item)) break;
		}

		sum_items += item->item_val;
		consumed[item->producer_id] = 1;
	}
}

int test_case(size_t count, int nproducers, int nconsumers) {
	sum_items = 0;
	total_items = 0;

	// Start producers
	count = (count / nproducers + 1) * nproducers;
	thread** producer_threads = new thread*[nproducers];
	for (int pi = 0; pi < nproducers; pi++) {
		producer_threads[pi] = new thread(produce, aq, pi,
						count / nproducers);
	}
	// Start consumers
	keep_consuming = 1;
	thread** consumer_threads = new thread*[nconsumers];
	for (int ci = 0; ci < nconsumers; ci++) {
		consumer_threads[pi] = new thread(consume, aq);
	}

	// Wait for producers
	for (int pi = 0; pi < nproducers; pi++)
		producer_threads[pi].join();
	// Wait for consumers
	keep_consuming = 0;
	for (int ci = 0; ci < nconsumers; ci++)
		consumer_threads[ci].join();

	// Check result
	size_t expected_sum_items = (0 + count - 1) * count / 2;
	int failed = expected_sum_items != sum_items;
	if (failed) {
		cout << "count = " << count << ", "
		     << "nproducers = " << nproducers << ", "
		     << "nconsumers = " << nconsumers << ", "
		     << "expected_sum = " << expected_sum_items << ","
		     << "actual_sum = " << sum_items << endl;
	}

	// Clean up
	for (int pi = 0; pi < nproducers; pi++)
		delete producer_threads[pi];
	delete [] producer_threads;
	for (int ci = 0; ci < nconsumers; ci++)
		delete consumer_threads[ci];
	delete [] consumer_threads;

	return failed;
}

int main() {
	srand(time(NULL));
	for (int tc_i = 0; /*tc_i < NUM_TESTCASES*/; tc_i++) {
		//cout << "#" << tc_i << " ";
		size_t count = MAX(rand() % MAX_COUNT, 100);
		int nproducers = MAX(rand() % MAX_NPRODUCERS, 1);
		int nconsumers = MAX(rand() % MAX_NCONSUMERS, 1);

		failed = test_case(count, nproducers, nconsumers);
		if (failed) break;
		tc_i++;
		if (tc_i % 100 == 0) {
			cout << tc_i << endl;
		}
	}
	//}
	return 0;
}
