#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define IDN 0
#define IF 1
#define THEN 2
#define ELSE 3
#define WHILE 4
#define DO 5
#define INT10 6
#define INT8 7
#define INT16 8
#define ADD 9
#define SUB 10
#define MUL 11
#define DIV 12
#define EQ 13
#define GT 14
#define LT 15
#define LP 16
#define RP 17
#define SEMI 18
#define ASG 19
#define CodeSize 500
#define BufSize 200
#define Snext 0

char *newTemp();
char resourceProject[10000];
int  newLabel();
int scan(char ss[]);
int IsKeyword(char ss[]);
int LookUp(char ss[]);
int to10(char s[], int n);
void match(int sym);
int proc_S(struct S_Attr *S);
int proc_C(struct C_Attr *C);
int proc_E(struct E_Attr *E);
int proc_T(struct T_Attr *T);
int proc_F(struct F_Attr *F);

static struct {
	char *KeyName;
	int type;
}Keywords[5] = { { "if",IF },{ "then",THEN },{ "else",ELSE },{ "while",WHILE },{ "do",DO } };

struct F_Attr {
	char code[CodeSize];
	char place[BufSize];
};
struct T_Attr {
	char code[CodeSize];
	char place[BufSize];
};
struct E_Attr {
	char code[CodeSize];
	char place[BufSize];
};
struct C_Attr {
	char code[CodeSize];
	int  afalse;
	int  atrue;
};
struct S_Attr {
	char code[CodeSize];
	int  begin;
	int  next;
};
struct IDN_Attr {
	char idname[BufSize];
	int  entry;
};
struct IDN_Attr IDNList[100];

int lookhead;
char token[1024];
char str[1024];
char attr[1024];
int sta;
int tempnum = 1, labelnum = 0, Idnum = 0;

char *newTemp() {
	char p[100] = "";
	char s[100] = "t";
	itoa(++tempnum, p, 10);
	return strcat(s, p);
}

int  newLabel() {
	return ++labelnum;
}

int proc_S(struct S_Attr *S) {
	int tempsta;
	struct E_Attr E;
	struct C_Attr C;
	struct C_Attr C1;
	struct S_Attr S1;
	struct S_Attr S2;
	struct S_Attr S3;
	char tempidn[BufSize];
	switch (lookhead) {
	case IDN:						///S-> id=E
		match(IDN);
		strcpy(tempidn, attr);
		match(EQ);
		proc_E(&E);
		if (strlen(E.code) == 0)
		{
			sprintf(S->code, "%s = %s", tempidn, E.place);
		} 
		else
		{
			sprintf(S->code, "%s\n\t%s = %s", E.code, tempidn, E.place);
		}
		

		return 1;
	case IF:
		tempsta = sta;
		match(IF);
		C.atrue = newLabel();
		S1.begin = C.atrue;
		S1.next = C.afalse = S->next;
		proc_C(&C);
		match(THEN);
		proc_S(&S1);
		if (lookhead != ELSE) {////////////////////////////               S->IF C THEN S1
			sprintf(S->code, "%s\nL%d:\t%s", C.code, C.atrue, S1.code);
			return 1;
		}
		/////////////////////////////////////////////////   S-> IF C THEN S2 ELSE S3
		sta = tempsta;

		lookhead = IF;
		match(IF);
		C1.atrue = newLabel();
		C1.afalse = newLabel();
		S2.next = S3.next = S->next;
		proc_C(&C1);
		match(THEN);
		proc_S(&S2);
		match(ELSE);
		proc_S(&S3);
		sprintf(S->code, "%s\nL%d:\t%s\nL%d:\t%s", C.code, C.atrue, S2.code, C.afalse, S3.code);
		return 1;
	case WHILE://////////////////// S-> WHILE C DO S1
		match(WHILE);
		S->begin = S1.next = newLabel();
		C.atrue = S1.begin = newLabel();
		C.afalse = S->next;
		proc_C(&C);
		match(DO);
		proc_S(&S1);
		sprintf(S->code, "L%d:\t%s\nL%d:\t%s\n\tgoto L%d", S->begin, C.code, C.atrue, S1.code, S->begin);
		return 1;
	default:
		printf("\nS match error~!");
		exit(0);
	}
	return 1;
}

