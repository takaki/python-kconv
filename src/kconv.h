// -*- c++ -*-
#include "inputer.h"
#include "outputer.h"
#include "checker.h"
#include <unistd.h>

#define GETCWD getcwd
#define WIN32_EXPORT 
A
//Version����
const char *version = KCONV_VERSION;

typedef Inputer * (*PInputer)(void);
typedef Outputer * (*POutputer)(Blcs);
typedef Checker * (*PChecker)(void);
  
  //�����������ϥ��饹���ɲä�����(ex.inputer)
  //1.Codes�ˤ��Υ��饹�����������ɤ��̤��뤿���ID���ɲä��롣(commonop.h)
  //2.Inputer��Ѿ��������֥��饹���롣(inputer.cc,inputer.h)
  //  ���饹�Υ��󥹥ȥ饯����̾����ID��񤤤Ƥ�����
  //3.���Υ��饹���󥹥��󥹤��֤��ؿ����롣(inputer.h)
  //4.��Ͽ�ơ��֥�ˤ��δؿ����ɲä��롣(kconv.h)
  //5.�ƥ���ѥ���

  //Inputer,Outputer,Checker�Ѥ���Ͽ�ơ��֥�
PInputer InputerTable[]={
  newEucInputer,
  newSjisInputer,
  newJisInputer,
  newUnicInputer,
  newUtf8Inputer,
};

POutputer OutputerTable[][2]={
  {newEucOutputer,newEucZenkanaOutputer},
  {newSjisOutputer,newSjisZenkanaOutputer},
  {newJisOutputer,newJisZenkanaOutputer},
  {newUnicOutputer,newUnicOutputer},
  {newUtf8Outputer,newUtf8Outputer},
};
  
PChecker CheckerTable[]={
  newFastChecker,
  newFullChecker,
  newTableChecker,
  newTable2Checker,
};
	
//���֥������Ȥι�¤
struct kconvobject
{
  PyObject_HEAD
  Inputer * pinputer;
  Outputer * poutputer;
  Checker * pchecker;
  int linemode;
	
};

char prefix[MAXPATHLEN]; //�ơ��֥�ξ��򼨤���

//�ơ��֥�Υե�����̾
const char * kconvu2etable = "kconvu2etable.dat";
const char * kconve2utable = "kconve2utable.dat";
const char * tc2tableefile = "kconveuctable2.dat";
const char * tc2tablesfile = "kconvsjistable2.dat";
  
PyObject *kconv_convert(PyObject *,PyObject *);
kconvobject * newkconvobject(void);
void deletekconvobject(PyObject *);
PyObject * kconv_new(PyObject *,PyObject *,PyObject *);
PyObject * kconv_mod_convert(PyObject *,PyObject *,PyObject *);
PyObject * kconv_getattr(PyObject *,char *);
extern "C" WIN32_EXPORT void initkconv(void);
int _load_defaults(void);

// kcovntools...
unsigned char * GetNextCharSJIS( char *where,
						char *hold,
						int *error/* ,encoding */);
char * ConvertGaijiAndAdd( char *convertchar,
						   char *out);
PyObject * convertgaiji( PyObject *self,
						 PyObject *args);
