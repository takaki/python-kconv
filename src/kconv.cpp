#include <Python.h>
#include "array.h"
#include "crl.h"
#include "kconv.h"

#define onError(message) \
{ PyErr_SetString(ErrorObject,message); return NULL; }
#define is_kconvobject(v) ((v)->ob_type == &Kconvtype)

//temporary comment out.
//const char *notfoundmessage = "IOError: No such file or directory:";

Codes _DEFAULT_INPUT_CODING = _UNKNOWN;
Codes _DEFAULT_OUTPUT_CODING = _UNKNOWN;
Blcs _DEFAULT_BREAKLINE_CODE = Blcs(0);
ChkMode _DEFAULT_CHECK_MODE = _TABLE;

static PyObject *ErrorObject;

// タイプデスクリプタ
static PyTypeObject Kconvtype = {
  //タイプヘッダ
	PyObject_HEAD_INIT(&PyType_Type)
  0,
  "kconv",
  sizeof(kconvobject),
  0,
  //標準メソッド
  (destructor)deletekconvobject,
  0,
  (getattrfunc)kconv_getattr,
  0,
  0,
  0,
  //タイプカテゴリ
  0,
  0,
  0,
  //other methods
  0,
  0,
  0,
};

//#define DEBUG
//インスタンスメソッド
PyObject *kconv_convert(PyObject *selfp,PyObject *args){
  PyObject * pret=NULL;
  kconvobject *self = (kconvobject*)selfp;
  unsigned char *input_string;
  unsigned int string_length;
  array * pout;
  
  //Stringとその長さを得る。
  if(!PyArg_ParseTuple(args,"s#",&input_string,&string_length))
	return(NULL);
  //変換開始
  if(self->linemode == _WHOLE){ //全文同時変換モード。
	if(!(self->pinputer)){ //まだinputerが決定していない。(AUTO mode)
#ifdef DEBUG
	  printf("input-code is %d.\n",self->pchecker->ChkCoding(input_string,string_length));
#endif
	  self->pinputer = InputerTable[self->pchecker->ChkCoding(input_string,string_length)]();
	  
	  array *temp_string = self->pinputer->input(input_string,string_length);
	  pout = self->poutputer->output(temp_string);
	  pret = Py_BuildValue("s#",pout->getvalue(),pout->getlength());
	  delete temp_string;
	  delete pout;
	  
	  delete self->pinputer;
	  self->pinputer = NULL;
	}else{//inputerが決定してるの。
	  
	  array *temp_string = self->pinputer->input(input_string,string_length);
	  pout = self->poutputer->output(temp_string);
	  pret = Py_BuildValue("s#",pout->getvalue(),pout->getlength());
	  delete temp_string;
	  delete pout;
	  
	}
  }else{//一行ごとに変換するみたい。 一行ごとだったら絶対にAUTOモードにしちゃう。
	array *out = new array();
	crl artemis(input_string,string_length);
	int pos = 0;
	int UnicodeMode = -1; // -1 -> not Unicode , 0 -> Big Endian , 1 -> Little Endian
	unsigned int line_length;
	while(pos != -1){
	  unsigned char *line = artemis.ReadLine(pos,line_length);
	  if((*line == 0xFF) && (*(line+1) == 0xFE)) //Unicode Big-Endian
		UnicodeMode = 0;
	  if((*line == 0xFE) && (*(line+1) == 0xFF)) //Unicode Little-Endian
		UnicodeMode = 1;
	  
	  /*
		//crl.ReadLine()が返す文字列は0x00ターミネートされていないのでターミネートする
		unsigned char *ukyu = new unsigned char[line_length+1];
		if(line_length != 0){
		memcpy(ukyu,line,line_length);
		ukyu[line_length] = 0;
		}
	  */
	  
	  if(UnicodeMode >= 0){ //Unicodeらしいです。
		if(self->pinputer)
		  delete(self->pinputer);
		self->pinputer = newUnicInputerEndian(UnicodeMode);
		if(line_length != 0){
		  array *temp_string = self->pinputer->input(line,line_length);
		  pout = self->poutputer->output(temp_string);
		  //pout = self->poutputer->output((unsigned char*)(self->pinputer->input(ukyu,line_length)));
		  out->append(pout->getvalue(),pout->getlength());
		  delete pout;
		  delete temp_string;
		}
		if(pos != -1)
		  out->append(0x0A);
		delete self->pinputer;
		self->pinputer = NULL;
	  }else{ //Unicodeではないのです。
		if(self->pinputer)
		  delete self->pinputer;
		self->pinputer = InputerTable[self->pchecker->ChkCoding(line,line_length)]();
		if(line_length != 0){
		  array *temp_string = self->pinputer->input(line,line_length);
		  pout = self->poutputer->output(temp_string);
		  //pout = self->poutputer->output((unsigned char*)(self->pinputer->input(ukyu,line_length)));
		  out->append(pout->getvalue(),pout->getlength());
		  delete pout;
		  delete temp_string;
		}
		if(pos != -1)
		  out->append(0x0A);
		delete self->pinputer;
		self->pinputer = NULL;
	  }
	  //	  delete ukyu;
	}
	pret = Py_BuildValue("s#",out->getvalue(),out->getlength());
	delete out;
  }
  return(pret);
}

