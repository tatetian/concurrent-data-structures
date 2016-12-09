#ifndef __TREE_BITMAP_H__
#define __TREE_BITMAP_H__

#include <atomic>
using namespace std;



class TreeBitmap {
public:
	TreeBitmap(size_t nbits);
	~TreeBitmap();
	/* Time-complexity: O(log(n)) */
	unsigned int set(size_t bit_i);
	/* Time complexity:
	 * O(n) for worst case scenario,
	 * O(log(n)) for best case */
	size_t scan(size_t *bit_indexes, size_t size, size_t from_index = 0);
private:
	void _set(size_t node_i, size_t bit_i, int level);

	typedef struct _collector {
		size_t *indexes;
		size_t capacity;
		size_t size;
	} collector_t;
	void _scan(size_t node_i, collector_t* collector);

	size_t m_nbits;
	atomic<unsigned int>* m_nodes;
	size_t m_nnonleafs;
	size_t m_height;
};


#endif
