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
    linemode = mode 

    if linemode == WHOLE:
        if incode == AUTO:
       	    incode = CallChecker(checkmode, string)
        t_str  = CallInputer(incode, string)
        o_str  = CallOutputer(outcode, hankanaconvert, t_str, blcode)
    else:
        raise RuntimeError, 'LINE mode is Not implemented.'
    return o_str



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
                      
                       