static PyMethodDef kconv_methods[] = {
  { "convert",kconv_convert,1},
  { NULL,NULL}
};

//基本演算
kconvobject * newkconvobject(void){ //コンストラクタ
  kconvobject *self;
  if(!(self = PyObject_NEW(kconvobject,&Kconvtype))){
	return NULL;
  }
  self->pinputer = NULL;
  self->poutputer = NULL;
  self->pchecker = NULL;
  return self;
}

void deletekconvobject(PyObject *selfp){ //デストラクタ
  kconvobject *self = (kconvobject*)selfp;
  if(self->pinputer)
	delete self->pinputer;
  if(self->poutputer)
	delete self->poutputer;
  if(self->pchecker)
	delete self->pchecker;
  PyMem_DEL(self);
  self = NULL;
}

PyObject *kconv_getattr(PyObject *self,char *name){
  return(Py_FindMethod(kconv_methods,self,name));
}
	 
// モジュールロジック
PyObject * kconv_new(PyObject *self,PyObject *args,  PyObject *keywds){
  //デフォルト値の読み込み。一度だけ行なう
  if(_DEFAULT_INPUT_CODING == _UNKNOWN)
	_load_defaults();

  int ocode,icode,km,cm,hm,blc;
  ocode=icode=km=cm=hm=blc=_UNDEFINED;
  static char *kwlist[] = {"outcode",
						   "incode",
						   "hankanaconvert",
						   "checkmode",
						   "mode",
						   "blcode",
						   NULL};
  if(!PyArg_ParseTupleAndKeywords(args,keywds,"|iiiiii",kwlist,&ocode,&icode,&km,&cm,&hm,&blc))
	return NULL;
  //デフォルトの値設定
  if(ocode == _UNDEFINED)//出力コード
	ocode = _DEFAULT_OUTPUT_CODING;
  if(icode == _UNDEFINED)//入力コード
	icode = _AUTO;
  if(km == _UNDEFINED)//半角->全角コンバートフラグ
	km = _ZENKAKU;
  if(cm == _UNDEFINED)//入力コード判別ルーチン選択
	cm = _DEFAULT_CHECK_MODE;
  if(hm == _UNDEFINED)//変換モード
	hm = _WHOLE;
  if(blc == _UNDEFINED)//改行コード
	blc = _DEFAULT_BREAKLINE_CODE;

  kconvobject *pself;

  if((pself = (kconvobject *)newkconvobject()) == NULL)
	return NULL;
  
  pself->poutputer = OutputerTable[ocode][km](Blcs(blc));
  if( (icode == _AUTO) || (icode == _UNKNOWN) )
	pself->pinputer = NULL;
  else
	pself->pinputer = InputerTable[icode]();
  if(pself->pinputer == NULL)
	pself->pchecker = CheckerTable[cm]();
  pself->linemode = hm;

  return((PyObject*)pself);
}

