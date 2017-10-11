// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pl0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message. 输出各种错误信息
/*
	打印数组下标为n的err_msg的信息
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
函数：void getch（void） 读取一个字符并返回给ch，line数组中用空格代替‘\n’
1. 获取单个字符的过程
2. 识别且跳过换行符
3. 将输入源文件复写到输出文件
4. 产生一份程序列表，输出其相应行号或者指令计数器的值
*/

void getch(void)
{
	if (cc == ll)		//cc: character count \ ll: line length
	{
		if (feof(infile))		//fefo : 文件结束，返回非0值 infile 为正在打开的文件
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx); //cx :index of current instruction to be generated
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))	//跳过换行符
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';	
	}
	ch = line[++cc];	//ch每次读一个字符
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
/*
函数：void getsym(void) 词法分析
*/
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];	//MAXIDLEN : length of identifiers

	while (ch == ' '||ch == '\t')	//跳过空格和制表符
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
		i = NRW;		//NRW 为保留字的数目
		while (strcmp(id, word[i--]));		//word : 保留字集合 wsym : 保留字的类型
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
		if (ch == '=')	//表示赋值
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
		if (ch == '/')	//判断注释
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
		else  //除法符号
		{
			sym = ssym[4];
		}
		
	}
/************************9.19增加了下面这个else if语句块*****************************/
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
/****************************9.19 增加了下面的语句块 用来判断 ++ 和 -- *************************/
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
函数名：void gen(int x, int y, int z)
功能：	将x，y，z放入code数组
*/
void gen(int x, int y, int z)
{
	if (cx > CXMAX)	//当前生成代码的行号大于允许的最大代码行数
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
函数：void test(symset s1, symset s2, int n)
功能：
	tests if error occurs and skips all symbols that do not belongs to s1 or s2.
	测试是否出现错误 并跳过所有不属于s1和s2的符号，这是出错恢复过程
	1. 可允许的下一个符号集合S1，如果当前符号不在此集合中，当即得到一个错误号
	2. 另加的停止符号集合S2，有些符号的出现，虽然无疑是错的，但它们绝对不应该忽略而被跳过
	3. 整数n，表示有关错误的诊断号
*/ 
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))	
	{
		//s1不在sym中
		error(n);		//报错
		s = uniteset(s1, s2);
		while(! inset(sym, s)) //通过循环找到下一个合法的符号，以恢复语法分析工作
			getsym();
		destroyset(s);	//销毁s
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index
/*
函数：void enter(int kind)
功能：
	enter object(constant, variable or procedre) into table.
	向符号表添加新的标识符，并确定标识符的有关属性
*/

void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:		//常数的属性为常数值
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;	//table是comtab类型 , 遇到变量和过程会被强制类型转换为mask
		break;
	case ID_VARIABLE:		//变量的属性为层次和修正量组成的地址
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:		//过程的属性是过程的入口地址和层次
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
/*
函数名：int position(char* id)
功能：
	locates identifier in symbol table.
	查找id在table中的位置 ，并将位置返回，如果没找到，返回0
*/
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;		//tx为符号表的长度
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
/*
函数名：void constdeclaration()
功能：
	常量定义分析过程 
*/
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)	//当前为标识符
	{
		getsym();		//获取下一个token
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);		// const ident = number  识别完成，将当前id（常量）存入符号表
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
函数名：void vardeclaration(void)
功能：
	变量分析过程
