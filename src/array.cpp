/*
  class array
  長さ可変の文字列
  array::append(char)で追加し
  array::getvalue()で文字列を
  array::getlength()で長さを得る。
  とりあえず0x0ターミネートはしてある。
  array::getlength()は最後の0x0の長さを含まない。
  
  ver 3.
  途中に0x0を含むことができるようになった。
  とともにarray::getlength()を追加
  
  ver 2.
  listを使うことにしてメモリの再配置をやめた。
  
  ver 1.
  適当に作ってみた。
*/										  

#include "array.h"

array::array(unsigned long l){
  base = NULL;
  start = new list(l);
  pos = start;
  length = 0;
  return;
}

void array::rewind(void){
  unsigned long l = start->length;
  if(start->isEmpty())
	return;
  delete start;
  delete base;
  start = NULL;
  base = NULL;
  start = new list(l);
  pos = start;
  length = 0;
  return;
}

array::~array(){
  delete start;
  delete base;
  start = NULL;
  base = NULL;
}

//#define DEBUG
void array::append(const unsigned char in){
  length++;
  if(pos->lappend(in)) //現在のlistへ追加。
	return;
#ifdef DEBUG
  printf("list full.");
#endif
  pos->next = new list(pos->length * 2 + 1); //新しいリストが必要。
  pos = pos->next;
#ifdef DEBUG
  printf("new pos=%x\n",pos);
#endif
  pos->lappend(in);
  return;
}

void array::append(const unsigned char *pin){
  while(*pin){
	append(*pin);
	pin++;
  }
  return;
}

void array::append(const unsigned char *pin,unsigned int len){
  for(unsigned int hoe = 0;hoe<len;hoe++){
	append(*pin);
	pin++;
  }
}
  
void array::append(const char *pin){
  append((const unsigned char*)pin);
  return;
}

const unsigned char * array::getvalue(void){
  if(base) //2度目以降のgetvalue()
	return(base);
  base = new unsigned char[length+1];
  unsigned char *pt = base;
  list * pl = start;
  while(pl){
	if(pl->next){
	  memcpy((char *)pt,(char *)pl->base,pl->length);
	  pt += (pl->length);
	  pl = pl->next;
	  continue;
	}
	memcpy((char *)pt,(char *)pl->base,(pl->pt - pl->base) + 1);
	pl = pl->next;
  }
  base[length] = 0; //Cで扱うための終端文字列。Pythonから使うだけなら不要。
  return(base);
}
  
unsigned int array::getlength(void){
  return(length);
}

array::list::list(unsigned long size){
  next = NULL;
  length = size;
  base = new unsigned char[length];
  pt = base - 1;
#ifdef DEBUG
  printf("\nnew list at %x(%d) ",this,length);
#endif
  return;
}

array::list::~list(){
  if(next)
	delete next;
  delete base;
  return;
}

bool array::list::lappend(const unsigned char in){
  if(((unsigned int)(pt - base + 1)) < length){
	*++pt = in;
	return true;  //appendに成功
  }
  return false;  //appendに失敗（listに空きがなし）
}

bool array::list::isEmpty(void){
  if(pt == (base - 1))
	return true;
  return false;
}

//#define STANDALONE
#ifdef STANDALONE

int main(int argc,char **argv)
{
  array str = array(3);
  //          123456789012345678901234567890123456789012345678901234567890
  str.append("Fly me to the moon,and let me play among the stars.\n"); //52
  str.append("Let me see what spring is like on Jupter and Mars.\n"); //51
  str.append("In ohter words,hold my hand,In other words,darling kiss me.\n"); //60
  /*  unsigned char asdf[1024];
  FILE *fi;
  fi = fopen("zenkana.euc","r");
  fscanf(fi,"%s",&asdf);
  str.append(asdf);
  rewind(fi);
  fscanf(fi,"%s",&asdf);
  str.append(asdf);*/
  printf("String is ---[][][]---\n%s---[][][]---\n",str.getvalue());
  printf("length is %d\n",str.getlength());
  return(0);
}
#endif
