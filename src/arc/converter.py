# -*- coding: utf-8 -*-
import sys
import re
import glob
import os
import StringIO

here = os.path.dirname(os.path.abspath(__file__))

def convert():
    for file_name in glob.glob(here+'/*'):
        arcsource = re.findall(r'^(.*\.arc)$', file_name)
        if len(arcsource) != 0:
            output = StringIO.StringIO()
            with file(arcsource[0], "r") as f:
                output.write('"')
                for ln in f:
                    nl = re.sub(r'[\n\r]', ' ', ln)
                    nl = re.sub(r'["]', '\\"', nl)
                    output.write(nl)
                output.write('",')
            with file(arcsource[0]+'.core', 'w') as f:
                output.seek(0, 0)
                f.write(output.getvalue())


convert()
