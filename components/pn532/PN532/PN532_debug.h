#ifndef __DEBUG_H__
#define __DEBUG_H__

// #define DEBUG

#ifdef DEBUG
#include <cstdio>
#define DMSG(args...)       printf(args)
#define DMSG_STR(str)       printf("%s\n", str)
#define DMSG_HEX(num)       printf(" 0x%x", num);
#define DMSG_INT(num)       printf(" %d", num);
#else
#define DMSG(args...)
#define DMSG_STR(str)
#define DMSG_HEX(num)
#define DMSG_INT(num)
#endif

#endif
