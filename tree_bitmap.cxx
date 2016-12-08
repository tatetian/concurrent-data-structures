#include "tree_bitmap.h"
#include <cstddef>
#include <cassert>

#define ROUND_UP(n, align)	(((n) + 1) / (align) * (align))

TreeBitmap::TreeBitmap(size_t nbits) :
	m_nbits(ROUND_UP(nbits, 32)),
	m_root(0),
	m_leafs(NULL)
{
	assert(m_nbits <= TREE_BITMAP_MAX_NBITS);
	m_leafs = new atomic<unsigned int>[m_nbits / 32];
	for (size_t li = 0; li < m_nbits / 32; li++)
		m_leafs[li] = 0;
}

TreeBitmap::~TreeBitmap() {
	delete [] m_leafs;
}

unsigned int TreeBitmap::set(size_t index) {
	if (index >= m_nbits) return -1;

	size_t leaf_i = index / 32;
	atomic<unsigned int>& leaf = m_leafs[leaf_i];
	leaf |= 1 << (index % 32);
	m_root |= 1 << leaf_i;
	return 0;
}

unsigned int TreeBitmap::extract(size_t *index) {
	unsigned int root_bits = m_root;
	if (root_bits == 0) return 0;
	size_t leaf_i = __builtin_ctz(root_bits);
	atomic<unsigned int>& leaf = m_leafs[leaf_i];
	unsigned int leaf_bits = leaf.exchange(0);
	m_root &= ~(1 << leaf_i);
	if (leaf != 0) m_root |= 1 << leaf_i;

	*index = leaf_i * 32;
	return leaf_bits;
}
