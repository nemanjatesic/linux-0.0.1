#define __LIBRARY__
#include <unistd.h>

_syscall2(int,keyset,int,mode,char *,string);
_syscall1(int,encry,char *,string);
_syscall1(int,decry,char *,string);
_syscall2(int,zapocni,char *,string,int,mode);
_syscall1(long,time,long *,tloc);
_syscall1(int,keyclear,int,mode);