PyObject * kconv_mod_convert(PyObject *self,PyObject *args,PyObject *keywds){
  //デフォルト値の読み込み。一度だけ行なう
  if(_DEFAULT_INPUT_CODING == _UNKNOWN)
	_load_defaults();

  int ocode,icode,km,cm,hm,blc;
  ocode=icode=km=cm=hm=blc=_UNDEFINED;
  PyObject *str=NULL;
  static char *kwlist[] = {"str",
						   "outcode",
						   "incode",
						   "hankanaconvert",
						   "checkmode",
						   "mode",
						   "blcode",
						   NULL};
  if(!PyArg_ParseTupleAndKeywords(args,keywds,"O|iiiiii",kwlist,
								  &str,&ocode,&icode,&km,&cm,&hm,&blc))
	return NULL;
  //デフォルトの値設定
  if(ocode == _UNDEFINED)//出力コード
	ocode = _DEFAULT_OUTPUT_CODING;
  if(icode == _UNDEFINED)//入力コード
	icode = _AUTO;
  if(km == _UNDEFINED)//半角->全角コンバートフラグ
	km = _ZENKAKU;
  if(cm == _UNDEFINED)//入力コード判別ルーチン選択
	cm = _DEFAULT_CHECK_MODE;
  if(hm == _UNDEFINED)//変換モード
	hm = _WHOLE;
  if(blc == _UNDEFINED)//改行コード
	blc = _DEFAULT_BREAKLINE_CODE;

  kconvobject *pself;

  if((pself = (kconvobject *)newkconvobject()) == NULL)
	return NULL;
  
  pself->poutputer = OutputerTable[ocode][km](Blcs(blc));
  if( (icode == _AUTO) || (icode == _UNKNOWN) )
	pself->pinputer = NULL;
  else
	pself->pinputer = InputerTable[icode]();
  if(pself->pinputer == NULL)
	pself->pchecker = CheckerTable[cm]();
  pself->linemode = hm;

  PyObject *argtuple = Py_BuildValue("(O)",str);
  return(kconv_convert((PyObject *)pself,argtuple));
}

PyObject * kconv_mod_ChkCoding(PyObject *self,PyObject *args,PyObject *keywds){
  //デフォルト値の読み込み。一度だけ行なう
  if(_DEFAULT_INPUT_CODING == _UNKNOWN)
	_load_defaults();

  int cm = _UNDEFINED;
  unsigned char *input_string;
  unsigned int string_length;
  static char *kwlist[] = {"str",
						   "checkmode",
						   NULL};
  if(!PyArg_ParseTupleAndKeywords(args,keywds,"s#|i",kwlist,
								  &input_string,&string_length,&cm))
	return NULL;
  //デフォルトの値設定
  if(cm == _UNDEFINED)//入力コード判別ルーチン選択
	cm = _DEFAULT_CHECK_MODE;
  
  Checker *pchecker = CheckerTable[cm]();
  Codes ret = pchecker->ChkCoding(input_string,string_length);
  delete pchecker;
  return(Py_BuildValue("i",ret));
}

int _load_defaults(void){
  PyObject *DefModule,*DefDict;
  //	PyObject *defin,*defout;
  DefModule = PyImport_ImportModule("defaults");
  DefDict = PyModule_GetDict(DefModule);
  _DEFAULT_INPUT_CODING = Codes(PyInt_AsLong(PyDict_GetItemString(DefDict,"DEFAULT_INPUT_CODING")));
  _DEFAULT_OUTPUT_CODING = Codes(PyInt_AsLong(PyDict_GetItemString(DefDict,"DEFAULT_OUTPUT_CODING")));
  _DEFAULT_BREAKLINE_CODE = Blcs(PyInt_AsLong(PyDict_GetItemString(DefDict,"DEFALUT_BREAKLINE_CODE")));
  _DEFAULT_CHECK_MODE = ChkMode(PyInt_AsLong(PyDict_GetItemString(DefDict,"DEFAULT_CHECK_MODE")));
  Py_XDECREF(DefDict);
  Py_XDECREF(DefModule);
  return(0);
}

  
static PyMethodDef KconvtypeMethods[] = {
  {"Kconv",(PyCFunction)kconv_new,METH_VARARGS|METH_KEYWORDS},
  {"convert",(PyCFunction)kconv_mod_convert,METH_VARARGS|METH_KEYWORDS},
  {"ChkCoding",(PyCFunction)kconv_mod_ChkCoding,METH_VARARGS|METH_KEYWORDS},
  {"UnGaiji",(PyCFunction)convertgaiji,METH_VARARGS},
  {NULL, NULL}
};

