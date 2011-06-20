/*
  class array
  Ĺ�����Ѥ�ʸ����
  array::append(char)���ɲä�
  array::getvalue()��ʸ�����
  array::getlength()��Ĺ�������롣
  �Ȥꤢ����0x0�����ߥ͡��ȤϤ��Ƥ��롣
  array::getlength()�ϺǸ��0x0��Ĺ����ޤޤʤ���
  
  ver 3.
  �����0x0��ޤळ�Ȥ��Ǥ���褦�ˤʤä���
  �ȤȤ��array::getlength()���ɲ�
  
  ver 2.
  list��Ȥ����Ȥˤ��ƥ���κ����֤��᤿��
  
  ver 1.
  Ŭ���˺�äƤߤ���
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
  if(pos->lappend(in)) //���ߤ�list���ɲá�
	return;
#ifdef DEBUG
  printf("list full.");
#endif
  pos->next = new list(pos->length * 2 + 1); //�������ꥹ�Ȥ�ɬ�ס�
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
  if(base) //2���ܰʹߤ�getvalue()
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
  base[length] = 0; //C�ǰ�������ν�üʸ����Python����Ȥ������ʤ����ס�
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
	return true;  //append������
  }
  return false;  //append�˼��ԡ�list�˶������ʤ���
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
