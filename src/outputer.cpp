//Kconv 0.5 Outputer
//EUC文字列をそれぞれのコードに変換
#include <stdio.h>
#include <string.h>
#include <Python.h>
#include "array.h"
#include "outputer.h"

const unsigned char * JisOutputer::hankanaconv(const unsigned char input_char,unsigned char *ret){
  //  unsigned char ret[2];
  ret[0] = input_char;
  ret[1] = (unsigned char)0;
  return(ret);
}

array * JisOutputer::output(array *input_string){
  array *out = new array();
  unsigned char *pin = (unsigned char *)input_string->getvalue();
  unsigned char *epos = pin + input_string->getlength();
  inJisMode = false;
  unsigned char ret[3];
  while(pin<epos){
	if(isAscii(*pin)){
	  if(inJisMode){
		out->append(ESC);
		out->append("(B");
		inJisMode = false;
	  }
	  if(*pin == LF){
		switch(BLCODE){
		case _LF:
		  out->append(LF);
		  break;
		case _CR:
		  out->append(CR);
		  break;
		case _CL:
		  out->append(CR);
		  out->append(LF);
		  break;
		}
	  }else
		out->append(*pin);
	}else if(isEuc(*pin)){
	  if(!inJisMode){
		out->append(ESC);
		out->append("$B");
		inJisMode = true;
	  }
	  out->append(*pin&0x7F);
	}else if(*pin == 0x8E){
	  pin++;
	  unsigned char test[3];
	  strcpy((char*)test,(const char*)hankanaconv(*pin,ret));
	  if(isHankana(test[0])){
		if(inJisMode){
		  out->append(ESC);
		  out->append("(B");
		  inJisMode = false;
		}
		out->append(test);
	  }else{
		if(!inJisMode){
		  out->append(ESC);
		  out->append("$B");
		  inJisMode = true;
		}
		out->append(test);
	  }
	}
	pin++;
  }
  if(inJisMode){
	out->append(ESC);
	out->append("(B");
  }
  return(out);
}

const unsigned char * JisZenkanaOutputer::hankanaconv(const unsigned char input_char,unsigned char *ret){
  //EUC -> JIS 全角仮名に変換
  //Table for kana -> JIS
  const unsigned int jisconvtable[63] = {
	0x2123,0x2156,0x2157,0x2122,0x2126,0x2572,0x2521,0x2523,
	0x2525,0x2527,0x2529,0x2563,0x2565,0x2567,0x2543,0x213c,
	0x2522,0x2524,0x2526,0x2528,0x252a,0x252b,0x252d,0x252f,
	0x2531,0x2533,0x2535,0x2537,0x2539,0x253b,0x253d,0x253f,
	0x2541,0x2544,0x2546,0x2548,0x254a,0x254b,0x254c,0x254d,
	0x254e,0x254f,0x2552,0x2555,0x2558,0x255b,0x255e,0x255f,
	0x2560,0x2561,0x2562,0x2564,0x2566,0x2568,0x2569,0x256a,
	0x256b,0x256c,0x256d,0x256f,0x2573,0x212b,0x212c};
  
  //  unsigned char ret[3];
  ret[0] = jisconvtable[input_char - 0xA1] >> 8;
  ret[1] = jisconvtable[input_char - 0xA1] & 0xFF;
  ret[2] = (unsigned char)0;
  return(ret);
}

array * EucOutputer::output(array *input_string){
  array *out = new array();
  out->rewind();
  unsigned char *pin = (unsigned char *)input_string->getvalue();
  unsigned char *epos = pin + input_string->getlength();
  while(pin<epos){
	if(*pin == LF){
	  switch(BLCODE){
	  case _LF:
		out->append(LF);
		break;
	  case _CR:
		out->append(CR);
		break;
	  case _CL:
		out->append(CR);
		out->append(LF);
		break;
	  }
	}else
	  out->append(*pin);
	pin++;
  }
  return(out);
}
  
//unsigned int eucconvtable;

array * EucZenkanaOutputer::output(array *input_string){
  //EUC -> EUC 全角仮名に変換
  //Table for kana -> EUC
  const unsigned int eucconvtable[63] = {
	0xa1a3,0xa1d6,0xa1d7,0xa1a2,0xa1a6,0xa5f2,0xa5a1,0xa5a3,
	0xa5a5,0xa5a7,0xa5a9,0xa5e3,0xa5e5,0xa5e7,0xa5c3,0xa1bc,
	0xa5a2,0xa5a4,0xa5a6,0xa5a8,0xa5aa,0xa5ab,0xa5ad,0xa5af,
	0xa5b1,0xa5b3,0xa5b5,0xa5b7,0xa5b9,0xa5bb,0xa5bd,0xa5bf,
	0xa5c1,0xa5c4,0xa5c6,0xa5c8,0xa5ca,0xa5cb,0xa5cc,0xa5cd,
	0xa5ce,0xa5cf,0xa5d2,0xa5d5,0xa5d8,0xa5db,0xa5de,0xa5df,
	0xa5e0,0xa5e1,0xa5e2,0xa5e4,0xa5e6,0xa5e8,0xa5e9,0xa5ea,
	0xa5eb,0xa5ec,0xa5ed,0xa5ef,0xa5f3,0xa1ab,0xa1ac};
  array *out = new array();
  unsigned char *pin = (unsigned char *)input_string->getvalue();
  unsigned char *epos = pin + input_string->getlength();
  while(pin<epos){
	if(*pin == 0x8E){
	  pin++;
	  out->append(eucconvtable[*pin - 0xA1] >> 8);
	  out->append(eucconvtable[*pin - 0xA1] & 0xFF);
	}else
	  out->append(*pin);
	pin++;
  }
  return(out);
}

