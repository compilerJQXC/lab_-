// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pl0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message. ������ִ�����Ϣ
/*
	��ӡ�����±�Ϊn��err_msg����Ϣ
*/
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
/*
������void getch��void�� ��ȡһ���ַ������ظ�ch��line�������ÿո���桮\n��
1. ��ȡ�����ַ��Ĺ���
2. ʶ�����������з�
3. ������Դ�ļ���д������ļ�
4. ����һ�ݳ����б��������Ӧ�кŻ���ָ���������ֵ
*/

void getch(void)
{
	if (cc == ll)		//cc: character count \ ll: line length
	{
		if (feof(infile))		//fefo : �ļ����������ط�0ֵ infile Ϊ���ڴ򿪵��ļ�
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx); //cx :index of current instruction to be generated
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))	//�������з�
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';	
	}
	ch = line[++cc];	//chÿ�ζ�һ���ַ�
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
/*
������void getsym(void) �ʷ�����
*/
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];	//MAXIDLEN : length of identifiers

	while (ch == ' '||ch == '\t')	//�����ո���Ʊ��
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;		//NRW Ϊ�����ֵ���Ŀ
		while (strcmp(id, word[i--]));		//word : �����ּ��� wsym : �����ֵ�����
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')	//��ʾ��ֵ
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '/')
	{
		getch();
		if (ch == '/')	//�ж�ע��
		{
			cc = ll;
			getch();
			getsym();
		}
		else if (ch == '*')
		{
			getch();
			while (1)
			{
				while (ch != '*')
				{
					getch();
				}
				getch();
				if (ch == '/')
				{
					break;
				}
			}
			getch();
			getsym();
		}
		else  //��������
		{
			sym = ssym[4];
		}
		
	}
/************************9.19�������������else if����*****************************/
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else
		{
			sym = SYM_BITSAND;
		}
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		else
		{
			sym = SYM_BITSOR;
		}
	}
/************************---------------------------*****************************/
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
/****************************9.19 ��������������� �����ж� ++ �� -- *************************/
			if (sym == SYM_PLUS   &&  ch == '+')
			{
				sym = SYM_DPLUS;
				getch();
			}
			else if (sym == SYM_MINUS && ch == '-')
			{
				sym = SYM_DMINUS;
				getch();
			}
/****************************-------------------------------------*************************/
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
/*
��������void gen(int x, int y, int z)
���ܣ�	��x��y��z����code����
*/
void gen(int x, int y, int z)
{
	if (cx > CXMAX)	//��ǰ���ɴ�����кŴ������������������
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;		//f: function code
	code[cx].l = y;		//l: level
	code[cx++].a = z;	//a: displacement address	
} // gen

//////////////////////////////////////////////////////////////////////
/*
������void test(symset s1, symset s2, int n)
���ܣ�
	tests if error occurs and skips all symbols that do not belongs to s1 or s2.
	�����Ƿ���ִ��� ���������в�����s1��s2�ķ��ţ����ǳ���ָ�����
	1. ���������һ�����ż���S1�������ǰ���Ų��ڴ˼����У������õ�һ�������
	2. ��ӵ�ֹͣ���ż���S2����Щ���ŵĳ��֣���Ȼ�����Ǵ�ģ������Ǿ��Բ�Ӧ�ú��Զ�������
	3. ����n����ʾ�йش������Ϻ�
*/ 
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))	
	{
		//s1����sym��
		error(n);		//����
		s = uniteset(s1, s2);
		while(! inset(sym, s)) //ͨ��ѭ���ҵ���һ���Ϸ��ķ��ţ��Իָ��﷨��������
			getsym();
		destroyset(s);	//����s
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index
/*
������void enter(int kind)
���ܣ�
	enter object(constant, variable or procedre) into table.
	����ű�����µı�ʶ������ȷ����ʶ�����й�����
*/

void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:		//����������Ϊ����ֵ
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;	//table��comtab���� , ���������͹��̻ᱻǿ������ת��Ϊmask
		break;
	case ID_VARIABLE:		//����������Ϊ��κ���������ɵĵ�ַ
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:		//���̵������ǹ��̵���ڵ�ַ�Ͳ��
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
/*
��������int position(char* id)
���ܣ�
	locates identifier in symbol table.
	����id��table�е�λ�� ������λ�÷��أ����û�ҵ�������0
*/
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;		//txΪ���ű�ĳ���
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
/*
��������void constdeclaration()
���ܣ�
	��������������� 
*/
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)	//��ǰΪ��ʶ��
	{
		getsym();		//��ȡ��һ��token
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);		// const ident = number  ʶ����ɣ�����ǰid��������������ű�
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

