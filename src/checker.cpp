#include "checker.h"

const Codes Checker::ChkAnycode5(unsigned char *pin,double *codings){
	if((*pin == 0x00) ||
	   (*pin == 0xFF)){ //Unicodeのみに現れるコード
	  return(_UNIC);
	  codings[_UNIC]++;
	}
	if((*pin <= 0x7F) &&
	   ((0x80 <= *(pin+1)) && (*(pin+1) <= 0xBF))) //単独のUnicode trailerバイトは減点対象
	  codings[_UTF8] -= 100;

	if((0xC0 <= *pin) && (*pin <= 0xDF)){  //UTF8の2バイトコード
	  if((0x80 <= *(pin+1)) && (*(pin+1) <= 0xBF)){
		code = ((*pin & 0x1F) << 6) |  (*(pin+1) & 0x3F);
		if((u2etable[2*code] == 0) &&
		   (u2etable[2*code+1] == 0)){
		  codings[_UTF8] -= 1;
		}else{
		  codings[_UTF8] += 1;
		}
		
		if((0x80 <= *(pin+2)) && (*(pin+2) <= 0xBF))
		  //#UTF8の2バイトコードの後2バイト以上trailerバイトが続いていたら減点
		  codings[_UTF8] -= 100;
	  }else{
		codings[_UTF8] -= 100;
	  }
	}
	if((0xE0 <= *pin) && (*pin <= 0xEF)){  //UTF8の3バイトコード
	  if(((0x80 <= *(pin+1)) && (*(pin+1) <= 0xBF)) &&
		 ((0x80 <= *(pin+2)) && (*(pin+2) <= 0xBF)))
		codings[_UTF8] += 1;
	  else
		codings[_UTF8] -= 100;
	}
	if((0x80 <= *pin) && (*pin <= 0xFF)) //JISには現れないコードは減点
	  codings[_JIS] -= 100;
	if(isJisInOut(pin)){
	  return(_JIS);
	  codings[_JIS]++;
	}

	if(*pin == 0x8E) //EUC SS2
	  if(isHankana(*(pin+1)))
		 codings[_EUC]++;
	if(*pin == 0x8F) //EUC SS3
	  if(isHankana(*(pin+1)))
		if(isHankana(*(pin+2)))
		  codings[_EUC]++;

	if(!isAscii(*pin)) //Asciiでなく半角仮名でもなければJISではない。
	  if(!isHankana(*pin))
		codings[_JIS] -= 1;
	return(_UNKNOWN);
};

const Codes FastChecker::ChkCoding(const unsigned char * input_string,unsigned int length){
  Codes coding = _UNKNOWN;
  unsigned char * pin = (unsigned char *)input_string;
  for(unsigned int cc = 0;cc<length;cc++){
	if(isJisInOut(pin)){
	  coding = _JIS;
	  break;
	}
	if(!isAscii(*pin))
	  if(!isHankana(*pin)){
		coding = ChkCoding2(pin,length-cc);
		break;
	  }
	pin++;
  }
  if(coding == _UNKNOWN)
	coding = _DEFAULT_INPUT_CODING;
  return(coding);
}

const Codes FastChecker::ChkCoding2(const unsigned char * input_string,unsigned int length){
  Codes coding = _UNKNOWN;
  unsigned char *pin = (unsigned char *)input_string;
  for(unsigned int cc = 0;cc<length;cc++){
	if(isAscii(*pin)){
	  pin++;
	  continue;
	}
	if(isSjis1(*pin)){
	  coding = _SJIS;
	  break;
	}
	if(isHankana(*pin))
	  if(*(pin+1) <= 0xA0){
		coding = _SJIS;
		break;
	  }
	if(isEuc1(*pin)){
	  coding = _EUC;
	  break;
	}
	pin++;
	if(isSjis2(*pin)){
	  coding = _SJIS;
	  break;
	}
	if(isEuc2(*pin)){
	  coding = _EUC;
	  break;
	}
	pin++;
  }
  if(coding == _UNKNOWN)
	coding = _DEFAULT_INPUT_CODING;
  return(coding);
}

