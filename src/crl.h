#ifndef __CRL_H
#define __CRL_H

# include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif
#if defined STDC_HEADERS | defined WIN32
# include <string.h>
# include <stdlib.h>
#else
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

class rec{
 protected:
  int len;
 public:
  int regs[2];
  virtual int search(unsigned char * input_stirng,int startpos);
  rec(int length){
	len = length;
  };
};

class reu : public rec{
 public:
  reu(int length):rec(length){
  };
};

class reub : public reu{
 public:
  reub(int length):reu(length){
  };
  int search(unsigned char * input_string,int startpos);
};

class reul : public reu{
 public:
  reul(int length):reu(length){
  };
  int search(unsigned char * input_stirng,int startpos);
};

#define RETSTRING(start,end,next) \
{ \
	if(OutString) \
	  delete OutString; \
	olen = end - start; \
	OutString = new unsigned char[olen]; \
	memcpy(OutString,input_string+start,olen); \
	startpos = next; \
	return(OutString); \
}

class crl{
private:
  reu *REUC[2];
  rec *REC;
  unsigned char *input_string;
  unsigned int len;
  bool ReadLine_UnicodeFlag;
  unsigned int UnicodeChar_StartPos;
  int ReadLine_UnicodeEndian;
  unsigned char *OutString;
  unsigned char * ReadLineUnicode(int &startpos,unsigned int &olen);

  bool isBIG(unsigned char *pin){
	if((*pin == 0xFF) && (*(pin+1) == 0xFE))
	  return(true);
	return(false);
  };
  bool isLIT(unsigned char *pin){
	if((*pin == 0xFE) && (*(pin+1) == 0xFF))
	  return(true);
	return(false);
  };

public:

  //unsigned char *input_string : 入力文字列の先頭を指すポインタ
  //unsigned int len : input_stringの長さ
  crl(unsigned char *instring,unsigned int inlength){
	OutString = NULL;
	ReadLine_UnicodeFlag = false;
	UnicodeChar_StartPos = 0;
	ReadLine_UnicodeEndian = 0; // 0 -> Big-Endian , 1 -> Little->Endian
	input_string = instring;
	len = inlength;
	REC = new rec(len);
	REUC[0] = new reub(len);
	REUC[1] = new reul(len);
	return;
  };

  ~crl(void){
	if(OutString)
	  delete OutString;
	delete REC;
	delete REUC[0];
	delete REUC[1];
  };
		
  unsigned char * ReadLine(int &startpos,unsigned int &olen);
};

#endif