const unsigned char * SjisOutputer::hankanaconv(const unsigned char input_char,unsigned char *ret){
  //  unsigned char ret[2];
  ret[0] = input_char;
  ret[1] = (unsigned char)0;
  return(ret);
}

array * SjisOutputer::output(array *input_string){
  array *out = new array();
  unsigned char *pin = (unsigned char *)input_string->getvalue();
  unsigned char *epos = pin + input_string->getlength();
  unsigned char ret[3];
  while(pin<epos){
	if(isAscii(*pin)){
	  if(*pin == LF){
		switch(BLCODE){
		case _LF:
		  out->append(LF);
		  break;
		case _CR:
		  out->append(CR);
		  break;
		case _CL:
		  out->append(CR);
		  out->append(LF);
		  break;
		}
	  }else{
		out->append(*pin);
	  }
	}else if(isEuc(*pin)){
	  unsigned char leader = *pin++ & 0x7F;
	  unsigned char trailer = *pin & 0x7F;
	  leader = leader - 0x21;
	  if(leader&0x1)
		trailer += 0x7E;
	  else{
		trailer += 0x1F;
		if((0x7F <= trailer) && (trailer <= 0x9D))
		  trailer++;
	  }
	  leader = (leader & 0x7E) >> 1;
	  if(leader <= 0x1E)
		leader += 0x81;
	  else
		leader += 0xC1;
	  out->append(leader);
	  out->append(trailer);
	}else if(*pin == 0x8E){
	  unsigned char test[3];
	  strcpy((char *)test,(const char*)hankanaconv(*++pin,ret));
	  out->append(test);
	}
	pin++;
  }
  return(out);
}

const unsigned char * SjisZenkanaOutputer::hankanaconv(const unsigned char input_char,unsigned char *ret){
  //Table for kana -> SJIS
  const unsigned int sjisconvtable[63] = {
	0x8142,0x8175,0x8176,0x8141,0x8145,0x8392,0x8340,0x8342,
	0x8344,0x8346,0x8348,0x8383,0x8385,0x8387,0x8362,0x815b,
	0x8341,0x8343,0x8345,0x8347,0x8349,0x834a,0x834c,0x834e,
	0x8350,0x8352,0x8354,0x8356,0x8358,0x835a,0x835c,0x835e,
	0x8360,0x8363,0x8365,0x8367,0x8369,0x836a,0x836b,0x836c,
	0x836d,0x836e,0x8371,0x8374,0x8377,0x837a,0x837d,0x837e,
	0x8380,0x8381,0x8382,0x8384,0x8386,0x8388,0x8389,0x838a,
	0x838b,0x838c,0x838d,0x838f,0x8393,0x814a,0x814b};
  //  unsigned char ret[3];
  ret[0] = sjisconvtable[input_char - 0xA1] >> 8;
  ret[1] = sjisconvtable[input_char - 0xA1] & 0xFF;
  ret[2] = (unsigned char)0;
  return(ret);
}

unsigned char *e2utable;

int UnicOutputer::LoadTable(void){
  if(e2utable == NULL){
	char *filename = new char[strlen(prefix)+strlen(kconve2utable)+4];
	PyObject *ospath,*f_join,*file;
	ospath = PyImport_ImportModule("os.path");
	f_join = PyObject_GetAttrString(ospath,"join");
	file = PyObject_CallFunction(f_join,"(ssss)",SYSPREFIX,"share","python-kconv",kconve2utable);
	PyArg_Parse(file,"s",&filename);
	FILE *IN = fopen(filename,"rb");
	if(!IN){
//          fprintf(stderr, "%s\n", filename);
	  char *mes = new char[strlen(filename)+strlen(notfoundmessage)+2];
	  sprintf(mes,"%s(%s)",notfoundmessage,mes);
	  PyErr_SetString(PyExc_IOError,mes);
	}
	e2utable = new unsigned char[131072];
	fread(e2utable,sizeof(unsigned char),(size_t)131072,IN);
	fclose(IN);
	return(1);
  }
  return(0);
}