const Codes FullChecker::ChkCoding(const unsigned char *input_string,unsigned int length){
  unsigned char *pin = (unsigned char *)input_string;
  unsigned int codings[5] = {0,0,0,0,0};
  for(unsigned int cc = 0;cc<length;cc++){
	if((*pin == 0x00) ||
	   (*pin == 0xFF)){ //Unicodeのみに現れるコード
	  codings[_UNIC]++;
	  codings[_EUC] -= 100;
	  codings[_SJIS] -= 100;
	  codings[_JIS] -= 100;
	  codings[_UTF8] -= 100;
	}
	if((*pin <= 0x7F) &&
	   ((0x80 <= *(pin+1)) && (*(pin+1) <= 0xBF))) //単独のUnicode trailerバイトは減点対象
	  codings[_UTF8] -= 100;
	if((0xC0 <= *pin) && (*pin <= 0xDF)){  //UTF8の2バイトコード
	  if((0x80 <= *(pin+1)) && (*(pin+1) <= 0xBF))
		codings[_UTF8] += 1;
	  else
		codings[_UTF8] -= 100;
	}
	if((0xC0 <= *pin) && (*pin <= 0xDF)){  //UTF8の3バイトコード
	  if(((0x80 <= *(pin+1)) && (*(pin+1) <= 0xBF)) &&
		 ((0x80 <= *(pin+2)) && (*(pin+2) <= 0xBF)))
		codings[_UTF8] += 1;
	  else
		codings[_UTF8] -= 100;
	}
	if((0x80 <= *pin) && (*pin <= 0xFF)) //JISには現れないコードは減点
	  codings[_JIS] -= 100;
	if(isAscii(*pin))
	  if(isJisInOut(pin))
		codings[_JIS]++;
	if((*pin == 0x8E) && isHankana(*(pin+1)))
	  codings[_EUC]++;
	if((0x81 <= *pin) && (*pin <= 0x9F))
	  codings[_SJIS]++;
	if(isHankana(*pin)){
	  pin++;
	  if((0x80 <= *pin) && (*pin <= 0x9F))
		codings[_SJIS]++;
	  if((0xF0 <= *pin) && (*pin <= 0xFE))
		codings[_EUC]++;
	}
	if((0xFD <= *pin) && (*pin <= 0xFE))
	  codings[_EUC]++;
	pin++;
  }
  if(codings[_UNIC] > 0)
	return(_UNIC);
  if(codings[_UTF8] > 0)
	return(_UTF8);
  Codes max=Codes(0);
  for(int c=0;c<3;c++){
	if(codings[c] >= codings[max])
	  max=Codes(c);
  }
  return(max);
}

const Codes TableChecker::ChkCoding(const unsigned char *input_string,unsigned int length){
  unsigned char *pin = (unsigned char *)input_string;
  double codings[5] = {0,0,0,0,0};
  if((0x80 <= *pin) && (*pin <= 0xBF))
	codings[_UTF8] -= 100;
  for(unsigned int cc = 0;cc<length;cc++){
	Codes retcode;
	if((retcode = ChkAnycode5(pin,codings)) != _UNKNOWN)
	  return(retcode);
	if((start <= *pin) && (*pin < end)){
	  codings[_SJIS] += tables[*pin-start];
	  codings[_EUC] += tablee[*pin-start];
	}
	pin++;
  }
  if(codings[_UNIC] > 0)
	return(_UNIC);
  if(codings[_UTF8] > 0)
	return(_UTF8);
  if(codings[_JIS] > 0)
	return(_JIS);
  if(codings[_EUC] > codings[_SJIS])
	return(_EUC);
  if(codings[_SJIS] > codings[_EUC])
	return(_SJIS);
  return(_DEFAULT_INPUT_CODING);
}

int TableChecker_hoehoe = 0;

