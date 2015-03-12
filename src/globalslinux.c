#include "globalslinux.h"

#ifndef	WIN32
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

int ONE_MB = 1000000;
int MEM_WARNING_THRESHOLD = 20*1000000;
int Bytes;
char *pMem;

double GetRAMSizeMB()
{
	const double DEFAULT_RAM = 500;
	static double RAMMB = 0;
	char Buffer[1024];
        int n, fd;
	if (RAMMB != 0)
		return RAMMB;

	fd = open("/proc/meminfo", O_RDONLY);
	if (-1 == fd)
        {
		return DEFAULT_RAM;
        }
	n = read(fd, Buffer, sizeof(Buffer) - 1);
	pMem = strstr(Buffer, "MemTotal: ");
	close(fd);
	fd = -1;

	if (n <= 0)
        {
		return DEFAULT_RAM;
        }
	Buffer[n] = 0;
	if (0 == pMem)
        {
		return DEFAULT_RAM;
        }
	Bytes = atoi(pMem+9)*1000;
	return ((double) Bytes)/1e6;
}

#endif	