*/
void vardeclaration(void)
{
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);		//var ident  识别完成，将当前id（变量）存入符号表
		getsym();
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
/*
函数：void listcode(int from, int to)
功能：列出从from到to的PL/0程序代码
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
函数：void factor(symset fsys)
功能：因子处理过程
*/
void factor(symset fsys)
{
	void expression(symset fsys);
	int i;
	symset set;
	/*
	开始因子处理前，先检查当前token是否在facbegsys集合中
	facbegsy为factor合法的开始符号集合
	*/
	test(facbegsys, fsys, 24); //err_msg[24]: The symbol can not be as the beginning of an expression.
	
	while (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)	//类型为标识符
		{
			if ((i = position(id)) == 0)	//查找符号表
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:		//标识符是常量
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:		//标识符是变量
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:		//标识符是过程
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)	//类型为数字
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)	//类型为'('
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys); //set为')'和fsys和null的集合
			expression(set);	//符合表达式
			destroyset(set);
			if (sym == SYM_RPAREN)//存在')'
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
		/***************9.30添加下面的非***************/
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
函数：void term(symset fsys)
功能：项处理过程
*/
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_MODULE,SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH || sym == SYM_MODULE) //* 和 /
	{
		mulop = sym;	//保存当前运算符
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
 /***********↓↓↓↓10.10*添加下面这一块,实现按位与或异或↓↓↓↓************/
void terms_bitsand(symset fsys)  //按位与
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
void terms_bitsxor(symset fsys)//按位异或
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
void terms_bitsor(symset fsys)//按位与
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
/****************↑↑↑↑10.10号添加↑↑↑↑************************/
//////////////////////////////////////////////////////////////////////
/*
函数：void expression(symset fsys)
功能：表达式处理过程
*/
void expression(symset fsys)
{
	int addop;
	symset set;

	//将fsys与新建的含有'+'，'-'的链表合并
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS,SYM_NULL));//10.10修改
	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;//把当前符号保存，并继续向下读取
		getsym();//获取下一个token
		term(set);//调用项处理过程term
		if (addop == SYM_PLUS)//保存下来的是'+'
		{
			gen(OPR, 0, OPR_ADD);//生成加指令
		}
		else
		{
			gen(OPR, 0, OPR_MIN);//生成减指令
		}
	} // while

	destroyset(set);
} // expression
//////////////////////////////////////////////////////////////////////
/*
函数：void condition(symset fsys)
功能：条件处理过程,参数fsys: 如果出错可用来恢复语法分析的符号集合
*/
void condition(symset fsys)
{
	int relop;//用来临时记录token内容
	symset set;

	if (sym == SYM_ODD)//如果是odd运算符
	{
		getsym();
		expression(fsys);//调用expression 进行处理运算
		gen(OPR, 0, OPR_ODD);//生成6号指令：奇偶判断
	}
	else
	{
		/*
		relset中存储逻辑运算符
		relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
		'=','<>','<','<=','>','>=',''
		*/
/***↓↓10.10修改如下代码，目的：让条件一般化，单个表达式也可以作为条件↓↓******/
		set = uniteset(relset, fsys);
		expression(set);	//对表达式左部进行处理运算
		destroyset(set);
/******↑↑↑↑10.10号添加↑↑↑↑************************/
		if(inset(sym, relset))
		{
			relop = sym;	//记录当前逻辑处理运算符
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
/****↓↓↓↓10.10*添加下面这一块↓↓↓↓保证按位与或异或优先级高于>=************/
void conditions_bitsand(symset fsys)  //按位与
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
void conditions_bitsxor(symset fsys)//按位异或
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
void conditions_bitsor(symset fsys)//按位与
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
/****************↑↑↑↑10.10号添加↑↑↑↑************************/
/*******************↓↓↓↓9.30号添加下面这一块↓↓↓↓************************/
void conditions_and(symset fsys)  //逻辑与
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

void conditions_or(symset fsys)//逻辑或
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
/****************↑↑↑↑9.30号添加↑↑↑↑************************/
//////////////////////////////////////////////////////////////////////
/*
函数：void statement(symset fsys）
功能：语句分析过程，参数fsys:如果出错可用来恢复语法分析的符号集合
*/
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER)//类型为标识符
	{ // variable assignment
		mask* mk;
		if (! (i = position(id)))//id不在符号表中  报错
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)	//如果在符号表中找到该标识符，但不是变量报错
		{
			error(12); // Illegal assignment.
			i = 0;	//i置0作为错误标志
		}
		getsym();
		if (sym == SYM_BECOMES)//当前token为 赋值
		{
			getsym();
		}
		else
		{
			error(13); // ':=' expected.
		}
		expression(fsys);//调用表达式处理过程expression
		mk = (mask*) &table[i];
		if (i)//如果不曾出错，i将不为0，i所指为当前标识符在符号表中的位置
		{
			gen(STO, level - mk->level, mk->address);//产生一行把表达式值写往指定内存的STO目标代码
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
			if (! (i = position(id)))	//没有在符号表中找到，报错
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address); //生成call目标代码，呼叫该过程
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
		set = uniteset(set1, fsys);//出错恢复集中加入then和do语句
		conditions_or(set);	/*9.30修改*/
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
		cx1 = cx;	//记下当前代码分配指针位置
		gen(JPC, 0, 0);	//生成跳转指令，跳转位置暂时填0，分析完语句后再填写
		statement(fsys);//分析then后语句
		code[cx1].a = cx;	// 上一行指令(cx1所指的)的跳转位置应为当前cx所指位置
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);//出错恢复集中加入 end和;
		statement(set);//对begin和end之间的语句进行处理
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys)) //如果分析完一句后遇到分号或语句开始符号
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);//分析语句
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
		cx1 = cx;	//记下当前代码分配位置，这是while循环的开始位置
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);//出错恢复集中加入do语句
		conditions_or(set);//对逻辑表达式进行分析计算
		destroyset(set1);
		destroyset(set);
		cx2 = cx;	//记下当前代码分配位置，这是while的do中的语句的开始位置
		gen(JPC, 0, 0);
		if (sym == SYM_DO)//遇到do
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);//分析do后的语句
		gen(JMP, 0, cx1);//循环跳转到cx1位置，即再次进行逻辑判断
		code[cx2].a = cx;//把刚才填0的跳转位置改成当前位置，完成while语句的处理
	}
	test(fsys, phi, 19); //err_msg[19],"Incorrect symbol.",
	// 至此一个语句处理完成，一定会遇到fsys集中的符号，如果没有遇到，就抛19号错
} // statement
			
