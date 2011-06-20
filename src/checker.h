#ifndef _CHECKER_H
#define _CHECKER_H

//Kconv0.5 Checker
//���������ɤ�Ƚ�̤�Ԥʤ�
#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif
#ifndef HAVE_LIBGCC
// #include "opnew.h"
#endif
#include "commonop.h"
#include "Python.h"

extern Codes _DEFAULT_INPUT_CODING;

extern char prefix[MAXPATHLEN]; //�ơ��֥�ξ��򼨤���

extern const char *tc2tableefile;//filename
extern const char *tc2tablesfile;//filename
extern unsigned char *u2etable;//unicode -> euc convert table

class Checker:public CommonOp{
  //Checker interface & some methods
 public:
  virtual const Codes ChkCoding(const unsigned char * input_string,unsigned int length)=0;
  Checker(char * n,ChkMode c):CommonOp(n,c){
  };
 protected:
  inline bool isSjis1(const unsigned char in){
	if((0x81 <= in) && (in <= 0x9F))
	  return(true);
	return(false);
  }
  
  inline bool isSjis2(const unsigned char in){
	if(in <= 0x7E)
	  return(true);
	if((0x80 <= in) && (in <= 0xA0))
	  return(true);
	return(false);
  }
  
  inline bool isEuc1(const unsigned char in){
	if((0xF0 <= in) && (in <= 0xFE))
	  return(true);
	return(false);
  }
  
  inline bool isEuc2(const unsigned char in){
	if((0xFD <= in) && (in <= 0xFE))
	  return(true);
	return(false);
  }
  
  inline bool isJisInOut(const unsigned char *pin){
	unsigned char ESC = 0x1B;
	if(!(*pin++ == ESC))
	  return(false);
	if(*pin == '$'){
	  pin++;
	  if(*pin == 'B')
		return(true);
	  if(*pin == '@')
		return(true);
	  return(false);
	}
	if(*pin == '('){
	  pin++;
	  if(*pin == 'J')
		return(true);
	  if(*pin == 'H')
		return(true);
	  if(*pin == 'B')
		return(true);
	  if(*pin == 'I')
		return(true);
	  return(false);
	}
	return(false);
  }
  const Codes ChkAnycode5(unsigned char *pin,double *codings);
};

class FastChecker : public Checker{
  //Fast check coding �ǽ��Ƚ�̲�ǽʸ���Ƿ���
 public:
  const Codes ChkCoding(const unsigned char *,unsigned int length);
  const Codes ChkCoding2(const unsigned char *,unsigned int length);
  FastChecker():Checker("FAST",_FAST){
  };
};

class FullChecker: public Checker{
  //Full check coding �����򸫤ƺǤ�Ƚ�̤Ǥ��������ɤˤ���
 public:
  const Codes ChkCoding(const unsigned char *,unsigned int length);
  FullChecker():Checker("FULL",_FULL){
  };
};

//Table Checker�Ѥ����
//start : SJIS/EUCȽ�̥ơ��֥�κǽ����
//end : SJIS/EUCȽ�̥ơ��֥�κǸ����

const int start = 0x80;
const int end = 0xFE;
extern double tables[];
extern double tablee[];

extern int TableChecker_hoehoe;

class TableChecker:public Checker{
  //Table check coding ����ɽ���Ѥ���EUC/SJIS��Ƚ���Ԥʤ�
 public:
  const Codes ChkCoding(const unsigned char *,unsigned int length);
  TableChecker():Checker("TABLE",_TABLE){
	if(TableChecker_hoehoe == 0){
	  TableChecker_hoehoe = 1;
	  int A = 200;//Ƚ�̲�ǽʸ������Ψ
	  int A2 = 2; //�ǥե���Ȥ�ʸ�������ɤ��Ф�����Ψ
	  for(int i = start ; i <= end ; i++){
		if(tablee[i - start] == 0){
		  tables[i - start] = tables[i - start] * A;
		  tablee[i - start] = -10;
		}
		if(tables[i - start] == 0){
		  tablee[i - start] = tablee[i - start] * A;
		  tables[i - start] = -10;
		}
		if(_DEFAULT_INPUT_CODING == _EUC)
		  tablee[i - start] = tablee[i - start] * A2;
		if(_DEFAULT_INPUT_CODING == _SJIS)
		  tables[i - start] = tables[i - start] * A2;
	  }
	}
  };
};

class Table2Checker:public Checker{
  //Table check coding ����ɽ���Ѥ���EUC/SJIS��Ƚ���Ԥʤ�
 public:
  const Codes ChkCoding(const unsigned char *,unsigned int length);
  Table2Checker();
};

extern unsigned int *tablee2;
extern unsigned int *tables2;
  

extern "C"{
//���饹���󥹥��󥹤��֤��ؿ�
Checker * newFastChecker(void);
Checker * newFullChecker(void);
Checker * newTableChecker(void);
Checker * newTable2Checker(void);
};

#endif