int proc_C(struct C_Attr *C) {
	struct E_Attr E1;
	struct E_Attr E2;
	proc_E(&E1);
	switch (lookhead) {
	case GT:///////////C->E1>E2
		match(GT);
		proc_E(&E2);
		if (strlen(E1.code) == 0&& strlen( E2.code ) ==0)
		{
			sprintf(C->code, "if %s > %s goto L%d\n\tgoto L%d", E1.place, E2.place, C->atrue, C->afalse);
		} 
		else
		{
			sprintf(C->code, "%s%s\n\tif %s > %s goto L%d\n\tgoto L%d", E1.code, E2.code, E1.place, E2.place, C->atrue, C->afalse);
		}
		return 1;
	case LT:///////////C->E1<E2
		match(LT);
		proc_E(&E2);
		if (strlen(E1.code) == 0 && strlen(E2.code) == 0)
		{
			sprintf(C->code, "if %s < %s goto L%d\n\tgoto L%d", E1.place, E2.place, C->atrue, C->afalse);
		}
		else
		{
			sprintf(C->code, "%s%s\n\tif %s < %s goto L%d\n\tgoto L%d", E1.code, E2.code, E1.place, E2.place, C->atrue, C->afalse);
		}
		return 1;
	case EQ:///////////C->E1=E2
		match(EQ);
		proc_E(&E2);
		if (strlen(E1.code) == 0 && strlen(E2.code) == 0)
		{
			sprintf(C->code, "if %s = %s goto L%d\n\tgoto L%d", E1.place, E2.place, C->atrue, C->afalse);
		}
		else
		{
			sprintf(C->code, "%s%s\n\tif %s = %s goto L%d\n\tgoto L%d", E1.code, E2.code, E1.place, E2.place, C->atrue, C->afalse);
		}
		return 1;
	default:
		printf("\nmatch error~!");
		exit(0);
	}
	return 1;
}

int proc_E(struct E_Attr *E) {
	struct T_Attr T1;
	struct T_Attr T2;
	proc_T(&T1);
	if ((lookhead != ADD) && (lookhead != SUB)) {/////////////////E->T
		strcpy(E->place, T1.place);
		sprintf(E->code, "%s", T1.code);
		return 1;
	}
	while ((lookhead == ADD) || (lookhead == SUB)) {////////////E->T((+|-)T)*
		if (lookhead == ADD) {
			match(ADD);
			proc_T(&T2);
			strcpy(E->place, newTemp());
			if (strlen(T1.code) == 0 && strlen(T2.code) ==0 )
			{
				sprintf(E->code, "%s = %s + %s", E->place, T1.place, T2.place);
			}
			else {
				sprintf(E->code, "%s%s\n\t%s = %s + %s", T1.code, T2.code, E->place, T1.place, T2.place);
			}
			strcpy(T1.code, E->code);
			strcpy(T1.place, E->place);
		}
		else {
			match(SUB);
			proc_T(&T2);
			strcpy(E->place, newTemp());
			if (strlen(T1.code) == 0 && strlen(T2.code)== 0)
			{
				sprintf(E->code, "%s = %s - %s", E->place, T1.place, T2.place);
			}
			else {
				sprintf(E->code, "%s%s\n\t%s = %s - %s", T1.code, T2.code, E->place, T1.place, T2.place);
			}
			
			strcpy(T1.code, E->code);
			strcpy(T1.place, E->place);
		}
	}
	return 1;
}

