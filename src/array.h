#ifndef ARRAY_H
#define ARRAY_H

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif

#if defined STDC_HEADERS || defined WIN32
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
#else
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
# endif
#endif
#ifndef HAVE_LIBGCC
// #include "opnew.h"
#endif
#ifdef WIN32
# define NULL 0
#endif


class array{
private:
  class list{
  public:
	list * next;
	unsigned long int length;
	unsigned char * pt;
	unsigned char * base;
	list(unsigned long size);
	~list();
	bool lappend(const unsigned char in);
	bool isEmpty(void);
  };
  unsigned char * base; //getvalue()で返す文字列ためのポインタ
  list * start; //最初のlistへのポインタ
  list * pos; //現在のlistへのポインタ
  unsigned int length;
  void print(unsigned char* ppp);
 public:
  array(unsigned long l=100);
  ~array();
  void append(const unsigned char in);
  void append(const unsigned char *pin);
  void append(const unsigned char *pin,unsigned int len);
  void append(const char *pin);
  unsigned int getlength(void);
  const unsigned char * getvalue(void);
  void rewind(void);
};

  
#endif
