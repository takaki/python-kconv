//Kconv 0.5 Outputer
//EUC文字列をそれぞれのコードに変換
#include "commonop.h"
#ifdef HAVE_CONFIG_H
#include "configure.h"
#endif
#ifndef HAVE_LIBGCC
// #include "opnew.h"
#endif
#include "array.h"

extern char prefix[MAXPATHLEN]; //テーブルの場所を示す。
extern const char *kconve2utable;//filename

class Outputer : public CommonOp {
 protected:
  Blcs BLCODE;
 public:
  //Outputer Interface & some methods
  virtual array * output(array *)=0;//Outputerのインターフェース
  Outputer(char * n,Codes c,Blcs Blcode):CommonOp(n,c),BLCODE(Blcode){
  };
};

class JisOutputer : public Outputer {
 public:
  //EUC -> JIS
  virtual const unsigned char * hankanaconv(const unsigned char,unsigned char *);
  array * output(array *);
  JisOutputer(Blcs Blcode):Outputer("JIS",_JIS,Blcode){
  };
 protected:
  bool inJisMode;
};

class JisZenkanaOutputer : public JisOutputer{
  //EUC -> JIS 全角仮名に変換
 public:
  const unsigned char * hankanaconv(const unsigned char,unsigned char *);
  JisZenkanaOutputer(Blcs Blcode):JisOutputer(Blcode){
  };
};

//Table for kana -> JIS
static const unsigned int jisconvtable[63] = {
  0x2123,0x2156,0x2157,0x2122,0x2126,0x2572,0x2521,0x2523,
  0x2525,0x2527,0x2529,0x2563,0x2565,0x2567,0x2543,0x213c,
  0x2522,0x2524,0x2526,0x2528,0x252a,0x252b,0x252d,0x252f,
  0x2531,0x2533,0x2535,0x2537,0x2539,0x253b,0x253d,0x253f,
  0x2541,0x2544,0x2546,0x2548,0x254a,0x254b,0x254c,0x254d,
  0x254e,0x254f,0x2552,0x2555,0x2558,0x255b,0x255e,0x255f,
  0x2560,0x2561,0x2562,0x2564,0x2566,0x2568,0x2569,0x256a,
  0x256b,0x256c,0x256d,0x256f,0x2573,0x212b,0x212c};
  

class EucOutputer : public Outputer{
 public:
  //EUC -> EUC 何もしない(^^;
  array * output(array *);
  EucOutputer(Blcs Blcode):Outputer("EUC",_EUC,Blcode){
  };
};

class EucZenkanaOutputer : public EucOutputer{
  //EUC -> EUC 全角仮名に変換
 public:
  //半角仮名を全EUCに変換（テーブル使用）
  array * output(array *);
  EucZenkanaOutputer(Blcs Blcode):EucOutputer(Blcode){
  };
};

//Table for kana -> EUC
static const unsigned int eucconvtable[63] = {
  0xa1a3,0xa1d6,0xa1d7,0xa1a2,0xa1a6,0xa5f2,0xa5a1,0xa5a3,
  0xa5a5,0xa5a7,0xa5a9,0xa5e3,0xa5e5,0xa5e7,0xa5c3,0xa1bc,
  0xa5a2,0xa5a4,0xa5a6,0xa5a8,0xa5aa,0xa5ab,0xa5ad,0xa5af,
  0xa5b1,0xa5b3,0xa5b5,0xa5b7,0xa5b9,0xa5bb,0xa5bd,0xa5bf,
  0xa5c1,0xa5c4,0xa5c6,0xa5c8,0xa5ca,0xa5cb,0xa5cc,0xa5cd,
  0xa5ce,0xa5cf,0xa5d2,0xa5d5,0xa5d8,0xa5db,0xa5de,0xa5df,
  0xa5e0,0xa5e1,0xa5e2,0xa5e4,0xa5e6,0xa5e8,0xa5e9,0xa5ea,
  0xa5eb,0xa5ec,0xa5ed,0xa5ef,0xa5f3,0xa1ab,0xa1ac};

class SjisOutputer : public Outputer{
 public:
  //EUC -> SJIS
  array * output(array *);
  SjisOutputer(Blcs Blcode):Outputer("SJIS",_SJIS,Blcode){
  };
  virtual const unsigned char * hankanaconv(const unsigned char,unsigned char *);
  //半角仮名には触れない
};

class SjisZenkanaOutputer : public SjisOutputer{
 public:
  SjisZenkanaOutputer(Blcs Blcode):SjisOutputer(Blcode){
  };
  //半角仮名を全角SJISに変換（テーブル使用）
  const unsigned char * hankanaconv(const unsigned char,unsigned char *);
};

//Table for kana -> SJIS
static const unsigned int sjisconvtable[63] = {
  0x8142,0x8175,0x8176,0x8141,0x8145,0x8392,0x8340,0x8342,
  0x8344,0x8346,0x8348,0x8383,0x8385,0x8387,0x8362,0x815b,
  0x8341,0x8343,0x8345,0x8347,0x8349,0x834a,0x834c,0x834e,
  0x8350,0x8352,0x8354,0x8356,0x8358,0x835a,0x835c,0x835e,
  0x8360,0x8363,0x8365,0x8367,0x8369,0x836a,0x836b,0x836c,
  0x836d,0x836e,0x8371,0x8374,0x8377,0x837a,0x837d,0x837e,
  0x8380,0x8381,0x8382,0x8384,0x8386,0x8388,0x8389,0x838a,
  0x838b,0x838c,0x838d,0x838f,0x8393,0x814a,0x814b};

extern unsigned char *e2utable;

class UnicOutputer : public Outputer{
 protected:
  virtual void write(unsigned char *unicode,bool endian,array *out);
  virtual void writehead(bool endian,array *out);
  int LoadTable(void);
 public:
  UnicOutputer(char *n,Codes c,Blcs Blcode):Outputer(n,c,Blcode){
	LoadTable();
  };
  UnicOutputer(Blcs Blcode):Outputer("UNICODE",_UNIC,Blcode){
	LoadTable();
  };
  array * output(array *);
};

class Utf8Outputer : public UnicOutputer{
 protected:
  void write(unsigned char *unicode,bool endian,array *out);
  void writehead(bool endian,array *out);
 public:
  Utf8Outputer(Blcs Blcode):UnicOutputer("UTF8",_UTF8,Blcode){
  };
};

extern "C"{
//クラスインスタンスを返す関数
Outputer * newEucOutputer(Blcs);
Outputer * newEucZenkanaOutputer(Blcs);
Outputer * newSjisOutputer(Blcs);
Outputer * newSjisZenkanaOutputer(Blcs);
Outputer * newJisOutputer(Blcs);
Outputer * newJisZenkanaOutputer(Blcs);
Outputer * newUnicOutputer(Blcs);
Outputer * newUnicZenkanaOutputer(Blcs);
Outputer * newUtf8Outputer(Blcs);
Outputer * newUtf8ZenkanaOutputer(Blcs);
};
