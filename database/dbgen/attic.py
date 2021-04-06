
class PCB_Data:
   def attrval(self, name):
      if self.xml:
         if self.xml.attributes:
            attr = self.xml.attributes.get(name, None)
            if attr:
               return attr
      return ''

   def merge_node(self, dom, node):

      if hasattr(node, "_attrs"):
         if not hasattr(dom, 'attributes'):
            dom.attributes = {}
         for attrname, attr in node._attrs.items():
            dom.attributes[attrname] = attr.value

      for child in node.childNodes:
         subdom = None
         deleted = False
         if child.attributes:
            deleted = child.attributes.get("delete", None)
         if deleted:
            continue
         child_name = child.localName
         if child_name:
            if not hasattr(dom, child_name):
               setattr(dom, child_name, [])
            attrs = getattr(dom, child_name)
            name_attr = None
            if child.attributes:
               for id in ("name", "alias", "target"):
                  name_attr = child.attributes.get(id, None)
                  if name_attr:
                     break
            for attr in attrs:
               if not name_attr or child.attributes and attr.attributes.get(id, None) == name_attr.value:
                  if deleted:
                     attrs.remove(attr)
                  else:
                     subdom = attr;
                  break;
            if not subdom:
               subdom = type('', (object,), {})()
               if not hasattr(dom, child_name):
                  setattr(dom, child_name, [])
               getattr(dom, child_name).append(subdom)
            self.merge_node(subdom, child)

   def merge(self, subdom):
      for child in subdom.childNodes:
         if child.localName:
            self.merge_node(self, child)

   def __init__(self, in_path):
      self.dom = type('', (object,), {})()
      main_dom = xml.dom.minidom.parse(str(in_path))
      inc_tags = main_dom.getElementsByTagName("include")
      for inc_tag in inc_tags:
         for attrname, attrvalue in inc_tag._attrs.items():
            if attrname == 'path':
               inc_path = Path(in_path).parent / attrvalue.value
               inc_dom = xml.dom.minidom.parse(str(inc_path))
               self.merge(inc_dom)
      self.merge(main_dom)
      self.project = self.attrval("proj")
      pass

if __name__ == "__main__":
   pcb = PCB_Data(sys.argv[1])
   pcb.Output(sys.argv[2], sys.argv[3])
   pass
