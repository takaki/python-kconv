#include "kconv.h"

#ifndef KSTRING_H
#define KSTRING_H

Codes _DEFAULT_INPUT_CODING = _EUC;

#define isKstringObject(v) ((v)->ob_type == &kstring_type)
#define onError(mes)\
{ PyErr_SetString(ErrorObject,mes);return NULL; }


extern "C" {
  //オブジェクトの構造
  struct kstringobject
  {
	PyObject_HEAD
	unsigned char *string;
	signed long int length;
  };
  
  static struct PyMethodDef kstring_methods[] = {
	{NULL,NULL},
  };
  
  //関数プロトタイプ
  static kstringobject * newkstringobject(void);
  static void deletekstringobject(PyObject *);
  static PyObject * kstring_new(PyObject *,PyObject *,PyObject *);
  static int kstring_print(kstringobject *,FILE *,int);
  static PyObject * kstring_getattr(PyObject *,char *);
  static int kstring_compare(kstringobject *,kstringobject *);
  static int kstring_length(kstringobject *);
  static PyObject * kstring_concat(kstringobject *,PyObject *);
  static PyObject * kstring_repeat(kstringobject *,int);
  static PyObject * kstring_item(kstringobject *,int);
  static PyObject * kstring_slice(kstringobject *,int,int);
  static PyObject * kstring_repfunc(kstringobject *);
  static PyObject * kstring_str(kstringobject *);
  void initkstring(void);

  //タイプデスクリプタ
  //シーケンス型の関数テーブル
  PySequenceMethods kstring_seq = {
	(inquiry) kstring_length,
	(binaryfunc) kstring_concat,
	(intargfunc) kstring_repeat,
	(intargfunc) kstring_item,
	(intintargfunc) kstring_slice,
	(intobjargproc) NULL,
	(intintobjargproc) NULL
  };
  
  PyTypeObject kstring_type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"Kstring",
	sizeof(kstringobject),
	0,
	
	(destructor) deletekstringobject,
	(printfunc) kstring_print,
 	(getattrfunc) kstring_getattr,
	NULL,
	(cmpfunc) kstring_compare,
	(reprfunc) kstring_repfunc,
		
	NULL,
	&kstring_seq,
	NULL,
	
	(hashfunc) NULL,
	NULL,
	(reprfunc) kstring_str,
	(getattrofunc) NULL,
	NULL
  };
  
  static struct PyMethodDef kstring_module_methods[] = {
	{ "kstring" ,(PyObject *(*)(PyObject *,PyObject *))kstring_new, 1},
	{ NULL,NULL,0}
  };
  

} // End of extern "C"

#endif
