// -*- c++ -*-
#include "inputer.h"
#include "outputer.h"
#include "checker.h"
#include <unistd.h>

#define GETCWD getcwd
#define WIN32_EXPORT 
A
//Version情報
const char *version = KCONV_VERSION;

typedef Inputer * (*PInputer)(void);
typedef Outputer * (*POutputer)(Blcs);
typedef Checker * (*PChecker)(void);
  
  //新しい入出力クラスを追加する手順(ex.inputer)
  //1.Codesにそのクラスが扱うコードを識別するためのIDを追加する。(commonop.h)
  //2.Inputerを継承したサブクラスを作る。(inputer.cc,inputer.h)
  //  クラスのコンストラクタに名前とIDを書いておく。
  //3.そのクラスインスタンスを返す関数を作る。(inputer.h)
  //4.登録テーブルにその関数を追加する。(kconv.h)
  //5.再コンパイル

  //Inputer,Outputer,Checker用の登録テーブル
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
	
//オブジェクトの構造
struct kconvobject
{
  PyObject_HEAD
  Inputer * pinputer;
  Outputer * poutputer;
  Checker * pchecker;
  int linemode;
	
};

char prefix[MAXPATHLEN]; //テーブルの場所を示す。

//テーブルのファイル名
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
