#include <iostream>
#include <bitset>
#include <thread>
#include <mutex>
#include <ctime>
#include <cstdlib>
#include "tree_bitmap.h"
using namespace std;

#define MAX(a, b)	((a) >= (b) ? (a) : (b))
//#define MAX_NBITS	TREE_BITMAP_MAX_NBITS
#define MAX_NBITS	128
#define MAX_NPRODUCERS	8
#define NUM_TESTCASES	1024

#define ROUND_UP(n, align)	(((n) + 1) / (align) * (align))

volatile bool keep_consuming = 1;

mutex producer_mtx;

void produce(int count, bitset<MAX_NBITS> *bitset, TreeBitmap *tb, size_t nbits) {
	while (count--) {
		size_t bit_i = rand() % nbits;
		producer_mtx.lock();
		bitset->set(bit_i);
		producer_mtx.unlock();
		tb->set(bit_i);
	}
}

void consume(bitset<MAX_NBITS> *bitset, TreeBitmap *tb, size_t nbits) {
	size_t index_base;
	unsigned int extracted_bits = 0;
	while (1) {
		extracted_bits = tb->extract(&index_base);
		if (extracted_bits == 0 && keep_consuming == 0) {
			extracted_bits = tb->extract(&index_base);
			if (extracted_bits == 0) break;
		}

		size_t index = index_base;
		unsigned int consumed_bits = extracted_bits;
		while (consumed_bits) {
			if (consumed_bits & 1) {
				bitset->set(index);
			}
			consumed_bits >>= 1;
			index++;
		}
	} ;
}

bitset<MAX_NBITS> bitset_xor(bitset<MAX_NBITS> a, bitset<MAX_NBITS> b) {
	bitset<MAX_NBITS> res;
	for (int i = 0; i < MAX_NBITS; i++) {
		if (a[i] != b[i]) res.set(i);
	}
	return res;
}

int test_case(size_t nbits, size_t count, int nproducers) {
	bitset<MAX_NBITS> expected;
	bitset<MAX_NBITS> actual;
	TreeBitmap tb(nbits);

	keep_consuming = 1;
	thread consumer(consume, &actual, &tb, nbits);
	thread** producers = new thread*[nproducers];
	for (int pi = 0; pi < nproducers; pi++)
		producers[pi] = new thread(produce, MAX(count / nproducers, 1), &expected, &tb, nbits);

	for (int pi = 0; pi < nproducers; pi++)
		producers[pi]->join();
	keep_consuming = 0;
	consumer.join();

	for (int pi = 0; pi < nproducers; pi++)
		delete producers[pi];
	delete [] producers;

	bool test_passed = expected == actual;
	//cout << (test_passed ? "Passed ^_^" : "Failed -_-") << endl;
	if (!test_passed) {
		cout << "Test Case (nbits = " << nbits
		      << ", count = " << count
		      << ", nproducers = " << nproducers << ")" << endl;
		cout << expected << endl;
		cout << actual << endl;
		cout << bitset_xor(expected, actual) << endl;
	}
	return test_passed;
}

int main() {
	srand(time(NULL));
	//for (int tc_i = 0; tc_i < NUM_TESTCASES; tc_i++) {
	int tc_i = 0;
	while (1) {
		//cout << "#" << tc_i << " ";
		size_t nbits = ROUND_UP(rand() % MAX_NBITS, 32);
		nbits = MAX(nbits, 32);
		size_t count = MAX(nbits * (rand() % 100) / 100, 1);
		int nproducers = MAX(rand() % MAX_NPRODUCERS, 1);
		int passed = test_case(nbits, count, nproducers);
		//int passed = test_case(64, 32, 2);
		//if (!passed) break;
		tc_i++;
		if (tc_i % 100000 == 0) {
			cout << tc_i << endl;
		}
	}
	//}
	return 0;
}
