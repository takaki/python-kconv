//Unicode ��ޤह�٤Ƥβ��ԥ����ɤ�ǧ������ReadLine()
//����ʸ���ϴޤޤʤ���Unicode�Υ���ǥ�����ؼ�������(0xFEFF)���֤�
//int &pos : õ���Ϥ��ǽ��ʸ����Index
//unsigned int &olen : �֤�ʸ�����Ĺ��
//�֤��� : 1��ʬ��ʸ������֤�
#include "crl.h"

unsigned char * crl::ReadLine(int &startpos,unsigned int &olen){
  if(ReadLine_UnicodeFlag == 1)
	return(ReadLineUnicode(startpos,olen));
  if(isBIG(input_string+startpos) | isLIT(input_string+startpos)){
	ReadLine_UnicodeFlag = 1;
	UnicodeChar_StartPos = startpos;
	return(ReadLineUnicode(startpos,olen));
  }
  int hoe = REC->search(input_string,startpos);
  if(hoe < 0)
	RETSTRING(startpos,len,-1)

  if( isBIG(input_string+hoe) || isLIT(input_string+hoe)){
	ReadLine_UnicodeFlag = 1;
	UnicodeChar_StartPos = hoe;
	RETSTRING(startpos,REC->regs[0],REC->regs[0])
  }
  RETSTRING(startpos,REC->regs[0],REC->regs[1])
}

unsigned char * crl::ReadLineUnicode(int &startpos,unsigned int &olen){
  int hoe = REUC[ReadLine_UnicodeEndian]->search(input_string,startpos);
  if(hoe < 0)
	RETSTRING(startpos,len,-1)
  while((( hoe - UnicodeChar_StartPos) % 2) == 1){
	//	int tstartpos = hoe + 1;
	hoe = REUC[ReadLine_UnicodeEndian]->search(input_string,startpos);
	if(hoe < 0)
	  RETSTRING(startpos,len,-1)
  }
  int reg0 = REUC[ReadLine_UnicodeEndian]->regs[0];
  int reg1 = REUC[ReadLine_UnicodeEndian]->regs[1];
  if(isBIG(input_string+reg0)){
	ReadLine_UnicodeEndian = 0;
	if(startpos == reg0)
	  RETSTRING(startpos,reg1,reg1)
	else
	  RETSTRING(startpos,reg0,reg0)
  }
  if(isLIT(input_string+reg0)){
	ReadLine_UnicodeEndian = 1;
	if(startpos == reg0)
	  RETSTRING(startpos,reg1,reg1)
	else
	  RETSTRING(startpos,reg0,reg0)
  }
  RETSTRING(startpos,reg0,reg1)
}

int rec::search(unsigned char * input_string,int startpos){
  for(int pos = startpos;pos < len;pos++){
	if(((*(input_string+pos) == 0xFE) && (*(input_string+pos+1) == 0xFF)) || //Little Endian
	   ((*(input_string+pos) == 0xFF) && (*(input_string+pos+1) == 0xFE)) || //Big Endian
	   ((*(input_string+pos) == 0x0D) && (*(input_string+pos+1) == 0x0A))){  //CR + LF
	  regs[0] = pos;
	  regs[1] = pos+2;
	  return(pos);
	}
	if((*(input_string+pos) == 0x0D) || (*(input_string+pos) == 0x0A)){ //CR or LF
	  regs[0] = pos;
	  regs[1] = pos+1;
	  return(pos);
	}
  }
  return(-1);
}

int reub::search(unsigned char * input_string,int startpos){
  for(int pos = startpos;pos < len;pos++){
	if(((*(input_string+pos) == 0xFE) && (*(input_string+pos+1) == 0xFF)) || //Little Endian
	   ((*(input_string+pos) == 0xFF) && (*(input_string+pos+1) == 0xFE))){  //Big Endian
	  regs[0] = pos;
	  regs[1] = pos+2;
	  return(pos);
	}
	if((*(input_string+pos) == 0x0D) && (*(input_string+pos+1) == 0x00) &&
	   (*(input_string+pos+2) == 0x0A) && (*(input_string+pos+3) == 0x00)){ //Unicode Big-Endian CR+LF
	  regs[0] = pos;
	  regs[1] = pos+4;
	  return(pos);
	}
	if(((*(input_string+pos) == 0x0A) && (*(input_string+pos+1) == 0x00)) || //Unicode Big-Endian LF
	   ((*(input_string+pos) == 0x0D) && (*(input_string+pos+1) == 0x00))){  //Unicode Big-Endian CR
	  regs[0] = pos;
	  regs[1] = pos+2;
	  return(pos);
	}
  }
  return(-1);
}

int reul::search(unsigned char * input_string,int startpos){
  for(int pos = startpos;pos < len;pos++){
	if(((*(input_string+pos) == 0xFE) && (*(input_string+pos+1) == 0xFF)) || //Little Endian
	   ((*(input_string+pos) == 0xFF) && (*(input_string+pos+1) == 0xFE))){  //Big Endian
	  regs[0] = pos;
	  regs[1] = pos+2;
	  return(pos);
	}
	if((*(input_string+pos) == 0x00) && (*(input_string+pos+1) == 0x0D) &&
	   (*(input_string+pos+2) == 0x00) && (*(input_string+pos+3) == 0x0A)){ //Unicode Little-Endian CR+LF
	  regs[0] = pos;
	  regs[1] = pos+4;
	  return(pos);
	}
	if(((*(input_string+pos) == 0x00) && (*(input_string+pos+1) == 0x0A)) || //Unicode Little-Endian LF
	   ((*(input_string+pos) == 0x00) && (*(input_string+pos+1) == 0x0D))){  //Unicode Little-Endian CR
	  regs[0] = pos;
	  regs[1] = pos+2;
	  return(pos);
	}
  }
  return(-1);
}

//#define STANDALONE
#ifdef STANDALONE
#include <stdio.h>

int main(int argc,char **argv){
  char * name = "Sawatari Honoka\rSugihara Manami\nIto Noemi\r\nYuki Mizho";
  crl line((unsigned char*)name,strlen(name));
  int spos = 0;
  unsigned int olen;
  while(spos >= 0){
	unsigned char *ostr = line.ReadLine(spos,olen);
	unsigned char *hoe = new unsigned char[olen+1];
	memcpy(hoe,ostr,olen);
	printf("%s(%d)\n",hoe,olen);
  }
}
#endif
