#ifndef _PTRLIST_H_
#define _PTRLIST_H_

class CPtrList
{
public:
	CPtrNode *first;

	CPtrList(void) { first = nil; }
	~CPtrList(void) { Flush(); }
	CPtrNode *FindItem(void *item){
		CPtrNode *node;
		for(node = first; node; node = node->next)
			if(node->item == item)
				return node;
		return nil;
	}
	CPtrNode *InsertNode(CPtrNode *node){
		node->prev = nil;
		node->next = first;
		if(first)
			first->prev = node;
		first = node;
		return node;
	}
	CPtrNode *InsertItem(void *item){
		CPtrNode *node = new CPtrNode;
		node->item = item;
		InsertNode(node);
		return node;
	}
	void RemoveNode(CPtrNode *node){
		if(node == first)
			first = node->next;
		if(node->prev)
			node->prev->next = node->next;
		if(node->next)
			node->next->prev = node->prev;
	}
	void DeleteNode(CPtrNode *node){
		RemoveNode(node);
		delete node;
	}
	void Flush(void){
		CPtrNode *node, *next;
		for(node = first; node; node = next){
			next = node->next;
			DeleteNode(node);
		}
	}
};

#endif
