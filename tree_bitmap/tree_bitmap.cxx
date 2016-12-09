#include "tree_bitmap.h"
#include <cstddef>
#include <cassert>
#include <iostream>

#define ROUND_UP(n, align)	(((n) + 1) / (align) * (align))

#define K			32
#define LB_K			5			/* log_2(k) */
#define MASK			((1UL << LB_K) - 1)

#define CHILD(i, c)		(((i) << LB_K) + 1 + (c))
#define PARENT(i)		(((i) - 1) >> LB_K)
#define IS_NONLEAF(i)		((i) < m_nnonleafs)

/* floor(log_k(n)) */
int log_k_floor(unsigned long long n){
	assert(n != 0);
	int res = 0;
	while ((n >>= LB_K)) res++;
	return res;
}

/* ceil(log_k(n)) */
int log_k_ceil(unsigned long long n){
	assert(n != 0);
	return n > 1 ? log_k_floor(n - 1) + 1 : 0;
}

TreeBitmap::TreeBitmap(size_t nbits) :
	m_nbits(ROUND_UP(nbits, K)),
	m_nodes(NULL),
	m_nnonleafs(0),
	m_height(0)
{
	assert(m_nbits >= K);

	size_t nleafs = m_nbits / K;
	size_t height = log_k_ceil(nleafs) + 1;
	size_t nnonleafs = ((1 << ((height - 1) * LB_K)) - 1) / (K - 1);
	size_t nnodes = nleafs + nnonleafs;

	m_nodes = new atomic<unsigned int>[nnodes];
	for (size_t ni = 0; ni < nnodes; ni++)
		m_nodes[ni] = 0;
	m_height = height;
	m_nnonleafs = nnonleafs;
#if 0
	cout << "m_nbits = " << m_nbits << ", "
	     << "m_nleafs = " << nleafs << ", "
	     << "m_height = " << m_height << ", "
	     << "m_nonleafs = " << nnonleafs << ", "
	     << "m_nnodes = " << nnodes << endl;
#endif

}

TreeBitmap::~TreeBitmap() {
	delete [] m_nodes;
}

void TreeBitmap::_set(size_t node_i, size_t bit_i, int level) {
	int c = (bit_i >> ((m_height - 1 - level) * LB_K)) & MASK;
	if (IS_NONLEAF(node_i)) {
		_set(CHILD(node_i, c), bit_i, level + 1);
	}
	m_nodes[node_i] |= (1UL << c);
}

unsigned int TreeBitmap::set(size_t bit_i) {
	if (bit_i >= m_nbits) return -1;
	_set(0, bit_i, 0);
	return 0;
}

size_t TreeBitmap::scan(size_t *bit_indexes, size_t size, size_t from_index) {
	collector_t collector = {bit_indexes, size, 0};
	_scan(0, &collector);
	return collector.size;
}

void TreeBitmap::_scan(size_t node_i, collector_t* _collector) {
#define IS_COLLECTOR_FULL	((_collector)->capacity == (_collector)->size)
#define COLLECT(index)		do {				\
	(_collector)->indexes[(_collector)->size] = (index);	\
	(_collector)->size++;					\
} while(0)
#define MASK_LOW_BITS(c)	((c < 32) ? ~((1UL << (c)) - 1) : 0UL)
#define FOR_ALL_ONE(ci, bits)								\
	for (int (ci) = ((bits) ? __builtin_ctz(bits) : 0), _remain_bits = (bits);	\
	     _remain_bits;				\
	     (_remain_bits = (bits) & MASK_LOW_BITS(++(ci))), (ci) = __builtin_ctz(_remain_bits))

	unsigned int node_bits = m_nodes[node_i].exchange(0);
	unsigned int used_bits = 0;

	if (IS_NONLEAF(node_i)) {
		FOR_ALL_ONE(ci, node_bits) {
			if (IS_COLLECTOR_FULL) break;

			size_t child_i = CHILD(node_i, ci);
			_scan(child_i, _collector);

			if (m_nodes[child_i] == 0) used_bits |= (1UL << ci);
		}
	}
	else {
		size_t base_index = (node_i - m_nnonleafs) << LB_K;

		FOR_ALL_ONE(ci, node_bits) {
			if (IS_COLLECTOR_FULL) break;
			COLLECT(base_index + ci);
			used_bits |= (1UL << ci);
		}
	}

	// Restore bits which is not used or should be set
	unsigned int restore_bits = node_bits & (~used_bits);
	m_nodes[node_i] |= restore_bits;

#undef IS_COLLECTOR_FULL
#undef COLLECT
#undef MASK_LOW_BITS
#undef FOR_ALL_ONE
}