//////////////////////////////////////////////////////////////////////
/*
函数：void block(symset fsys)
功能：程序分析过程
*/
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;
	/*
	地址指示器给出每层局部量当前已分配到的相对位置。      
	置初始值为3的原因是：每一层最开始的位置有三个空间分别用于存放静态链SL、动态链DL和返回地址RA
	*/
	dx = 3;	// data allocation index
	block_dx = dx;
	mk = (mask*) &table[tx];	//将table[tx]的地址强制转化成mask*类型 并赋值给mk
	mk->address = cx;			//index of current instruction to be generated
	//符号表记下当前层代码的开始位置
	gen(JMP, 0, 0);	//产生一条跳转指令，跳转位置暂时填0
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do //开始循环处理源程序中所有的声明部分
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do //开始处理源程序中的常量声明部分
			{
				constdeclaration();		//声明以当前token为标识符的常量
				while (sym == SYM_COMMA)	//如果遇到了逗号则反复声明下一个常量
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON) //';'，为下一轮循环做准备
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);//若当前特征表示为标识符 循环执行
		} // if

		if (sym == SYM_VAR)	//变量
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();	//声明以当前token为标识符的变量
				while (sym == SYM_COMMA)	//如果遇到了逗号则反复声明下一个变量
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON) //';'，为下一轮循环做准备
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);//若当前特征表示为标识符 循环执行
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)	//循环声明各子过程
		{ // procedure declarations
			getsym();//获取下一个token，此处正常应为作为过程名的标识符
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);//把这个过程登录到名字表中
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}
			if (sym == SYM_SEMICOLON)//读到的是';' 
			{
				getsym();// 获取下一个token，准备进行语法分析的递归调用
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;//嵌套深度+1
			savedTx = tx;//暂存tx的值，初始符号表指针指向当前层的符号在符号表中的开始位置
			set1 = createset(SYM_SEMICOLON, SYM_NULL);//创建链表 并将';'插入
			set = uniteset(set1, fsys); 
			block(set);	//递归调用block
			destroyset(set1);
			destroyset(set);
			tx = savedTx;//恢复tx值
			level--;	//嵌套深度-1

			if (sym == SYM_SEMICOLON)//读到的是';' 
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);	//set为合法的后继集合
				test(set, fsys, 6);//检查当前token是否合法，不合法则用fsys恢复语法分析同时抛6号错，"Incorrect procedure name.",
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);//创建存储标识符的链表
		set = uniteset(statbegsys, set1);  //用声明开始符号作出错恢复
		test(set, declbegsys, 7);////检查当前状态是否合法，抛7号错"Statement expected."
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));//直到声明性的源程序分析完毕，继续向下执行，分析主程序

	code[mk->address].a = cx;	//把前面生成的跳转语句的跳转位置改成当前位置
	mk->address = cx;		//地址为当前代码分配地址
	cx0 = cx;		//记下当前代码分配位置
	gen(INT, 0, block_dx);	////生成分配空间指令，分配dx个空间
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);//创建含有节点";"和"END"的链表
	set = uniteset(set1, fsys);	
	statement(set);//处理当前遇到的语句或语句块
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);//调用listcode函数 输出从cx0到cx之间的代码
} // block

//////////////////////////////////////////////////////////////////////
/*
函数：
*/
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];	//用当前层数据区基址中的内容作为新的当前层，即向上找了一层
	return b;
} // base

//////////////////////////////////////////////////////////////////////
/*
函数：void interpret()
功能：目标代码解释运行过程
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
			/***9.30添加↓↓↓↓*/
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

	getsym();	//调用函数getsym获得输入字符的symbol

	set1 = createset(SYM_PERIOD, SYM_NULL); //将'.'插入进列表
	set2 = uniteset(declbegsys, statbegsys); //declbegsys和statbegsys的集合
	set = uniteset(set1, set2);
	block(set);		//调用程序分析过程
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