//ADD DICTionary
#define ADDDICT(PyValue,CpValue,Key) \
if((PyValue = PyInt_FromLong(CpValue))){ \
  PyDict_SetItemString(dict,Key,PyValue);\
  Py_XDECREF(PyValue); \
}

//ADD DICTionary String
#define ADDDICTS(CpValue,Key) \
if((tobject = PyInt_FromLong(CpValue))){ \
  PyDict_SetItemString(dict,Key,tobject);\
  Py_XDECREF(tobject); \
}

extern "C" WIN32_EXPORT void initkconv(void) 
{ 
  //kconvの場所を調べる
  PyObject *ImpModule,*PythonPrefix,*FindModule;
  ImpModule = PyImport_ImportModule("imp");
  FindModule = PyObject_GetAttrString(ImpModule,"find_module");
  PythonPrefix = PyTuple_GetItem(PyEval_CallObject(FindModule,Py_BuildValue("(s)","kconv")),1);
  char *tmpprefix;
  PyArg_Parse(PythonPrefix,"s",&tmpprefix);
  char *dir = GETCWD(NULL,MAXPATHLEN+1);
  //  printf("dir(%s),tmpprefix(%s)\n",dir,tmpprefix);

#ifdef WIN32
# define SHARED_PREFIX ".pyd"
# define PATHCHR '\\'
#else
  // UNIX の SHARED_PREFIXはconfigure & Makefile で定義される
# define PATHCHR '/'
#endif
  char *firstpos = strchr(tmpprefix,PATHCHR);
  char *lastpos = strrchr(tmpprefix,PATHCHR);
  if( firstpos == tmpprefix )
	sprintf(prefix,"%s",tmpprefix);
  else{
	if(lastpos == NULL)
	  sprintf(prefix,"%s%c",dir,PATHCHR);
	else
	  sprintf(prefix,"%s%c%s",dir,PATHCHR,tmpprefix);
  }
  //  printf("tmpprefix(%s) , prefix(%s)\n",tmpprefix,prefix);

  //sys moduleを呼出し、pathにkconvの場所を追加
  PyObject *SysModule,*SysDict,*SysPath;
  SysModule = PyImport_ImportModule("sys");
  SysDict = PyModule_GetDict(SysModule);
  SysPath = PyDict_GetItemString(SysDict,"path");
  PyList_Append(SysPath,Py_BuildValue("s",prefix));
  
  //Kconvで使用する定数を登録
  PyObject *module,*dict;
  PyObject *tobject; //一時的に使い回すPyObject変数
  module = Py_InitModule("kconv",KconvtypeMethods);
  dict = PyModule_GetDict(module);

  //version情報
  if((tobject = PyString_FromString(version))){
	PyDict_SetItemString(dict,"__version__",tobject);
	Py_XDECREF(tobject);
  }  
  unsigned int ccc; //Temporary counter

  //inputerが使う定数
  for(ccc = 0;ccc < sizeof(InputerTable)/sizeof(PInputer);ccc++){
	Inputer *tobject2 = InputerTable[ccc](); //一時的に使い回すPyObject変数
	ADDDICTS(tobject2->getCode(),(char *)tobject2->getName());
	delete tobject2;
  };

  //outputerが使う定数:いまのところ無駄
  for(ccc = 0;ccc < sizeof(OutputerTable)/sizeof(POutputer)/2;ccc++){
	Outputer *tobject2 = OutputerTable[ccc][0](_LF); //一時的に使い回すPyObject変数
	ADDDICTS(tobject2->getCode(),(char *)tobject2->getName());
	delete tobject2;
  };
  
  //checkerが使う定数
  for(ccc = 0;ccc < sizeof(CheckerTable)/sizeof(PChecker);ccc++){
	Checker *tobject2 = CheckerTable[ccc](); //一時的に使い回すPyObject変数
	ADDDICTS(tobject2->getCode(),(char *)tobject2->getName());
	delete tobject2;
  };
  ADDDICTS(_ZENKAKU,"ZENKAKU");
  ADDDICTS(_HANKAKU,"HANKAKU");
  ADDDICTS(_LINE,"LINE");
  ADDDICTS(_WHOLE,"WHOLE");
  ADDDICTS(_LF,"LF");
  ADDDICTS(_AUTO,"AUTO");
  ADDDICTS(_UNKNOWN,"UNKNOWN");
  ADDDICTS(_CR,"CR");
  ADDDICTS(_CL,"CL");
  
  //  printf("Loading Kconvtools\n");
  //Kconvtoolsの読み込み
  PyObject *ToolsModule,*ToolsDict;
  ToolsModule = PyImport_ImportModule("kconvtools");
  //  printf("Module at %X\n",ToolsModule);
  if(!ToolsModule)
	exit(0);
  ToolsDict = PyModule_GetDict(ToolsModule);

  /*
	Py_XDECREF(ToolsModule);
	Py_XDECREF(ToolsDict);
	Py_XDECREF(ImpModule);
	Py_XDECREF(PythonPrefix);
	Py_XDECREF(FindModule);
  */

  //I must not decref Sys
  //  Py_XDECREF(SysDict);
  //  Py_XDECREF(SysModule);
  ErrorObject = Py_BuildValue("s","kconv.error");
  PyDict_SetItemString(dict,"error",ErrorObject);
  if (PyErr_Occurred())
	Py_FatalError("can't initialize module kconv");
  return;
}

