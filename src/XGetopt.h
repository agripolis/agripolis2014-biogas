// XGetopt.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XGETOPT_H
#define XGETOPT_H
#include <tchar.h>
#include <stdio.h>
extern int optind, opterr;
extern char *optarg;

int getopt(int argc, char *argv[], char *optstring);

#endif //XGETOPT_H
