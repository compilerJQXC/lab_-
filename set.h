#ifndef SET_H
#define SET_H

typedef struct snode
{
	int elem;
	struct snode* next;
} snode, *symset;

symset phi, declbegsys, statbegsys, facbegsys, relset;

symset createset(int data, .../* SYM_NULL */);	//����symset
void destroyset(symset s);				//����s
symset uniteset(symset s1, symset s2);	//�ϲ�s1��s2 Ϊ s
int inset(int elem, symset s);			//elem��s�����1,���򣬷���0

#endif
// EOF set.h
