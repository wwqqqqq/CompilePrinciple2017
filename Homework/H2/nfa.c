#include <stdio.h>

char * start;

void nfa(int state, char * str)
{
	switch(state)
	{
		case 0:
			if(*str=='\0') return;
			if(*str!='a'&&*str!='b')
			{	nfa(0,str+1); break; }
			nfa(1,str);
			nfa(3,str);
			break;
	    case 1:
			if(*str=='a') { start=str;  nfa(2,str+1);}
			break;
	    case 2:
			if(*str=='a') nfa(2,str+1);
			else
			{
				for(;start<str;start++)
					putchar(*start);
				printf("\n");
				nfa(0,str);
			}
			break;
	    case 3:
			if(*str=='b') { start=str;  nfa(4,str+1);}
			break;
	    case 4:
			if(*str=='b') nfa(4,str+1);
			else
			{
				for(;start<str;start++)
					putchar(*start);
				printf("\n");
				nfa(0,str);
			}
			break;
	   default:
			printf("Error!");
			break;
	}
}




int main(int argc,char **argv)
{
	if(argc!=2)
		printf("USAGE: ./nfa <the given string>\n");
	else nfa(0,argv[1]);
	return 0;
}
