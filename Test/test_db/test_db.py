import sys, pathlib
from pcb import *

types = [Type('Comp', [Prop(name, 'string'), Rel(one2many, 'Pin')], shape=yes),
         Type('Pin', [Prop(name, 'string')])
       ]

enums = []

if __name__ == "__main__":
   if len(sys.argv) > 1:
      out_path = sys.argv[1]
   else:
      out_path = str(pathlib.Path(__file__).parent.absolute())
   generate(out_path, types, enums)
