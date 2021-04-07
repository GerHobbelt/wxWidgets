import sys, pathlib
from pcb import *

types = [
      Type('Board', [
           Prop(name, string),
           Rel(one2many, 'Stackup'),
           Rel(one2many, 'BoardRegion'),
           Rel(one2many, 'Cavity'),
         ], shape=yes),
      Type('Stackup', [
           Prop(name, string),
           Rel(many2many, 'Layer'),
         ]),
      Type('Layer', [
           Rel(one2many, 'AreaFill'),
           Rel(one2many, 'Component'),
           Rel(one2many, 'Path'),
           Prop(name, string),
           Prop('LayerNumber', integer),
         ]),
      Type('BoardRegion', [
           Prop(name, string),
           Prop('BoardRegionType', 'BoardRegionType'),
           Rel(one2many, 'BoardRegion'),
           Rel(one2many, 'Stackup'),
           Rel(one2many, 'Component'),
           Rel(one2many, 'AreaFill'),
           Rel(one2many, 'Path'),
           Rel(one2many, 'Pin'),
           Rel(one2many, 'Via'),
           Rel(one2many, 'MountingHole'),
           Rel(one2many, 'Fiducial'),
           Rel(one2many, 'Cavity'),
           Rel(one2many, 'BendArea'),
         ], shape=yes),
      Type('Component', [
           Prop(name, string),
         ], shape=yes),
      Type('Pin', [
           Prop(name, string),
         ]),
      Type('Cavity', [
           Prop(name, string),
           Prop('StartLayer', integer),
           Prop('EndLayer', integer),
         ], shape=yes),
      Type('MountingHole', [
           Prop(name, string),
         ]),
      Type('Fiducial', [
         ]),
      Type('Via', [
         ]),
      Type('Path', [
         ], shape=yes),
      Type('AreaFill', [
         ], shape=yes),
      Type('BendArea', [
         ], shape=yes),
   ]

enums = [
   Enum('BoardRegionType', [
         'Rigid', 'Flex'
      ]),
   ]

if __name__ == "__main__":
   if len(sys.argv) > 1:
      out_path = sys.argv[1]
   else:
      out_path = str(pathlib.Path(__file__).parent.absolute())
   generate(out_path, types, enums)
