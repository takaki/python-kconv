#include <Python.h>
#include "inputer.h"
#include "outputer.h"
#include "checker.h"
#include "commonop.h"

#define onError(message) \
{ PyErr_SetString(ErrorObject,message); return NULL; }
#define is_kconvobject(v) ((v)->ob_type == &Kconvtype)
static PyObject *ErrorObject;
static PyObject * convertgaiji( PyObject *self,	 PyObject *args);

typedef Inputer * (*PInputer)(void);
typedef Outputer * (*POutputer)(Blcs);
typedef Checker * (*PChecker)(void);

static PInputer InputerTable[]={
  newEucInputer,
  newSjisInputer,
  newJisInputer,
  newUnicInputer,
  newUtf8Inputer,
};

static POutputer OutputerTable[][2]={
  {newEucOutputer,newEucZenkanaOutputer},
  {newSjisOutputer,newSjisZenkanaOutputer},
  {newJisOutputer,newJisZenkanaOutputer},
  {newUnicOutputer,newUnicOutputer},
  {newUtf8Outputer,newUtf8Outputer},
};
  
static PChecker CheckerTable[]={
  newFastChecker,
  newFullChecker,
  newTableChecker,
  newTable2Checker,
};

static PyObject*
CallInputer( PyObject *self, PyObject *args)
{
	unsigned char *str;
	int len;
	int type;
	PyObject *pret;

	Inputer *ip;
	array *ar;
	if(!PyArg_ParseTuple(args, "is#", &type, &str, &len)){
		return NULL;
	}
	ip = InputerTable[type]();
	ar = ip->input(str, len);
	pret = Py_BuildValue("s#", ar->getvalue(), ar->getlength());
	delete ip;
	delete ar;
	return pret;
}

static PyObject*
CallOutputer(PyObject *self, PyObject *args)
{
	unsigned char *str;
	int type;
	int blcs;
	int bl;
	PyObject *pret;

	Outputer *op;
	array *ar;
	if(!PyArg_ParseTuple(args, "iisi", &type, &blcs, &str, &bl)){
		return NULL;
	}
	array is;
	is.append(str);
	op = OutputerTable[type][blcs](Blcs(blcs));
	ar = op->output(&is);
	pret = Py_BuildValue("s#", ar->getvalue(), ar->getlength());
	delete op;
	delete ar;
	return pret;
}


static PyObject*
CallChecker(PyObject *self, PyObject *args)
{
	unsigned char *str;
	int type;
	int len;
	Codes ret;
	PyObject *pret;

	Checker *op;
	if(!PyArg_ParseTuple(args, "isi", &type, &str, &len)){
		return NULL;
	}
	op = CheckerTable[type]();
	ret = op->ChkCoding(str, len);
	pret = Py_BuildValue("i", ret);
	delete op;
	return pret;
};


static PyMethodDef KconvMethods[] = {
	{"UnGaiji",convertgaiji,METH_VARARGS},
	{"CallInputer", CallInputer, METH_VARARGS},
	{"CallOutputer",CallOutputer,METH_VARARGS},
	{"CallChecker", CallChecker, METH_VARARGS},

	{NULL, NULL},
};

typedef struct KconvConst {
	char *name;
	int  value;
} KconvConst;

extern "C" {
#pragma }

void
init_kconv(void)
{
	PyObject *m = Py_InitModule("_kconv", KconvMethods);

	KconvConst const_table_list[] = {
		{"AUTO", _AUTO},
		{"EUC",_EUC},
		{"SJIS",_SJIS},
		{"JIS",_JIS     },
		{"UNICODE",_UNIC },
		{"UTF8",_UTF8    },
		
		{"HANKAKU",_HANKAKU },
		{"ZENKAKU",_ZENKAKU },
		
		{"LF",_LF     },
		{"CR",_CR     },
		{"CL",_CL     },
		
		{"FAST",_FAST   },
		{"FULL",_FULL   },
		{"TABLE",_TABLE  },
		{"TABLE2",_TABLE2 },
		
		{"WHOLE",_WHOLE },
		{"LINE",_LINE  },
		{NULL,NULL}
	};
	PyObject *d = PyModule_GetDict(m);

	int n = 0;
	while(1){
		KconvConst kc = const_table_list[n];
		if (kc.name == NULL){
			break;
		}
		PyObject *i = PyInt_FromLong(kc.value);
		PyDict_SetItemString(d, kc.name, i);
		Py_DECREF(i);
		n++;
	}
	return ;
}

#pragma {
} // extern "C"

//
// check wrapper 
//

// private 

static unsigned char * GetNextCharSJIS( char *where,
					char *hold,
					int *error/* ,encoding */);
static char * ConvertGaijiAndAdd( char *convertchar,  char *out);

static unsigned char * 
GetNextCharSJIS( char *where,
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


static char * 
ConvertGaijiAndAdd( char *convertchar,
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


static PyObject *
convertgaiji( PyObject *self, PyObject *args)
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

// -*- c++ -*-

char prefix[MAXPATHLEN]; //テーブルの場所を示す。
Codes _DEFAULT_INPUT_CODING;
//テーブルのファイル名
const char * kconvu2etable = "kconvu2etable.dat";
const char * kconve2utable = "kconve2utable.dat";
const char * tc2tableefile = "kconveuctable2.dat";
const char * tc2tablesfile = "kconvsjistable2.dat";
  

