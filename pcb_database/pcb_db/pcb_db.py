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
           Rel(one2many, 'Comp'),
           Rel(one2many, 'Trace'),
           Prop(name, string),
           Prop('LayerNumber', integer),
           Prop('LayerType', 'LayerType'),
           Prop('ConductiveLayerType', 'ConductiveLayerType'),
           Prop('DielectricLayerType', 'DielectricLayerType'),
           Prop('ManufacturingLayerType', 'ManufacturingLayerType'),
           Prop('Thickness', coord),
           Prop('DielectricConstant', real),
         ]),
      Type('BoardRegion', [
           Prop(name, string),
           Prop('BoardRegionType', 'BoardRegionType'),
           Rel(one2many, 'BoardRegion'),
           Rel(one2many, 'Stackup'),
           Rel(one2many, 'Comp'),
           Rel(one2many, 'AreaFill'),
           Rel(one2many, 'Trace'),
           Rel(one2many, 'Pin'),
           Rel(one2many, 'Via'),
           Rel(one2many, 'MountingHole'),
           Rel(one2many, 'Fiducial'),
           Rel(one2many, 'Cavity'),
           Rel(one2many, 'BendArea'),
         ], shape=yes),
      Type('Comp', [
           Prop(name, string),
           Rel(one2many, 'Pin'),
           Prop('Position', point),
           Prop('CompType', 'CompType'),
           Prop('PartType', string),
           Prop('Layer', integer),
         ], shape=yes),
      Type('Pin', [
           Prop(name, string),
           Prop('Position', point),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Pad', [
         ], shape=yes),
      Type('Cavity', [
           Prop(name, string),
           Prop('StartLayer', integer),
           Prop('EndLayer', integer),
         ], shape=yes),
      Type('MountingHole', [
           Prop(name, string),
           Prop('Position', point),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Fiducial', [
           Prop(name, string),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Via', [
           Prop('Position', point),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Trace', [
           Prop('Beg', point),
           Prop('End', point),
           Prop('Width', coord),
           Prop('Layer', integer),
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
   Enum('LayerType', [
         'Unknown', 'Conductive', 'Dielectric', 'Manufacturing'
      ]),
   Enum('ConductiveLayerType', [
         'Signal', 'Plane', 'PlaneNegative', 'Split'
      ]),
   Enum('DielectricLayerType', [
         'Substrate', 'Conformal', 'Environment', 'SolderMaskTop', 'SolderMaskBottom', 'Adhesive', 'Coverfilm'
      ]),
   Enum('ManufacturingLayerType', [
         'SilkscreenTop', 'SilkscreenBottom', 'SolderPasteTop', 'SolderPasteBottom'
      ]),
   Enum('CompType', [
      'Unknown', 'IC', 'Resistor', 'Capacitor', 'Inductor', 'FerriteBead', 'Diode', 'Connector', 'TestPoint'
      ]),
   ]

if __name__ == "__main__":
   if len(sys.argv) > 1:
      out_path = sys.argv[1]
   else:
      out_path = str(pathlib.Path(__file__).parent.absolute())
   generate(out_path, types, enums, 'PCBDATABASE')