//////////////////////////////////////////////////////////////////////
/*
��������void vardeclaration(void)
���ܣ�
	������������
*/
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);		//var ident  ʶ����ɣ�����ǰid��������������ű�
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
/*
������void listcode(int from, int to)
���ܣ��г���from��to��PL/0�������
*/
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
/*
������void factor(symset fsys)
���ܣ����Ӵ������
*/
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;
	/*
	��ʼ���Ӵ���ǰ���ȼ�鵱ǰtoken�Ƿ���facbegsys������
	facbegsyΪfactor�Ϸ��Ŀ�ʼ���ż���
	*/
	test(facbegsys, fsys, 24); //err_msg[24]: The symbol can not be as the beginning of an expression.
	
	while (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)	//����Ϊ��ʶ��
		{
			if ((i = position(id)) == 0)	//���ҷ��ű�
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:		//��ʶ���ǳ���
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:		//��ʶ���Ǳ���
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:		//��ʶ���ǹ���
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)	//����Ϊ����
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)	//����Ϊ'('
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys); //setΪ')'��fsys��null�ļ���
			expression(set);	//���ϱ��ʽ
			destroyset(set);
			if (sym == SYM_RPAREN)//����')'
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 expression(fsys);
			 gen(OPR, 0, OPR_NEG);
		}
		/***************9.30�������ķ�***************/
		else if (sym == SYM_NOT)	
		{
			getsym();
			expression(fsys);
			gen(OPR, 0, OPR_NOT);
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);//err_msg[23]:The symbol can not be followed by a factor.
	} // while
} // factor

//////////////////////////////////////////////////////////////////////
/*
������void term(symset fsys)
���ܣ�������
*/
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MODULE,SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_MODULE) //* �� /
	{
		mulop = sym;	//���浱ǰ�����
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else if (mulop == SYM_SLASH)
		{
			gen(OPR, 0, OPR_DIV);
		}
		else
		{
			gen(OPR, 0, OPR_MOD);
		}
	} // while
	destroyset(set);
} // term
 /***********��������10.10*���������һ��,ʵ�ְ�λ�������������************/
