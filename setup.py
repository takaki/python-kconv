#! /usr/bin/env python
from distutils.core import setup, Extension
import sys

VERSION = "1.3.0"
setup(name="python-kconv",
      version=VERSION,
      description="Python Kanji-code converter",
      author="TANIGUCHI, Takaki",
      author_email="takaki@assist.media.nagoya-u.ac.jp",
      license = "GPL",
      url="http://kconv.sourceforge.jp/",
      
      data_files = [('share/python-kconv',
                     ['data/kconve2utable.dat',
                      'data/kconveuctable2.dat',
                      'data/kconvsjistable2.dat',
                      'data/kconvu2etable.dat',
                      ])],

      py_modules = ["kconv.defaults",
                    "kconv.kconvtools",
                    ],
      
      ext_modules = [Extension(name = '_kconv',
                               sources=['src/_kconv.cpp',
                                        'src/array.cpp',
                                        'src/checker.cpp',
                                        'src/crl.cpp',
                                        'src/inputer.cpp',
                                        'src/outputer.cpp'
                                        ],
                               define_macros=[('STDC_HEADERS', '1'),
                                              ('KCONV_VERSION',
                                               '"%s"' % VERSION),
                                              ('SYSPREFIX',
                                               '"%s"' % sys.prefix),
                                              ],
                               ),
                     ]
      )
                     