// kconvtools...
unsigned char * GetNextCharSJIS( char *where,
						char *hold,
						int *error/* ,encoding */)
{

  //char c;
  unsigned char *whereweare = (unsigned char *)where; /*pointer along the where string */

  *error = 0;

  /*  printf("%02x",*whereweare);*/
  /* one byte chars */
  if (((*whereweare >= 0x01) && (*whereweare <= 0x7f)) ||
      ((*whereweare >= 0xa1) && (*whereweare <= 0xdf)))
    {
      *hold++ = *whereweare++;
      *hold= '\0';
      /*      printf("\n"); */
      return whereweare;
    }
  

  /* Two byte chars */
  if (((*whereweare >= 0x81) && (*whereweare <= 0x9f)) ||
      ((*whereweare >= 0xe0) && (*whereweare <= 0xfc)))
    {
      whereweare++;
      if (((*whereweare >= 0x40) && (*whereweare <= 0x7e))||
      ((*whereweare >= 0x80) && (*whereweare <= 0xfc)))
	{
	  /*	  printf("%02x\n",*whereweare) ; */
	  whereweare--;
	}
      else
	{
	  /*	  printf("%02x\n",*whereweare) ;*/
	  /*printf("Bad SJIS(second byte bad char)\n");*/
	  *error = 1;
	  whereweare++;
	  return whereweare++;
	}
      *hold++ = *whereweare++;
      *hold++ = *whereweare++;
      *hold= '\0';
      return whereweare;
    }

  /*  printf("Bad SJIS(not in any group) char\n");*/
  *error = 1;
  return whereweare;

}


