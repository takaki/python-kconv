from kconvtools import *
from _kconv import *

DEFAULT_INPUT_CODING   = EUC
DEFAULT_OUTPUT_CODING  = EUC
DEFALUT_BREAKLINE_CODE = LF
DEFAULT_CHECK_MODE     = TABLE

def convert(string,
            outcode = DEFAULT_OUTPUT_CODING,
            incode = AUTO,
            hankanaconvert = ZENKAKU,
            checkmode = DEFAULT_CHECK_MODE,
            mode = WHOLE,
            blcode = DEFALUT_BREAKLINE_CODE):
    outputer = _OutputerTable[outcode][hankanaconvert]
    if incode == AUTO:
        inputer = None
        checker = _CheckerTable[checkmode]
    else:
        inputer = _InputerTable[incode]
    linemode = mode 

    if linemode == WHOLE:
        if inputer == None:
            inputer = _InputerTable[checker(string)]
            tmp_string = inputer(string)
            out_string = outputer(tmp_string, blcode)
        else:
            tmp_string = inputer(string)
            out_string = outputer(tmp_string, blcode)
    else:
        raise RuntimeError, 'Not implemented LINE'
    return out_string

#   }else{//一行ごとに変換するみたい。 一行ごとだったら絶対にAUTOモードにしちゃう。
# 	array *out = new array();
# 	crl artemis(input_string,string_length);
# 	int pos = 0;
# 	int UnicodeMode = -1; // -1 -> not Unicode , 0 -> Big Endian , 1 -> Little Endian
# 	unsigned int line_length;
# 	while(pos != -1){
# 	  unsigned char *line = artemis.ReadLine(pos,line_length);
# 	  if((*line == 0xFF) && (*(line+1) == 0xFE)) //Unicode Big-Endian
# 		UnicodeMode = 0;
# 	  if((*line == 0xFE) && (*(line+1) == 0xFF)) //Unicode Little-Endian
# 		UnicodeMode = 1;
	  
# 	  if(UnicodeMode >= 0){ //Unicodeらしいです。
# 		if(self->pinputer)
# 		  delete(self->pinputer);
# 		self->pinputer = newUnicInputerEndian(UnicodeMode);
# 		if(line_length != 0){
# 		  array *temp_string = self->pinputer->input(line,line_length);
# 		  pout = self->poutputer->output(temp_string);
# 		  //pout = self->poutputer->output((unsigned char*)(self->pinputer->input(ukyu,line_length)));
# 		  out->append(pout->getvalue(),pout->getlength());
# 		  delete pout;
# 		  delete temp_string;
# 		}
# 		if(pos != -1)
# 		  out->append(0x0A);
# 		delete self->pinputer;
# 		self->pinputer = NULL;
# 	  }else{ //Unicodeではないのです。
# 		if(self->pinputer)
# 		  delete self->pinputer;
# 		self->pinputer = InputerTable[self->pchecker->ChkCoding(line,line_length)]();
# 		if(line_length != 0){
# 		  array *temp_string = self->pinputer->input(line,line_length);
# 		  pout = self->poutputer->output(temp_string);
# 		  //pout = self->poutputer->output((unsigned char*)(self->pinputer->input(ukyu,line_length)));
# 		  out->append(pout->getvalue(),pout->getlength());
# 		  delete pout;
# 		  delete temp_string;
# 		}
# 		if(pos != -1)
# 		  out->append(0x0A);
# 		delete self->pinputer;
# 		self->pinputer = NULL;
# 	  }
# 	  //	  delete ukyu;
# 	}
# 	pret = Py_BuildValue("s#",out->getvalue(),out->getlength());
# 	delete out;
#   }



def ChkCoding(string, checkmode=DEFAULT_CHECK_MODE):
    checker = _CheckerTable[checkmode]
    return checker(string)

class Kconv:
    def __init__(self,
                 outcode = DEFAULT_OUTPUT_CODING,
                 incode = AUTO,
                 hankanaconvert = ZENKAKU,
                 checkmode = DEFAULT_CHECK_MODE,
                 mode = WHOLE,
                 blcode = DEFALUT_BREAKLINE_CODE):
        pass
    
    def convert(self, string):
        return convert(string,
                       self.outcode,
                       self.incode,
                       self.hankanaconvert,
                       self.checkmode,
                       self.mode,
                       self.blcode)
                      
                       