void terms_bitsand(symset fsys)  //��λ��
{
	//int op;
	symset set;
	set = uniteset(fsys, createset(SYM_BITSAND, SYM_NULL));
	term(set);
	//destroyset(set);
	while (sym == SYM_BITSAND)
	{
		//op = sym;
		getsym();
		term(set);
		gen(OPR, 0, OPR_BAND);
	}
	destroyset(set);
}
void terms_bitsxor(symset fsys)//��λ���
{
	symset set;
	set = uniteset(fsys, createset(SYM_BITSXOR, SYM_NULL));
	terms_bitsand(set);
	while (sym == SYM_BITSXOR)
	{
		getsym();
		terms_bitsand(set);
		gen(OPR, 0, OPR_BXOR);
	}
	destroyset(set);
}
void terms_bitsor(symset fsys)//��λ��
{
	symset set;
	set = uniteset(fsys, createset(SYM_BITSOR, SYM_NULL));
	terms_bitsxor(set);
	while (sym == SYM_BITSOR)
	{
		getsym();
		terms_bitsxor(set);
		gen(OPR, 0, OPR_BOR);
	}
	destroyset(set);
}
/****************��������10.10����ӡ�������************************/
//////////////////////////////////////////////////////////////////////
/*
������void expression(symset fsys)
���ܣ����ʽ�������
*/
void expression(symset fsys)
{
	int addop;
	symset set;

	//��fsys���½��ĺ���'+'��'-'������ϲ�
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS,SYM_NULL));//10.10�޸�
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;//�ѵ�ǰ���ű��棬���������¶�ȡ
		getsym();//��ȡ��һ��token
		term(set);//����������term
		if (addop == SYM_PLUS)//������������'+'
		{
			gen(OPR, 0, OPR_ADD);//���ɼ�ָ��
		}
		else
		{
			gen(OPR, 0, OPR_MIN);//���ɼ�ָ��
		}
	} // while

	destroyset(set);
} // expression
//////////////////////////////////////////////////////////////////////
/*
������void condition(symset fsys)
���ܣ������������,����fsys: �������������ָ��﷨�����ķ��ż���
*/
void condition(symset fsys)
{
	int relop;//������ʱ��¼token����
	symset set;

	if (sym == SYM_ODD)//�����odd�����
	{
		getsym();
		expression(fsys);//����expression ���д�������
		gen(OPR, 0, OPR_ODD);//����6��ָ���ż�ж�
	}
	else
	{
		/*
		relset�д洢�߼������
		relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
		'=','<>','<','<=','>','>=',''
		*/
/***����10.10�޸����´��룬Ŀ�ģ�������һ�㻯���������ʽҲ������Ϊ��������******/
		set = uniteset(relset, fsys);
		expression(set);	//�Ա��ʽ�󲿽��д�������
		destroyset(set);
/******��������10.10����ӡ�������************************/
		if(inset(sym, relset))
		{
			relop = sym;	//��¼��ǰ�߼����������
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition
/****��������10.10*���������һ�����������֤��λ���������ȼ�����>=************/
void conditions_bitsand(symset fsys)  //��λ��
{
	//int op;
	symset set;
	set = uniteset(fsys, createset(SYM_BITSAND, SYM_NULL));
	condition(set);
	//destroyset(set);
	while (sym == SYM_BITSAND)
	{
		//op = sym;
		getsym();
		condition(set);
		gen(OPR, 0, OPR_BAND);
	}
	destroyset(set);
}
void conditions_bitsxor(symset fsys)//��λ���
{
	symset set;
	set = uniteset(fsys, createset(SYM_BITSXOR, SYM_NULL));
	conditions_bitsand(set);
	while (sym == SYM_BITSXOR)
	{
		getsym();
		conditions_bitsand(set);
		gen(OPR, 0, OPR_BXOR);
	}
	destroyset(set);
}
void conditions_bitsor(symset fsys)//��λ��
{
	symset set;
	set = uniteset(fsys, createset(SYM_BITSOR, SYM_NULL));
	conditions_bitsxor(set);
	while (sym == SYM_BITSOR)
	{
		getsym();
		conditions_bitsxor(set);
		gen(OPR, 0, OPR_BOR);
	}
	destroyset(set);
}
/****************��������10.10����ӡ�������************************/
/*******************��������9.30�����������һ���������************************/
void conditions_and(symset fsys)  //�߼���
{
	//int op;
	symset set;
	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));
	conditions_bitsor(set);
	//destroyset(set);
	while (sym == SYM_AND)
	{
		//op = sym;
		getsym();
		conditions_bitsor(set);
		gen(OPR, 0, OPR_AND);
	}
	destroyset(set);
}

void conditions_or(symset fsys)//�߼���
{
	symset set;
	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));
	conditions_and(set);
	while (sym == SYM_OR)
	{
		getsym();
		conditions_and(set);
		gen(OPR, 0, OPR_OR);
	}
	destroyset(set);
}
/****************��������9.30����ӡ�������************************/
//////////////////////////////////////////////////////////////////////
/*
������void statement(symset fsys��
���ܣ����������̣�����fsys:�������������ָ��﷨�����ķ��ż���
*/
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)//����Ϊ��ʶ��
	{ // variable assignment
		mask* mk;
		if (! (i = position(id)))//id���ڷ��ű���  ����
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)	//����ڷ��ű����ҵ��ñ�ʶ���������Ǳ�������
		{
			error(12); // Illegal assignment.
			i = 0;	//i��0��Ϊ�����־
		}
		getsym();
		if (sym == SYM_BECOMES)//��ǰtokenΪ ��ֵ
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);//���ñ��ʽ�������expression
		mk = (mask*) &table[i];
		if (i)//�����������i����Ϊ0��i��ָΪ��ǰ��ʶ���ڷ��ű��е�λ��
		{
			gen(STO, level - mk->level, mk->address);//����һ�аѱ��ʽֵд��ָ���ڴ��STOĿ�����
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))	//û���ڷ��ű����ҵ�������
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address); //����callĿ����룬���иù���
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);//����ָ����м���then��do���
		conditions_or(set);	/*9.30�޸�*/
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}	
		cx1 = cx;	//���µ�ǰ�������ָ��λ��
		gen(JPC, 0, 0);	//������תָ���תλ����ʱ��0����������������д
		statement(fsys);//����then�����
		code[cx1].a = cx;	// ��һ��ָ��(cx1��ָ��)����תλ��ӦΪ��ǰcx��ָλ��
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);//����ָ����м��� end��;
		statement(set);//��begin��end֮��������д���
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys)) //���������һ��������ֺŻ���俪ʼ����
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);//�������
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;	//���µ�ǰ�������λ�ã�����whileѭ���Ŀ�ʼλ��
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);//����ָ����м���do���
		conditions_or(set);//���߼����ʽ���з�������
		destroyset(set1);
		destroyset(set);
		cx2 = cx;	//���µ�ǰ�������λ�ã�����while��do�е����Ŀ�ʼλ��
		gen(JPC, 0, 0);
		if (sym == SYM_DO)//����do
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);//����do������
		gen(JMP, 0, cx1);//ѭ����ת��cx1λ�ã����ٴν����߼��ж�
		code[cx2].a = cx;//�Ѹղ���0����תλ�øĳɵ�ǰλ�ã����while���Ĵ���
	}
	test(fsys, phi, 19); //err_msg[19],"Incorrect symbol.",
	// ����һ����䴦����ɣ�һ��������fsys���еķ��ţ����û������������19�Ŵ�
} // statement
			
