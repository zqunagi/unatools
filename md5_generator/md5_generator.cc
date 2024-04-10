#include "md5.h"

#include <stdio.h>

bool md5_file(const char *file, char checksum[33])
{
	MD5 md5;
	FILE *fp;

	if ((fp = fopen(file, "rb")) == NULL)
	{
		ErrorMessage("file %s is NULL", file);
		return false;
	}

	md5.update(fp);
	md5.finalize();
	md5.hex_digest(checksum);

	return true;
}

int main(char *argc, int argv)
{
	return 0;
}