int proc_T(struct T_Attr *T) {
	struct F_Attr F1;
	struct F_Attr F2;
	proc_F(&F1);
	if ((lookhead != MUL) && (lookhead != DIV)) {/////////////T->F
		strcpy(T->place, F1.place);
		sprintf(T->code, "%s", F1.code);
		return 1;
	}
	while ((lookhead == MUL) || (lookhead == DIV)) {/////////////T->F((*|/)F)*
		if (lookhead == MUL) {
			match(MUL);
			proc_F(&F2);
			strcpy(T->place, newTemp());
			if (strlen(F1.code)== 0&&strlen( F2.code)==0)
			{
				sprintf(T->code, "%s = %s * %s",  T->place, F1.place, F2.place);
			} 
			else
			{
				sprintf(T->code, "%s%s\n\t%s = %s * %s", F1.code, F2.code, T->place, F1.place, F2.place);
			}
			strcpy(F1.code, T->code);
			strcpy(F1.place, T->place);
		}
		else {
			match(DIV);
			proc_F(&F2);
			strcpy(T->place, newTemp());
			if (strlen(F1.code) == 0 && strlen(F2.code) == 0)
			{
				sprintf(T->code, "%s = %s / %s", T->place, F1.place, F2.place);
			} 
			else
			{
				sprintf(T->code, "%s%s\n\t%s = %s / %s", F1.code, F2.code, T->place, F1.place, F2.place);
			}
			strcpy(F1.code, T->code);
			strcpy(F1.place, T->place);
		}
	}
	return 1;
}

int proc_F(struct F_Attr *F) {
	struct E_Attr E;
	if ((lookhead == INT8) || (lookhead == INT10) || (lookhead == INT16) || (lookhead == IDN)) {
		strcpy(F->place, attr);
		sprintf(F->code, "\0");
		switch (lookhead) {//////////////F->id|INT8|INT10|INT16
		case INT8:
			match(INT8); return 1;
		case INT10:
			match(INT10); return 1;
		case INT16:
			match(INT16); return 1;
		case IDN:
			match(IDN);	return 1;
		default:
			return 1;
		}
	}
	else if (lookhead == LP) {///////////////E->(E)
		match(LP);
		proc_E(&E);
		strcpy(F->place, E.place);
		sprintf(F->code, "%s", E.code);
		match(RP);
		return 1;
	}
	else {
		printf("\nF match error~!");
		exit(0);
	}
	return 1;
}

void match(int sym) {
	memset(token, 0, 32);
	if (lookhead == sym)		lookhead = scan(str);
	else {
		printf("\n  match error");
		exit(0);
	}
	return;
}

int scan(char ss[]) {
	int i = 0;
	int j = 0;
	while (ss[sta] == ' ')	sta++;
	for (; ss[sta] != '\0'; sta++) {
		if (isdigit(ss[sta])) {
			if ((ss[sta] - '0') == 0) {
				if ((ss[sta + 1]) == 'x') {
					token[i++] = ss[sta++];
					token[i++] = ss[sta++];
					if (!(isdigit(ss[sta]))&(ss[sta]<'a' || ss[sta]>'f')) {
						sta--;
						token[0] = '0';
						strcpy(attr, "0");
						return INT10;   //digit
					}
					else {
						while (isdigit(ss[sta]) || (ss[sta] >= 'a'&&ss[sta] <= 'f'))
							token[i++] = ss[sta++];
						itoa(to10(token, 16), attr, 10);
						return INT16;
					}
				}
				else {
					if ((ss[sta + 1] >= '0')&(ss[sta + 1] <= '7')) {
						token[i++] = ss[sta++];
						while (isdigit(ss[sta]))	token[i++] = ss[sta++];
						itoa(to10(token, 8), attr, 10);
						return INT8;
					}
					else {
						token[0] = '0';
						sta++;
						itoa(to10(token, 10), attr, 10);
						return INT10;
					}
				}
			}
			else {
				token[i++] = ss[sta++];
				while (isdigit(ss[sta]))	token[i++] = ss[sta++];
				itoa(to10(token, 10), attr, 10);
				return INT10;
			}
		}
		if (isalpha(ss[sta])) {
			token[i++] = ss[sta++];
			while (isalpha(ss[sta]) || isdigit(ss[sta]))	token[i++] = ss[sta++];
			switch (IsKeyword(token)) {
			case 0:	return IF;
			case 1: 	return THEN;
			case 2:	return ELSE;
			case 3:	return WHILE;
			case 4:	return DO;
			case -1:
				if ((j = LookUp(token)) >= 0) {
					strcpy(attr, IDNList[j].idname);
					return IDN;
				}
			default:
				printf("\nInput Error~!"); 	break;
			}
		}
		if (ss[sta] == '+') { sta++;	return ADD; }
		if (ss[sta] == '-') { sta++;	return SUB; }
		if (ss[sta] == '*') { sta++;	return MUL; }
		if (ss[sta] == '/') { sta++;	return DIV; }
		if (ss[sta] == '=') { sta++;	return EQ; }
		if (ss[sta] == '>') { sta++;	return GT; }
		if (ss[sta] == '<') { sta++;	return LT; }
		if (ss[sta] == '(') { sta++;	return LP; }
		if (ss[sta] == ')') { sta++;	return RP; }
		if (ss[sta] == ';') { sta++;	return SEMI; }
		if (ss[sta] == ':') {
			if (ss[sta + 1] == '=') { sta++; return ASG; }
			else	printf("\nInput Error~!");
		}
	}
	return 111;
}