//////////////////////////////////////////////////////////////////////
/*
������void block(symset fsys)
���ܣ������������
*/
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;
	/*
	��ַָʾ������ÿ��ֲ�����ǰ�ѷ��䵽�����λ�á�      
	�ó�ʼֵΪ3��ԭ���ǣ�ÿһ���ʼ��λ���������ռ�ֱ����ڴ�ž�̬��SL����̬��DL�ͷ��ص�ַRA
	*/
	dx = 3;	// data allocation index
	block_dx = dx;
	mk = (mask*) &table[tx];	//��table[tx]�ĵ�ַǿ��ת����mask*���� ����ֵ��mk
	mk->address = cx;			//index of current instruction to be generated
	//���ű���µ�ǰ�����Ŀ�ʼλ��
	gen(JMP, 0, 0);	//����һ����תָ���תλ����ʱ��0
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do //��ʼѭ������Դ���������е���������
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do //��ʼ����Դ�����еĳ�����������
			{
				constdeclaration();		//�����Ե�ǰtokenΪ��ʶ���ĳ���
				while (sym == SYM_COMMA)	//��������˶����򷴸�������һ������
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON) //';'��Ϊ��һ��ѭ����׼��
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);//����ǰ������ʾΪ��ʶ�� ѭ��ִ��
		} // if

		if (sym == SYM_VAR)	//����
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();	//�����Ե�ǰtokenΪ��ʶ���ı���
				while (sym == SYM_COMMA)	//��������˶����򷴸�������һ������
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON) //';'��Ϊ��һ��ѭ����׼��
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);//����ǰ������ʾΪ��ʶ�� ѭ��ִ��
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)	//ѭ���������ӹ���
		{ // procedure declarations
			getsym();//��ȡ��һ��token���˴�����ӦΪ��Ϊ�������ı�ʶ��
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);//��������̵�¼�����ֱ���
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}
			if (sym == SYM_SEMICOLON)//��������';' 
			{
				getsym();// ��ȡ��һ��token��׼�������﷨�����ĵݹ����
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;//Ƕ�����+1
			savedTx = tx;//�ݴ�tx��ֵ����ʼ���ű�ָ��ָ��ǰ��ķ����ڷ��ű��еĿ�ʼλ��
			set1 = createset(SYM_SEMICOLON, SYM_NULL);//�������� ����';'����
			set = uniteset(set1, fsys); 
			block(set);	//�ݹ����block
			destroyset(set1);
			destroyset(set);
			tx = savedTx;//�ָ�txֵ
			level--;	//Ƕ�����-1

			if (sym == SYM_SEMICOLON)//��������';' 
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);	//setΪ�Ϸ��ĺ�̼���
				test(set, fsys, 6);//��鵱ǰtoken�Ƿ�Ϸ������Ϸ�����fsys�ָ��﷨����ͬʱ��6�Ŵ�"Incorrect procedure name.",
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);//�����洢��ʶ��������
		set = uniteset(statbegsys, set1);  //��������ʼ����������ָ�
		test(set, declbegsys, 7);////��鵱ǰ״̬�Ƿ�Ϸ�����7�Ŵ�"Statement expected."
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));//ֱ�������Ե�Դ���������ϣ���������ִ�У�����������

	code[mk->address].a = cx;	//��ǰ�����ɵ���ת������תλ�øĳɵ�ǰλ��
	mk->address = cx;		//��ַΪ��ǰ��������ַ
	cx0 = cx;		//���µ�ǰ�������λ��
	gen(INT, 0, block_dx);	////���ɷ���ռ�ָ�����dx���ռ�
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);//�������нڵ�";"��"END"������
	set = uniteset(set1, fsys);	
	statement(set);//����ǰ��������������
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);//����listcode���� �����cx0��cx֮��Ĵ���
} // block

//////////////////////////////////////////////////////////////////////
/*
������
*/
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];	//�õ�ǰ����������ַ�е�������Ϊ�µĵ�ǰ�㣬����������һ��
	return b;
} // base

//////////////////////////////////////////////////////////////////////
/*
������void interpret()
���ܣ�Ŀ�����������й���
*/
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			/***9.30��ӡ�������*/
			case OPR_AND:
				top--;
				stack[top] = stack[top] && stack[top + 1];  
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top] || stack[top + 1];
				break;
			case OPR_NOT:
				stack[top] = !stack[top];
				break;
			
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;	// length of identifiers

	getsym();	//���ú���getsym��������ַ���symbol

	set1 = createset(SYM_PERIOD, SYM_NULL); //��'.'������б�
	set2 = uniteset(declbegsys, statbegsys); //declbegsys��statbegsys�ļ���
	set = uniteset(set1, set2);
	block(set);		//���ó����������
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
