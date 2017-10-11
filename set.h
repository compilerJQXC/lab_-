#ifndef SET_H
#define SET_H

typedef struct snode
{
	int elem;
	struct snode* next;
} snode, *symset;

symset phi, declbegsys, statbegsys, facbegsys, relset;

symset createset(int data, .../* SYM_NULL */);	//创建symset
void destroyset(symset s);				//销毁s
symset uniteset(symset s1, symset s2);	//合并s1，s2 为 s
int inset(int elem, symset s);			//elem在s里，返回1,否则，返回0

#endif
// EOF set.h
