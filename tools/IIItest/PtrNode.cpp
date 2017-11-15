#include "III.h"

void*
CPtrNode::operator new(size_t)
{
	CPtrNode *node = CPools::GetPtrNodePool()->New();
	assert(node);
	return node;
}

void
CPtrNode::operator delete(void *p, size_t)
{
	CPools::GetPtrNodePool()->Delete((CPtrNode*)p);
}
