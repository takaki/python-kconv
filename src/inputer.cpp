//Kconv0.5 Inputer
//入力文字列をEUCに変換
//半角仮名は0x8E+??で格納（2バイト半角仮名）
#include <stdio.h>
#include <string.h>
#include <Python.h>
#include "array.h"
#include "inputer.h"

int JisInputer::isJisIn(unsigned char * const pos){
  //Esc $B etc...
  unsigned char *cp = pos;
  if((*cp++ == ESC) && (*cp++ == '$')){
	if(*cp == '(')
	  cp++;
	if(*cp == '@')
	  return(int(cp - pos + 1));
	if(*cp == 'B')
	  return(int(cp - pos + 1));
	if(*cp == 'D')
	  return(int(cp - pos + 1));
	return 0;
  }
  return 0;
}
	
int JisInputer::isJisOut(unsigned char * const pos){
  //#Esc )B etc...
  unsigned char *cp = pos;
  if((*cp++ == ESC) && (*cp++ == '(')){
	if(*cp == 'J')
	  return(int(cp - pos + 1));
	if(*cp == 'H')
	  return(int(cp - pos + 1));
	if(*cp == 'B')
	  return(int(cp - pos + 1));
	if(*cp == 'I')
	  return(int(cp - pos + 1));
	return(0);
  }
  return(0);
};

array * JisInputer::input(unsigned char * const input_string,unsigned int len){
  array *out = new array();
  unsigned char *pt = input_string;
  inJisMode = false;
  unsigned char *epos = input_string + len;
  while(pt<epos){
	if(isAscii(*pt)){
	  if(inJisMode){
		if(isJisOut(pt)){
		  inJisMode = false;
		  pt += isJisOut(pt)-1;
		}else{
		  out->append(*pt|0x80);
		}
	  }else{
		if(isJisIn(pt)){
		  inJisMode = true;
		  pt += isJisIn(pt)-1;
		}else{
		  if((*pt == CR) && (*(pt+1) == LF)){ //改行コードはすべてLFにするの
			out->append(LF);
			pt++;
		  }else if((*pt == LF) || (*pt == CR))
			out->append(LF);
		  else
			out->append(*pt);
		}
	  }
	}else if(isHankana(*pt)){
	  out->append(0x8E);
	  out->append(*pt);
	}else{
	  //	  printf("BadInput");
	}
	pt++;
  }
  return(out);
}

array * EucInputer::input(unsigned char * const input_string,unsigned int len){
  array *out = new array();
  unsigned char *pin = input_string;
  unsigned char *epos = input_string + len;
  while(pin<epos){
	if((*pin == CR) && (*(pin+1) == LF)){ //改行コードはすべてLFにするの
	  out->append(LF);
	  pin++;
	}else if((*pin == LF) || (*pin == CR))
	  out->append(LF);
	else
	  out->append(*pin);
	pin++;
  }
  return(out);
}

array * SjisInputer::input(unsigned char * const input_string,unsigned int len){
  array *out = new array();
  unsigned char *pin = input_string;
  unsigned char *epos = input_string + len;
  while(pin<epos){
	if(isSjisLead(*pin)){
	  if(isSjisTrail(*(pin+1))){
		int leader = *pin++;
		int trailer = *pin;
		int leaderlow = 0;
		if(trailer < 0x9F){
		  leaderlow = 0;
		  if((0x80 <= trailer) && (trailer <= 0x9E))
			trailer -= 1;
		  trailer -= 0x1F;
		}else{
		  leaderlow = 1;
		  trailer -= 0x7E;
		}
		if((0x81 <= leader) && (leader <= 0x9F))
		  leader -= 0x81;
		else
		  leader -= 0xC1;
		leader = (leader << 1) + 0x21 + leaderlow;
		
		out->append((unsigned char)(leader|0x80));
		out->append((unsigned char)(trailer|0x80));
	  }
	}else if(isHankana(*pin)){
	  out->append((unsigned char)0x8E);
	  out->append((unsigned char)*pin);
	}else if(isAscii(*pin)){
	  if((*pin == CR) && (*(pin+1) == LF)){ //改行コードはすべてLFにするの
		out->append(LF);
		pin++;
	  }else if((*pin == LF) || (*pin == CR))
		out->append(LF);
	  else
		out->append(*pin);
	}
	pin++;
  }
  return(out);
}

unsigned char * u2etable;

unsigned int UnicInputer::ChkBL(unsigned char *input_string){
  if((input_string[1-endian] == NC) && // CR + LF
	 (input_string[endian]   == CR) &&
	 (input_string[3-endian] == NC) &&
	 (input_string[2+endian] == LF))
	return(4);
  if(input_string[1-endian] != NC)
	return(0);
  if((input_string[endian] == CR) || // CR or LF
	 (input_string[endian] == LF))
	return(2);
  return(0);
}

int UnicInputer::ChkEndian(unsigned char *input_string){
  if((input_string[0] == 0xFE) && (input_string[1] == 0xFF)){
	endian = 1; // Little Endian
	return(2);
  }
  if((input_string[0] == 0xFF) && (input_string[1] == 0xFE)){
	endian = 0; // Big Endian
	return(2);
  }
  return(0);
}