int IsKeyword(char ss[]) {
	int i;
	for (i = 0; i < 5; i++)
		if (!strcmp(ss, Keywords[i].KeyName))		return i;
	return -1;
}

int LookUp(char ss[]) {
	int i = 0;
	for (i = 0; i <= Idnum; i++)
		if (!strcmp(IDNList[i].idname, ss))		return i;
	strcpy(IDNList[Idnum++].idname, ss);
	return Idnum - 1;
}
int to10(char s[], int n) {
	int i, no = 0;
	switch (n) {
	case 10:for (i = 0; s[i] != '\0'; i++)	no = no * 10 + (s[i] - '0');
		return no;
	case 8:	for (i = 0; s[i] != '\0'; i++)	no = no * 8 + (s[i] - '0');
		return no;
	case 16:for (i = 2; s[i] != '\0'; i++) {
		if (isdigit(s[i])) no = no * 16 + (s[i] - '0');
		else no = no * 16 + (s[i] - 'a' + 10);
	}
			return no;
	default:	break;
	}
	return 0;
}

int main() {
	char resourceProject[10000];
	int sta = 0;//源程序指针
	FILE *fp, *fp1;
	if ((fp = fopen("test.txt", "r")) == NULL)
	{//打开源程序
		cout << "can't open this file";
		exit(0);
	}
	resourceProject[sta] = fgetc(fp);
	while (resourceProject[sta] != '$')
	{//将源程序读入resourceProject[]
		sta++;
		resourceProject[sta] = fgetc(fp);
	}
	resourceProject[++sta] = '\0';
	fclose(fp);
	cout << endl << "The source code input by the user is:" << endl;
	cout << resourceProject << endl;
	strcpy(str, resourceProject);
	sta = 0;
	struct S_Attr S;
	//printf("Input the string:\n");
	//gets(str);
	//   strcpy(str,"if a=1 then x=1+2+3");
	//    strcpy(str,"while(a3+15)>0xa do if x2=07 then while y<z do y=x*y/z");
	//    printf("while(a3+15)>0xa do if x2=07 then while y<z do y=x*y/z");
	//strcpy(str,"if a<1 then a=1 else a=0");
	//strcpy(str,"while a<1 do a=1");
	//strcpy(str,"if x2=07 then a=1");
	//    strcpy(str,"a=3");
	lookhead = scan(str);
	S.next = Snext;
	proc_S(&S);
	fp1 = fopen("out.txt", "w");
	fprintf(fp1, S.code);
	string last = str;
	
	size_t indx = last.find("print");
	if (indx != string::npos) {
		last = last.substr(indx, last.size() - indx - 1);
		last = string("\n") + last;
		fprintf(fp1,last.c_str());
	}
	fclose(fp1);
	printf("%s", S.code);
	if (indx != string::npos) {
		printf("%s",last.c_str());
	}
	return 0;
}
