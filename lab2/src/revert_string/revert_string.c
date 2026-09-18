#include "revert_string.h"
#include "stdlib.h"
#include "string.h"

void RevertString(char *str)
{
	int len = strlen(str);
	char* tempStr = malloc(sizeof(char) * (len + 1));
	for (int i = 0; i < len; i++) {
		*(tempStr + i) = *(str + (len - 1 - i)); 
	}
	strcpy(str, tempStr);
}

