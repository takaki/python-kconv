#ifndef COMMONOP
#define COMMONOP

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

const int _UNDEFINED = -1;
const unsigned char LF = 0x0A;
const unsigned char CR = 0x0D;
const unsigned char NC = 0x00;

const unsigned char ESC = 0x1B;

enum Codes { _AUTO = -2, _UNKNOWN, _EUC , _SJIS , _JIS , _UNIC , _UTF8};//UNKNOWN = AUTO in Inputer
enum LineMode { _WHOLE,_LINE };
enum KanaMode { _HANKAKU , _ZENKAKU };
enum ChkMode { _FAST , _FULL ,_TABLE,_TABLE2};
enum Blcs { _LF , _CR , _CL };

//Euc code Data
static const int elstart = 0xA1;
static const int elend = 0xFE;
static const int etstart = 0xA1;
static const int etend = 0xFE;
  
//Sjis code Data
static const int slstart = 0x80;
static const int slend = 0xFC;
static const int ststart = 0x40;
static const int stend = 0xFC;
  
//Table Size
static const int lstart = (elstart > slstart) ? slstart : elstart;
static const int lend  = (elend > slend) ? elend : slend;
static const int tstart = (etstart > ststart) ? ststart : etstart;
static const int tend = (etend > stend) ? etend : stend;
  
static const int llength = lend - lstart + 1;
static const int tlength = tend - tstart + 1;
  
//Error Message
#define notfoundmessage "IOError: No such file or directory:"


class CommonOp{
  //共通小物だよ
 protected:
  char* name;
  int code;
 public:
  virtual const int getCode(void){ //自分の受け持つコードを返す
	return(code);
  };
  virtual const char * getName(void){ //Python側で指定する時の名前を返す
	return(name);
  };
 protected:
  CommonOp(char * n,int c):name(n),code(c){
  };
  inline bool isAscii(const unsigned char in){
	if(in <= 0x7F)
	  return true;
	return false;
  }
  
  inline bool isCtrl(const unsigned char in){
	if(in <= 0x1F)
	  return true;
	if(in == 0x7F)
	  return true;
	return false;
  }
  
  inline bool isPict(const unsigned char in){
	if(in <= 0x7E)
	  return true;
	return false;
  }
  
  inline bool isHankana(const unsigned char in){
	if((0xA1 <= in) & (in  <= 0xDF))
	  return true;
	return false;
  }

  inline bool isEuc(const unsigned char in){
	if((0xA1 <= in) && (in <= 0xFE))
	  return(true);
	return(false);
  }
};

#endif