//tables,tablee : SJIS/EUC判別テーブル（頻度表）
double tables[end - start + 1] = {0.00223387561787, 0.0730618018026, 0.22234824392, 0.108494260574, 0.00132758734236, 0.00182077965556, 0.00249474981963, 0.00256757171932, 0.00779372318562, 0.0160764597444, 0.0163259579427, 0.0179701386874, 0.0170891562155, 0.0162740307432, 0.0222915496338, 0.0149333648663, 0.0167686160364, 0.0171465005595, 0.0116619513003, 0.0269934762876, 0.0118910965129, 0.0137699943902, 0.0140956421626, 0.0119119138253, 0.00312445417432, 0.000410155226728, 0.000510681951166, 0.000380360931956, 0.00105495019826, 0.00144583586811, 0.00206006266708, 0.00187526065171, 0.00273496148449, 0.00160889191767, 0.0113309637711, 0.000760102761683, 0.00549855644707, 0.000367514560704, 0.00237255451717, 0.00163079265903, 0.00160850497878, 0.00766069359416, 0.00973693030697, 0.00350110049291, 0.00191016253987, 0.00458313641368, 0.00157816896956, 0.00369487949059, 0.000635508438093, 0.00441907432299, 0.0014143390422, 0.00322250448985, 0.00111105633776, 0.00855289729396, 0.00218411527622, 0.00721083843699, 0.00115733422938, 0.00165625323819, 0.00117141880509, 0.00339964511512,
								  0.00176227449492, 0.00943387976586, 0.00467824599362, 0.00215114808252, 0.00119285521977, 0.00471771376072, 0.00254745089687, 0.000797016732089, 0.00825340659067, 0.00977090354178, 0.0104316403957, 0.00265796064475, 0.00831253085354, 0.0112574453814, 0.00281397440647, 0.00050704472557, 0.020764147047, 0.00993736465361, 0.00195187455255, 0.00157499607064, 0.00213164636231, 0.000614923288977, 0.00131241933775, 0.000752363983821, 0.00179725377085, 0.000255843996144, 0.0018061533654, 0.000706627806652, 0.000989634913094, 0.000887096106412, 0.00166437895495, 0.00101323818558, 0.00472963147862, 0.00185289558369, 0.000892900189809, 0.00276761912707, 0.00559536855813, 0.000789742280898, 0.00262638643107, 0.00341040201635, 0.000774574276287, 0.00294886130462, 0.00421972340524, 0.00430307004283, 0.00612980855734, 0.0107484659614, 0.00704522859073, 0.000801659998807, 0.00190257853757, 0.00265494252139, 0.00151896731891, 0.00242409477774, 0.0107198324833, 0.00407872287258, 0.000325338221352, 0.000959763230544, 0.000259403833961, 0.00123541849802, 0.000771246601806, 0.000287031270931, 0.000574294705198, 0.000166538499607, 0.00466741170461, 0.00130251370209, 0.00215300538921, 0.0, 0.0};

double tablee[end - start + 1] = {0.0, 1.25547465416e-07, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.76642396249e-07, 0.0, 0.0, 6.27737327082e-08, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 6.27737327082e-08, 0.0, 0.0, 0.0, 0.0, 0.0, 0.066129553685, 0.0160557003885, 0.0258200917375, 0.178688694388, 0.0882604331513, 0.00951725116335, 0.00147198125827, 0.00311728079256, 0.00219952882036, 0.0041537378933, 0.00855053567965, 0.00898430217266, 0.00451236422826, 0.00237228213278, 0.00860565101697, 0.00926960878782, 0.00739945374298, 0.00967537819605, 0.0112247594667, 0.00846202471653, 0.00991950524255, 0.00708464347345, 0.0155543265854, 0.0119600909717, 0.0180467576425, 0.00741163184712, 0.0111668193115, 0.020979232566, 0.0113930558441, 0.00799084507882, 0.0166875807819, 0.0129220356517, 0.00863898386903, 0.00894977661967, 0.0131318882402, 0.00912598248738, 0.0085189604921,
								  0.017455052438, 0.014174183298, 0.0172288786791, 0.00941091246015, 0.0177846773085, 0.0172332728404, 0.00913269927678, 0.0111223755087, 0.0217208414442, 0.0112842689654, 0.00642677475466, 0.00391293785463, 0.0028918603184, 0.00338582682108, 0.00361432320814, 0.0031998910248, 0.00521153806317, 0.00630606086667, 0.00309738151929, 0.00204899740933, 0.00229569817887, 0.0019481827946, 0.00420998315781, 0.00192953899598, 0.00540205634194, 0.00260699311937, 0.00254302668574, 0.00432542405226, 0.00565961696724, 0.00171064699003, 0.00316737423126, 0.0042339627237, 0.00265018144747, 0.00438907661722, 0.00504493657656, 0.00623795136668, 0.00805882903134, 0.0129636546365, 0.00760240122082, 0.00214190253374, 0.0031797406566, 0.00304509099994, 0.00155923674674, 0.00368519475237, 0.00887965836024, 0.0101748687872, 0.0017840922573, 0.00104392717494, 0.00174404261583, 0.00171924699141, 0.00143663964676, 0.000556489140458, 0.000858995758379, 0.000430376711447, 0.00459058029922, 0.00219551130147, 0.00339875821002};

unsigned int *tablee2=NULL;
unsigned int *tables2=NULL;

