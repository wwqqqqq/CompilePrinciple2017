#include <stdio.h>
#include <string.h>

int main(void){
	char str1[]="012348";
	char str2[]="abcx";
	char str3[]="567";
	char str4[]="defghij";
	strcpy(str3,str4);
	strcpy(str2,str1);
	printf("%s\n%s\n%s\n%s\n",str1,str2,str3,str4);
	return 0;
}