char * ConvertGaijiAndAdd( char *convertchar,
						   char *out)
{

  char *whereweare = out;
  unsigned char * convertpointer = (unsigned char *)convertchar;
  
  if ( *(convertpointer) == 0x84 )
    {
      
      if (*(convertpointer+1) >=0x92)
	{
	  *whereweare++ = 0x81;
	  *whereweare++ = 0x40;
	}
      else
	{
	  *whereweare++= *(convertpointer)++;
	  *whereweare++= *(convertpointer)++;
	}
      return whereweare;
    }
  
  if ( ( *(convertpointer) >= 0x85 ) &&  ( *(convertpointer) <= 0x86 ))
    {
      *whereweare++ = 0x81;
      *whereweare++ = 0x40;
       return whereweare;
    }

  if ( *(convertpointer) == 0x87 )
    {
      if (*(convertpointer+1) >= 0x9d)
	{
	  *whereweare++ = 0x81;
	  *whereweare++ = 0x40;
	}
      else
	{
	  *whereweare++= *(convertpointer)++;
	  *whereweare++= *(convertpointer)++;
	}
      return whereweare;
    }

  if ( *(convertpointer) == 0x88 )
    {
      if (*(convertpointer+1) <= 0x9e)
	{
	  *whereweare++ = 0x81;
	  *whereweare++ = 0x40;
	}
      else
	{
	  *whereweare++= *(convertpointer)++;
	  *whereweare++= *(convertpointer)++;
	}
      return whereweare;
    }

   if ( *(convertpointer) == 0xea )
    {
      if (*(convertpointer+1) >=0xa5)
	{
	  *whereweare++ = 0x81;
	  *whereweare++ = 0x40;
	}
      else
	{
	  *whereweare++= *(convertpointer)++;
	  *whereweare++= *(convertpointer)++;
	}
      return whereweare;
    }
  

  if ( ( *(convertpointer) >= 0xeb ) &&  ( *(convertpointer) <= 0xec ))
    {
      *whereweare++ = 0x81;
      *whereweare++ = 0x40;
       return whereweare;
    }

  if ( *(convertpointer) == 0xee )
    {
      if (*(convertpointer+1) >=0xfd)
	{
	  *whereweare++ = 0x81;
	  *whereweare++ = 0x40;
	}
      else
	{
	  *whereweare++= *(convertpointer)++;
	  *whereweare++= *(convertpointer)++;
	}
      return whereweare;
    }
  
  if ( ( *(convertpointer) >= 0xef ) &&  ( *(convertpointer) <= 0xfa ))
    {
      *whereweare++ = 0x81;
      *whereweare++ = 0x40;
       return whereweare;
    }
  
  if ( *(convertpointer) == 0xfc )
    {
      if (*(convertpointer+1) >=0x4b)
	{
	  *whereweare++ = 0x81;
	  *whereweare++ = 0x40;
	}
      else
	{
	  *whereweare++= *(convertpointer)++;
	  *whereweare++= *(convertpointer)++;
	}
      return whereweare;
    }
  
  while (*convertpointer) 
    *whereweare++ = *convertpointer++;
  return whereweare;
}


PyObject * convertgaiji( PyObject *self,
						 PyObject *args)
{

  char * input_string, *output_string; /* the input and output strings */

  char * inwhereweare; /* pointer to where we are in the input string */
  char * outwhereweare; /* pointer to where we are in the output string */
  char holdchar[4];
  int in_len; /* length of input_string used to calculate how long the
		 output string should be */
  int error;
  PyObject *out; /* The python object to return */
  
  /* 'convert' python arguments to c arguments */
  if (!PyArg_ParseTuple(args, "s", &input_string)) 
    {
      return NULL;
    }
  
  /* figure out how long our output_string needs to be */
  in_len = strlen(input_string);

  /* allocate enough space for the results to be stored */
  output_string = (char*)malloc((size_t)in_len+1);
  memset(output_string,0,in_len+1);
  inwhereweare = input_string;
  outwhereweare = output_string;

  while (*inwhereweare) 
    {
      /* get the next character and update inwhereweare */
      inwhereweare = (char *)GetNextCharSJIS(inwhereweare,holdchar,&error);

      if (!error)
	{
	  /* now append the character or converted character */
	  outwhereweare = ConvertGaijiAndAdd(holdchar,outwhereweare);
	}
      else 
	  onError("Bad SJIS char\n");
    }

  /* convert the relevant c value into a python value */
  out = Py_BuildValue("s",output_string);
  
  /* free the allocated memory */
  free((void *)output_string);
  
  /* return the value we wish to pass to python */
  return out;
}