const Codes Table2Checker::ChkCoding(const unsigned char *input_string,unsigned int length){
  unsigned char *pin = (unsigned char *)input_string;
  double codings[5] = {0,0,0,0,0};
  if((0x80 <= *pin) && (*pin <= 0xBF))
	codings[_UTF8] -= 100;
  while(pin < input_string+length){
	Codes retcode;
	if((retcode = ChkAnycode5(pin,codings)) != _UNKNOWN)
	  return(retcode);
	unsigned char c1 = *pin;
	unsigned char c2 = *(pin+1);
	if((lstart <= c1) && (c1 <= lend)){
	  if((tstart <= c2) && (c2 <= tend)){
		codings[_SJIS] += tables2[(c1 - lstart)*tlength+(c2 - tstart)] / 65536.0;
		codings[_EUC] += tablee2[(c1 - lstart)*tlength+(c2 - tstart)] / 65536.0;
	  }
	}
	pin++;
  }
  

  //  printf("SJIS : %f , EUC : %f\n",codings[_SJIS],codings[_EUC]);

  
  if(codings[_UNIC] > 0)
	return(_UNIC);
  if(codings[_UTF8] > 0)
	return(_UTF8);
  if(codings[_JIS] > 0)
	return(_JIS);
  if(codings[_EUC] > codings[_SJIS])
	return(_EUC);
  if(codings[_SJIS] > codings[_EUC])
	return(_SJIS);
  return(_DEFAULT_INPUT_CODING);
}

Table2Checker::Table2Checker():Checker("TABLE2",_TABLE2){
  if(tablee2 == NULL){
	char *filenamee = new char[strlen(prefix)+strlen(tc2tableefile)+4];
	char *filenames = new char[strlen(prefix)+strlen(tc2tablesfile)+4];
	PyObject *ospath,*f_join,*filee,*files;
	ospath = PyImport_ImportModule("os.path");
	f_join = PyObject_GetAttrString(ospath,"join");
	char *prefix_ = "/usr/share/python-kconv";
	filee = PyObject_CallFunction(f_join,"(ss)",prefix_,tc2tableefile);
	files = PyObject_CallFunction(f_join,"(ss)",prefix_,tc2tablesfile);
	PyArg_Parse(filee,"s",&filenamee);
	PyArg_Parse(files,"s",&filenames);
	//	Py_XDECREF(ospath);
	//Py_XDECREF(f_join);
	//Py_XDECREF(filee);
	//Py_XDECREF(files);
	FILE *INe = fopen(filenamee,"rb");
	if(!INe){
	  fprintf(stderr, "%s\n", filenamee);
	  char *mes = new char[strlen(filenamee)+strlen(notfoundmessage)+2];
	  sprintf(mes,"%s(%s)",notfoundmessage,mes);
	  PyErr_SetString(PyExc_IOError,mes);
	}
	FILE *INs = fopen(filenames,"rb");
	if(!INe){
	  fprintf(stderr, "%s\n", filenames);
	  char *mes = new char[strlen(filenames)+strlen(notfoundmessage)+2];
	  sprintf(mes,"%s(%s)",notfoundmessage,mes);
	  PyErr_SetString(PyExc_IOError,mes);
	}
	tablee2 = new unsigned int[llength*tlength];
	tables2 = new unsigned int[llength*tlength];
	for(int i=0;i < llength;i++){
	  for(int ii=0;ii < tlength;ii++){
		unsigned char hoe[3];
		fread(hoe,sizeof(unsigned char),sizeof(hoe)/sizeof(unsigned char),INe);
		tablee2[i*tlength+ii] = hoe[0] + (hoe[1]<<8) + (hoe[2]<<16);
		fread(hoe,sizeof(unsigned int),sizeof(hoe)/sizeof(unsigned int),INs);
		tables2[i*tlength+ii] = hoe[0] + (hoe[1]<<8) + (hoe[2]<<16);
	  }
	}
	fclose(INe);
	fclose(INs);
  }
  return;
}

//クラスインスタンスを返す関数
Checker * newFastChecker(void){
  return((Checker *)new FastChecker());
};

Checker * newFullChecker(void){
  return((Checker *)new FullChecker());
};

Checker * newTableChecker(void){
  return((Checker *)new TableChecker());
};

Checker * newTable2Checker(void){
  return((Checker *)new Table2Checker());
};

//単独デバッグ用コード
#ifdef STANDALONE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv){
  FILE *fin;
  if((fin = fopen("zenkana.sjis","r")) == NULL){
	printf("File open Error.\n");
	exit(255);
  }
  char *inputstring = new char[1024];
  fscanf(fin,"%s",inputstring);
  strcat(inputstring,"\n");
  Checker *ip = new FastChecker();
  printf("%d",ip->ChkCoding((unsigned char*)inputstring));
  delete ip;
  fclose(fin);
  return(0);
}
#endif
