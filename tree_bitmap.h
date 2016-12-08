#ifndef __TREE_BITMAP_H__
#define __TREE_BITMAP_H__

#include <atomic>
using namespace std;

#define TREE_BITMAP_MAX_NBITS	(32 * 32)

class TreeBitmap {
public:
	TreeBitmap(size_t nbits);
	~TreeBitmap();
	unsigned int set(size_t index);
	unsigned int extract(size_t *index);
private:
	size_t m_nbits;
	atomic<unsigned int> m_root;
	atomic<unsigned int>* m_leafs;
};


#endif