unsigned int UnicInputer::MakeCode(unsigned char *&pin){
  unsigned int ret = *pin << (8*endian);
  pin++;
  ret += *pin << (8*(1-endian));
  pin++;
  return(ret);
}

UnicInputer::UnicInputer(int ue,char * n,Codes c):Inputer(n,c),endian(ue){
  if(u2etable == NULL){
	//テーブルの読み込み
	char *filename;
	PyObject *ospath,*f_join,*file;
	ospath = PyImport_ImportModule("os.path");
	f_join = PyObject_GetAttrString(ospath,"join");
	file = PyObject_CallFunction(f_join,"(ssss)",SYSPREFIX,"share","python-kconv",kconvu2etable);
	filename = PyString_AsString(file);
	FILE *IN = fopen(filename,"rb");
	if(!IN){
	  fprintf(stderr,"%s\n", filename);
	  char *mes = new char[strlen(filename)+strlen(notfoundmessage)+2];
	  sprintf(mes,"%s(%s)",notfoundmessage,mes);
	  PyErr_SetString(PyExc_IOError,mes);
	}
	u2etable = new unsigned char[131072];
	fread(u2etable,sizeof(unsigned char),131072,IN);
	fclose(IN);
	Py_XDECREF(ospath);
	Py_XDECREF(f_join);
	Py_XDECREF(file);
  }
  return;
};

array * UnicInputer::input(unsigned char * const input_string,unsigned int length){
  array *out = new array();
  unsigned char *pin = input_string;
  unsigned char *epos = input_string + length;

  //変換開始
  while(pin<epos){
	int endiansize = 0;
	if((endiansize = ChkEndian(pin)) > 0){
	  pin += endiansize;
	  continue;
	}
	unsigned int bltmp = 0;
	if((bltmp = ChkBL(pin)) > 0){
	  out->append(LF);
	  pin += bltmp;
	  continue;
	}
	unsigned int code = MakeCode(pin);
	if(u2etable[code*2])
	  out->append(u2etable[code*2]);
	if(u2etable[code*2+1])
	  out->append(u2etable[code*2+1]);
  }
  return(out);
}

unsigned int Utf8Inputer::ChkBL(unsigned char *input_string){
  if((input_string[0] == CR) && // CR + LF
	 (input_string[1] == LF))
	return(2);
  if((input_string[0] == CR) || // CR or LF
	 (input_string[0] == LF))
	return(1);
  return(0);
}

int Utf8Inputer::ChkEndian(unsigned char *input_string){
  if((input_string[0] == 0xEF) && //Little Endian こんなコードが出現することがあるのだろうか？
	 (input_string[1] == 0xBB) && //仮に出現したとしても現段階でendianは未使用。
	 (input_string[2] == 0xBF)){
	endian = 1;
	return(3);
  }
  if((input_string[0] == 0xEF) && //Big Endian
	 (input_string[1] == 0xBF) &&
	 (input_string[2] == 0xBE)){
	endian = 0;
	return(3);
  }
  return(0);
}

unsigned int Utf8Inputer::MakeCode(unsigned char *&pin){
  unsigned int c1 = *pin;
  unsigned int code;
  if((c1 & 0x80) == 0)
	code = (unsigned char)c1;
  else{
	if((c1 & 0xE0) == 0xC0){
	  pin++;
	  unsigned int c2 = *pin;
	  code = ((c1 & 0x1F) << 6) |  (c2 & 0x3F);
	}else{
	  if((c1 & 0xF0) == 0xE0){
		pin++;
		unsigned int c2 = *pin;
		pin++;
		unsigned int c3 = *pin;
		code = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
	  }else{
		code= 0;
	  }
	}
  }
  pin++;
  return(code);
}

//クラスインスタンスを返す関数
Inputer * newEucInputer(void){
  return((Inputer *)new EucInputer());
};

Inputer * newSjisInputer(void){
  return((Inputer *)new SjisInputer());
};

Inputer * newJisInputer(void){
  return((Inputer *)new JisInputer());
};

Inputer * newUnicInputer(void){
  return((Inputer *)new UnicInputer(0));// Big-Endian を仮定
};

Inputer * newUnicInputerEndian(int endian){
  return((Inputer *)new UnicInputer(endian));
};

Inputer * newUtf8Inputer(void){
  return((Inputer *)new Utf8Inputer());
};

//スタンドアローンデバッグ用コード
//#define STAND_ALONE
#ifdef STAND_ALONE
#include <string.h>

int main(int argc,char **argv){
  FILE *fin;
  if((fin = fopen("licence.euc","r")) == NULL){
	printf("File open Error.\n");
	exit(255);
  }
  char *inputstring = new char[1024];
  fscanf(fin,"%s",inputstring);
  strcat(inputstring,"\n");
  Inputer *ip = new EucInputer();
  array *str = ip->input((unsigned char *)inputstring,strlen(inputstring));
  printf("%s",str->getvalue());
  delete str,ip;
  fclose(fin);
  return(0);
}
#endif
