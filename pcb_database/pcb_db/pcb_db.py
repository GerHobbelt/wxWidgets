import sys, pathlib
from pcb import *

types = [
      Type('AttributeName', [
           Prop(name, string),
           Rel(one2many, 'Attribute'),
         ]),
      Type('Attribute', [
           Prop('Value', string),
         ]),
      Type('BoardRegion', [
           Prop(name, string),
           Prop('BoardRegionType', 'BoardRegionType'),
           Rel(one2many, 'BoardRegion'),
           Rel(many2many, 'Layer'),
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
         ], plane=yes),
      Type('Comp', [
           Prop(name, string),
           Rel(one2one, 'Outline'),
           Rel(one2one, 'Outline', 'PlacementOutline'),
           Rel(one2many, 'Pin'),
           Rel(one2many, 'Attribute'),
           Prop('Position', point),
           Prop('CompType', 'CompType'),
           Prop('PartType', string),
           Prop('Layer', integer),
         ], shape=yes),
      Type('Net', [
           Prop(name, string),
           Rel(one2many, 'Pin'),
           Rel(one2many, 'Via'),
           Rel(one2many, 'Trace'),
           Rel(one2many, 'Link'),
           Rel(one2many, 'AreaFill'),
           Rel(one2many, 'MountingHole'),
           Rel(one2many, 'Attribute'),
         ]),
      Type('NetClass', [
           Prop(name, string),
           Rel(one2many, 'Net'),
         ]),
      Type('Pin', [
           Prop(name, string),
           Prop('Position', point),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Pad', [
           Prop('Layer', integer),
         ], shape=yes),
      Type('Teardrop', [
           Prop(name, string),
           Prop('Layer', integer),
         ], shape=yes),
      Type('Text', [
           Prop('Layer', integer),
           Prop('Text', string),
           Prop('FontName', string),
           Prop('FontBold', boolean),
           Prop('FontItalic', boolean),
           Prop('FontUnderline', boolean),
           Prop('FontHeight', coord),
           Prop('FontWidth', coord),
           Prop('LineWidth', coord),
           Prop('Mirrored', boolean),
           Prop('Position', point),
           Prop('Rotation', real),
         ], shape=yes),
      Type('Outline', [
         ], shape=yes),
      Type('Cavity', [
           Prop(name, string),
           Prop('StartLayer', integer),
           Prop('EndLayer', integer),
         ], shape=yes),
      Type('Drawing', [
           Prop('Layer', integer),
         ], shape=yes),
      Type('MountingHole', [
           Prop(name, string),
           Prop('Position', point),
           Prop('DrillSize', coord),
           Prop('Plated', boolean),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Fiducial', [
           Prop(name, string),
           Prop('Side', integer),
           Prop('Position', point),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Via', [
           Prop('Position', point),
           Prop('DrillSize', coord),
           Rel(one2many, 'Pad', parent_type='Object'),
         ]),
      Type('Trace', [
           Prop('Beg', point),
           Prop('End', point),
           Prop('Width', coord),
           Prop('TieLeg', boolean),
         ], shape=yes),
      Type('Link', [
           Prop('Beg', point),
           Prop('End', point),
           Prop('Layer1', integer),
           Prop('Layer2', integer),
         ]),
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
