#include "commonop.h"
#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif
#ifndef HAVE_LIBGCC
// #include "opnew.h"
#endif
#include "array.h"

extern char prefix[MAXPATHLEN]; //テーブルの場所を示す。
extern const char * kconvu2etable;//file name

//入力クラスの宣言
class Inputer: public CommonOp{
  //inputer interface & some methods
 public:
  virtual array * input(unsigned char * const input_string,unsigned int len)=0;
  Inputer(char * n,Codes c):CommonOp(n,c){
  };
};

class JisInputer :public Inputer{
  //JIS -> EUC
 protected:
  bool inJisMode;
  
  int isJisIn(unsigned char * const);
  int isJisOut(unsigned char * const);
 public:
  array * input(unsigned char * const input_string,unsigned int len);
  JisInputer():Inputer("JIS",_JIS),inJisMode(false){
  };
};

class EucInputer : public Inputer{
  //なにもしない(^^;
 public:
  array * input(unsigned char * const input_string,unsigned int len);
  EucInputer(void):Inputer("EUC",_EUC){
  };
};

class SjisInputer : public Inputer{
  inline bool isSjisLead(const unsigned char in){
	if((0x81 <= in) && (in <= 0x9F))
	  return true;
	if((0xE0 <= in) && (in <= 0xFC))
	  return true;
	return false;
  }
  
  inline bool isSjisTrail(const unsigned char in){
	if((0x40 <= in) && (in <= 0xFC))
	  return true;
	return false;
  }
 public:  
  array * input(unsigned char * const input_string,unsigned int len);
  SjisInputer(void):Inputer("SJIS",_SJIS){
  };
};

extern unsigned char * u2etable;

class UnicInputer : public Inputer{
 protected:
  int endian;
  virtual unsigned int ChkBL(unsigned char *input_string);
  virtual int ChkEndian(unsigned char *input_string);
  virtual unsigned int MakeCode(unsigned char *&pin);
 public:
  array * input(unsigned char * const input_string,unsigned int len);
  UnicInputer(int ue,char * n="UNICODE",Codes c=_UNIC);
};

class Utf8Inputer : public UnicInputer{
 protected:
  unsigned int ChkBL(unsigned char *input_string);
  int ChkEndian(unsigned char *input_string);
  unsigned int MakeCode(unsigned char *&pin);
 public:
  Utf8Inputer(void):UnicInputer(0,"UTF8",_UTF8){
  };
};

extern "C"{
  //クラスインスタンスを返す関数
  Inputer * newEucInputer(void);
  Inputer * newSjisInputer(void);
  Inputer * newJisInputer(void);
  Inputer * newUnicInputer(void);
  Inputer * newUnicInputerEndian(int endian);
  Inputer * newUtf8Inputer(void);
};
