#include "kstring.h"

extern "C" {
  static PyObject *ErrorObject;

  //基本演算
  static kstringobject * newkstringobject(void){
	//コンストラクタ
	kstringobject *self;
	if(!(self = PyObject_NEW(kstringobject,&kstring_type))){
	  return NULL;
	}
	self->string = NULL;
	self->length = 0;

	return self;
  };

  //#define DEBUG_DELETE
  static void deletekstringobject(PyObject *selfp){
	//デストラクタ
	kstringobject *self = (kstringobject*)selfp;
	if(self->string){
	  delete self->string;
	  self->string = NULL;
	}
#ifdef DEBUG_DELETE
	printf("deletekstringobject()\n");
#endif
	PyMem_DEL(self);
	self = NULL;
  
  }

  static PyObject * kstring_getattr(PyObject *self,char *name){
	//kstirng.???
	return(Py_FindMethod(kstring_methods,self,name));
  }
	 
  // モジュールロジック
  static PyObject * kstring_new(PyObject *self,PyObject *args,
								PyObject *keys){
	unsigned char *pstr=NULL;
	signed long int len = _UNDEFINED;
  
	if(!PyArg_ParseTuple(args,"s#",&pstr,&len))
	  return NULL;
  
  
	kstringobject *pself;

	if((pself = (kstringobject *)newkstringobject()) == NULL)
	  return NULL;
  
  //入力コードが何か調べてEUCに変換して格納
	Table2Checker chk;
	Inputer *pinputer = 
	  InputerTable[chk.ChkCoding(pstr,len)]();
	array *out = pinputer->input(pstr,len);

	pself->string = new unsigned char[out->getlength()+1];
	if(!pself->string)
	  return NULL;
	strncpy((char *)pself->string,
			(char *)out->getvalue(),out->getlength()+1);
	pself->length = out->getlength();
	
	return((PyObject*)pself);
  };


  static int kstring_print(kstringobject *self,FILE *fp,int flags){
	if(flags&Py_PRINT_RAW){
	  //ナマで表示するの
	  // >>> print pospos
	  fprintf(fp,"%s",self->string);
	}else{
	  //焼いて表示？
	  // >>> pospos
	  fprintf(fp,"'");
	  for(long int i=0;i< self->length;i++){
		if( (0x20 <= self->string[i]) &&
			(self->string[i] <= 0x7E))
		  fprintf(fp,"%c",self->string[i]);
		else
		  fprintf(fp,"\\%03o",self->string[i]);
	  }
	  fprintf(fp,"'");
	}
	return(0);
  };
  
  static int kstring_compare(kstringobject *ks1,kstringobject *ks2){
	unsigned char *pstr1,*pstr2;

	/*
	  printf("kstring_compare((%d/%d),(%d/%d))(kstring/string)\n",
	  isKstringObject(ks1),
	  PyString_Check(ks1),
	  isKstringObject(ks2),
	  PyString_Check(ks2));
	*/
		 
	if(PyString_Check(ks1)){
	  pstr1 = (unsigned char *)PyString_AS_STRING(ks1);
	}else if(isKstringObject(ks1)){
	  pstr1 = ks1->string;
	}else{
	  onError("string or kstring required.");
	}
	if(PyString_Check(ks2)){
	  pstr2 = (unsigned char *)PyString_AS_STRING(ks2);
	}else if(isKstringObject(ks2)){
	  pstr2 = ks2->string;
	}else{
	  onError("string or kstring required.");
	}
	return(strcmp((char *)pstr1,(char *)pstr2));
  };

  static int kstring_length(kstringobject *self){
	unsigned long ret = 0;
	for(long int i=0;i< self->length;i++){
	  register unsigned int c = self->string[i];
	  if(!(c&0x80)){
		//0x80よか小さければAlpabet
	  }else{
		if(c == 0x8E){
		  //SS2(半角かな) 2byte
		  i++;
		}else	if(c == 0x8F){
		  //SS3(補助漢字) 3byte
		  i++;i++;
		}else{
		  //普通の漢字 2byte
		  i++;
		}
	  }
	  ret++;
	}
	return(ret);
  };

  static PyObject * kstring_concat(kstringobject *self,PyObject *other){
	unsigned long int srclen;
	unsigned char *src;
  
	if(PyString_Check(other)){
	  //otherはstringオブジェクト
	  srclen = PyString_GET_SIZE(other);
	  src = (unsigned char *)PyString_AS_STRING(other);
	}else if(isKstringObject(other)){
	  //otherはkstringオブジェクト
	  kstringobject *okstring = (kstringobject *)other;
	  srclen = okstring->length;
	  src = okstring->string;
	}else{
	  //ohterはその他のオブジェクト
	  onError("'+' requires string or kstring.");
	};
	unsigned char *newstring = 
	  new unsigned char[self->length + srclen + 1];
	if(!newstring)
	  onError("can't allocate memory");
	strncpy((char *)newstring,
			(char *)self->string,self->length);
	strncpy((char *)(newstring+self->length),
			(char *)src,srclen);
	newstring[self->length + srclen] = 0;
  
	kstringobject *pret;
	if((pret = (kstringobject *)newkstringobject()) == NULL)
	  onError("can't allocate memory");
	pret->string = newstring;
	pret->length = self->length + srclen;
	return((PyObject *)pret);
  };

  static PyObject * kstring_repeat(kstringobject *self,int n){
	unsigned char *newstring = 
	  new unsigned char[self->length * n + 1];
	if(!newstring)
	  onError("can't allocate memory");
	for(int i = 0;i<n;i++)
	  strncpy((char *)(newstring + self->length*i),
			  (char *)self->string,self->length);
  
	newstring[self->length * n] = 0;
	kstringobject *pret;
	if((pret = (kstringobject *)newkstringobject()) == NULL)
	  onError("can't allocate memory");
	pret->string = newstring;
	pret->length = self->length * n;
	return((PyObject *)pret);
  };

  static PyObject * kstring_item(kstringobject *self,int n){
	unsigned char retc[4];
	long ret = 0;
	memset(retc,0,sizeof(ret));
  
	if(n<0){
	  //今のところn>0
	  //どうもpythonが勝手に処理しているみたい。
	  //このコードほとんど意味無し・・・かな？
	  onError("index no requires lager than or equal to 0");
	}
  
	for(long int i=0;i< self->length;i++){
	  register unsigned int c = self->string[i];
	  if(!(c&0x80)){
		//0x80よか小さければAlpabet
		if(ret == n){
		  strncpy((char *)retc,
				  (char *)self->string+i,1);
		  break;
		}
	  }else{
		if(c == 0x8E){
		  //SS2(半角かな) 2byte
		  if(ret == n){
			strncpy((char *)retc,
					(char *)self->string+i,2);
			break;
		  }
		  i++;
		}else	if(c == 0x8F){
		  //SS3(補助漢字) 3byte
		  if(ret == n){
			strncpy((char *)retc,
					(char *)self->string+i,3);
			break;
		  }
		  i++;i++;
		}else{
		  //普通の漢字 2byte
		  if(ret == n){
			strncpy((char *)retc,
					(char *)self->string+i,2);
			break;
		  }
		  i++;
		}
	  }
	  ret ++;
	}

	if(!retc[0]){
	  //ヒットしなかった
	  onError("index no out of range");
	}
  
	unsigned int len = 0;
	while(retc[len])
	  len++;
  
	unsigned char *newstring = 
	  new unsigned char[len + 1];
	strncpy((char *)newstring,
			(char *)retc,len);
	newstring[len] = 0;
	kstringobject *pret;
	if((pret = (kstringobject *)newkstringobject()) == NULL)
	  onError("can't allocate memory");
	pret->string = newstring;
	pret->length = len;
	return((PyObject *)pret);
  };

  static PyObject * kstring_slice(kstringobject *self,int bottom,int top){
	unsigned char *pbottom=NULL,*ptop=NULL;
	long int ret=0;
	if(bottom >= top){
	  kstringobject *pret;
	  if((pret = (kstringobject *)newkstringobject()) == NULL)
		onError("can't allocate memory");
	  return((PyObject *)pret);
	}	
	for(long int i=0;i< self->length;i++){
	  register unsigned int c = self->string[i];
	  if(ret == bottom)
		pbottom = self->string + i;
	  if(ret == top)
		ptop = self->string + i - 1;
	  if(!(c&0x80)){
		//0x80よか小さければAlpabet
	  }else{
		if(c == 0x8E){
		  //SS2(半角かな) 2byte
		  i++;
		}else	if(c == 0x8F){
		  //SS3(補助漢字) 3byte
		  i++;i++;
		}else{
		  //普通の漢字 2byte
		  i++;
		}
	  }
	  ret ++;
	}
	if(!pbottom)
	  onError("index no out of range");
	if(!ptop)
	  ptop = self->string + self->length - 1;
  
	unsigned int len = ptop - pbottom + 1;
	unsigned char *newstring = 
	  new unsigned char[len + 1];
	strncpy((char *)newstring,
			(char *)pbottom,len);
	newstring[len] = 0;
	kstringobject *pret;
	if((pret = (kstringobject *)newkstringobject()) == NULL)
	  onError("can't allocate memory");
	pret->string = newstring;
	pret->length = len;
	return((PyObject *)pret);
  };

  static PyObject * kstring_repfunc(kstringobject *self){
	//  printf("kstring_reprfunc\n");
	PyObject *pret = Py_BuildValue("s",self->string);
	return((PyObject *)pret);
  };

  static PyObject * kstring_str(kstringobject *self){
	//  printf("kstring_str\n");
	PyObject *pret = Py_BuildValue("s",self->string);
	return((PyObject *)pret);
  };

  void initkstring(void){
	//kconvの場所を調べる
	PyObject *ImpModule,*PythonPrefix,*FindModule;
	ImpModule = PyImport_ImportModule("imp");
	FindModule = PyObject_GetAttrString(ImpModule,"find_module");
	PythonPrefix = PyTuple_GetItem(PyEval_CallObject(FindModule,Py_BuildValue("(s)","kconv")),1);
	char *tmpprefix;
	PyArg_Parse(PythonPrefix,"s",&tmpprefix);
	if(strcmp(tmpprefix,"kconv.so") == 0) //コンパイルしたディレクトリで実行している（インストールしていない）
	  strcpy(prefix,"./");
	else
	  strcpy(prefix,tmpprefix);
	//sys moduleを呼出し、pathにkconvの場所を追加
	PyObject *SysModule,*SysDict,*SysPath;
	SysModule = PyImport_ImportModule("sys");
	SysDict = PyModule_GetDict(SysModule);
	SysPath = PyDict_GetItemString(SysDict,"path");
	PyList_Append(SysPath,Py_BuildValue("s",prefix));
  
  //Kstringで使用する定数を登録
	PyObject *module,*dict;
	module = Py_InitModule("kstring",kstring_module_methods);
	dict = PyModule_GetDict(module);
  
	//各変換テーブルの読み込み
	Inputer *tinputer = newUnicInputer();
	Outputer *toutputer = newUnicOutputer(_LF);
	delete tinputer;
	delete toutputer;
  
  //version情報
	PyObject *tobject;
	if((tobject = PyString_FromString(version))){
	  PyDict_SetItemString(dict,"__version__",tobject);
	  Py_XDECREF(tobject);
	}
  
	ErrorObject = Py_BuildValue("s","kstring.error");
	PyDict_SetItemString(dict,"error",ErrorObject);
	if (PyErr_Occurred())
	  Py_FatalError("can't initialize module kconv");
	return;
  }
}