array * UnicOutputer::output(array *input_string){
  const unsigned int eucconvtable[63] = {
	0xa1a3,0xa1d6,0xa1d7,0xa1a2,0xa1a6,0xa5f2,0xa5a1,0xa5a3,
	0xa5a5,0xa5a7,0xa5a9,0xa5e3,0xa5e5,0xa5e7,0xa5c3,0xa1bc,
	0xa5a2,0xa5a4,0xa5a6,0xa5a8,0xa5aa,0xa5ab,0xa5ad,0xa5af,
	0xa5b1,0xa5b3,0xa5b5,0xa5b7,0xa5b9,0xa5bb,0xa5bd,0xa5bf,
	0xa5c1,0xa5c4,0xa5c6,0xa5c8,0xa5ca,0xa5cb,0xa5cc,0xa5cd,
	0xa5ce,0xa5cf,0xa5d2,0xa5d5,0xa5d8,0xa5db,0xa5de,0xa5df,
	0xa5e0,0xa5e1,0xa5e2,0xa5e4,0xa5e6,0xa5e8,0xa5e9,0xa5ea,
	0xa5eb,0xa5ec,0xa5ed,0xa5ef,0xa5f3,0xa1ab,0xa1ac};
  array *out = new array();
  unsigned char *pin = (unsigned char *)input_string->getvalue();
  unsigned char *epos = pin + input_string->getlength();
  unsigned int ch;
  bool endian = 0; // big endian で出力。どっちがいいのか誰か教えて(^^;
  writehead(endian,out);
  while(pin<epos){
	ch = 0;
	if( *pin == 0x8E){
	  pin++;
	  ch = eucconvtable[*pin - 0xA1];
	}else{
	  if(isAscii(*pin)){
		if(*pin == LF){
		  switch(BLCODE){
		  case _CR:
			ch = CR;
			break;
		  case _LF:
			ch = LF;
			break;
		  case _CL:
			unsigned char hoe[2] = { CR , 0x0 };
			write(hoe,endian,out);
			break;
		  }
		}else
		  ch = *pin;
	  }else{
		ch = (*pin << 8) + *(pin+1);
		pin++;
	  }
	}
	unsigned int pos = ((ch & 0x7F00) >> 1) + (ch & 0x7F);
	if(ch != 0)
	  write(&e2utable[2*pos],endian,out);
	pin++;
  }
  return(out);
}

void UnicOutputer::write(unsigned char *unicode,bool endian,array *out){
  out->append(*(unicode+endian));
  out->append(*(unicode+1-endian));
  return;
}

void UnicOutputer::writehead(bool endian,array *out){
  out->append(0xFF - endian);
  out->append(0xFE + endian);
  return;
};

void Utf8Outputer::write(unsigned char *unicode,bool endian,array *out){
  unsigned int code = *unicode  + (*(unicode+1) << 8);
  if((code & 0xFF80) == 0)
	out->append(code);
  else if((code & 0xF800) == 0){
	out->append((code >>6) | 0xC0);
	out->append((code & 0x3F) | 0x80);
  }else{
	out->append((code >>12) + 0xE0);
	out->append(((code >> 6) & 0x3F) + 0x80);
	out->append((code & 0x3F) + 0x80);
  }
  return;
}

void Utf8Outputer::writehead(bool endain,array *out){
  return;
};

//クラスインスタンスを返す関数
Outputer * newEucOutputer(Blcs Blcode){
  return((Outputer *)new EucOutputer(Blcode));
};

Outputer * newEucZenkanaOutputer(Blcs Blcode){
  return((Outputer *)new EucZenkanaOutputer(Blcode));
};

Outputer * newSjisOutputer(Blcs Blcode){
  return((Outputer *)new SjisOutputer(Blcode));
};

Outputer * newSjisZenkanaOutputer(Blcs Blcode){
  return((Outputer *)new SjisZenkanaOutputer(Blcode));
};

Outputer * newJisOutputer(Blcs Blcode){
  return((Outputer *)new JisOutputer(Blcode));
};

Outputer * newJisZenkanaOutputer(Blcs Blcode){
  return((Outputer *)new JisOutputer(Blcode));
};

Outputer * newUnicOutputer(Blcs Blcode){
  return((Outputer *)new UnicOutputer(Blcode));
};

Outputer * newUtf8Outputer(Blcs Blcode){
  return((Outputer *)new Utf8Outputer(Blcode));
};

#ifdef DEBUG
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv){
  FILE *fin;
  if((fin = fopen("mix.EnH","r")) == NULL){
	printf("File open Error.\n");
	exit(255);
  }
  char *inputstring = new char[1024];
  fscanf(fin,"%s",inputstring);
  strcat(inputstring,"\n");
  Outputer *ip = new JisOutputer();
  printf("%s",ip->output((unsigned char*)inputstring));
  fclose(fin);
  delete ip;
  return(0);
}
#endif
