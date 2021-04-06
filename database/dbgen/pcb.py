import re, os, filecmp
from pathlib import Path
from tempfile import NamedTemporaryFile

#keywords:
yes = True
no = False
name = 'Name'
shape = 'shape'
values = 'values'
properties = 'properties'
relationships = 'relationships'

#relation types:
one2one = 'One2One'
one2many = 'One2Many'

class Prop:
   def __init__(self, name, type):
      self.name = name
      self.type = type
      self.is_rel = False
      pass

   def generate_backend_includes(self, parent):
      return ""

   def generate_backend_data(self, parent):
      text = f"// property {self.name}\n";
      text += f"{self.data_type} m_{self.name};\n";
      return text

   def generate_backend_methods(self, parent):
      text = f"// property {self.name}\n";
      text += f"{self.data_type} get{self.name}() const{{return m_{self.name};}}\n";
      text += f"""void set{self.name}(const {self.data_type}& val){{if (m_{self.name}!=val) {{
         before_propmodify(cDbTraits::ePropId::{self.id}, m_{self.name});m_{self.name}=val;
         after_propmodify(cDbTraits::ePropId::{self.id}, m_{self.name});}}}}\n\n"""
      return text

class Rel:
   def __init__(self, type, to):
      self.type = type
      self.to = to
      self.is_rel = True
      pass

   def generate_backend_includes(self, parent):
      if (not self.parent_ref):
         return f'#include "{self.to}.h"\n';
      else:
         return f'class c{self.to};';

   def generate_backend_data(self, parent):
      return ""

   def generate_backend_methods(self, parent):
      text = f"// relationship {parent.name}->{self.to}\n";
      if (not self.parent_ref):
         text += f"void include{self.to}(c{self.to}& x){{include(cDbTraits::eRelId::{self.id}, x);}}\n";
         text += f"void exclude{self.to}(c{self.to}& x){{exclude(cDbTraits::eRelId::{self.id}, x);}}\n";
      if self.parent_ref:
         parent_ref = "true"
         text += f"c{self.to}* parent{self.to}() const {{return (c{self.to}*)parent(cDbTraits::eRelId::{self.id});}}\n";
      else:
         parent_ref = "false"
      text += f"size_t count{self.to}() const{{return count(cDbTraits::eRelId::{self.id}, {parent_ref});}}\n\n";
      return text

class Type:
   def __init__(self, name, contents, shape=False):
      self.name = name
      self.shape = shape
      self.contents = contents
      pass

class Enum:
   def __init__(self, name, values):
      self.name = name
      self.values = values

class FileGen:
   def __enter__(self):
      return self

   def __init__(self, directory, class_name, extension, overwrite = True):
      self.directory = Path(directory.replace('"', ""))
      self.filename = Path(class_name + extension)
      self.extension = extension
      self.overwrite = overwrite
      self.contents = ""

      if self.filename.suffix == ".h":
         self.contents += """
#pragma once

"""

   def __exit__(self, exception_type, exception_value, traceback):

      self.directory.mkdir(parents = True, exist_ok = True)
      output = self.directory / self.filename

      try:
         existing_file = output.open()
      except:
         existing_file = False

      if existing_file and not self.overwrite:
         existing_file.close()
         return self
      
      temp_file = NamedTemporaryFile('w+t', dir = self.directory, suffix = self.extension, delete = False)
      temp_name = temp_file.name
      
      temp_file.write(self.contents)
      temp_file.close()

      clang = os.environ['CLANG_FORMAT'];
      if clang != "":
         os.system(clang + " -i " + temp_name)
      
      if not existing_file:
         os.rename(temp_name, output)
         return
      
      existing_file.close()

      unchanged = filecmp.cmp(temp_name, output, shallow = False)
      if unchanged and self.overwrite <= 1:
         os.remove(temp_name)
         return self

      mode = os.stat(output).st_mode
      os.chmod(output, 0o777)
      os.remove(output)
      os.rename(temp_name, output)
      os.chmod(output, mode)
      return self

def generate_backend_header(path, type):
   with FileGen(path, type.name, ".h", True) as fg:
      fg.contents += f'#include "database_traits.h"\n\n'
      for cont in type.contents:
         fg.contents += cont.generate_backend_includes(type)

      fg.contents += f"""

      class c{type.name}: public db::cObject<cDbTraits>{{public:

"""
      for cont in type.contents:
         fg.contents += cont.generate_backend_data(type)

      fg.contents += f"""
         public:
            c{type.name}(): cObject(cDbTraits::eObjId::{type.id}){{}}
            ~c{type.name}(){{on_destroy();}}

"""
      for cont in type.contents:
         fg.contents += cont.generate_backend_methods(type)
      fg.contents += f"}};\n"
      pass
   pass

def generate_database_header(path, types):
   with FileGen(path, "database", ".h", True) as fg:
      fg.contents += f'#pragma once\n\n'
      for type in types:
         fg.contents += f'#include "{type.name}.h"\n'

def generate_traits_types(path):
   with FileGen(path, "database_traits_types", ".h", True) as fg:
      fg.contents += f'''
#pragma once

using eObjId = typename cDbTraits::eObjId;
using ePropId = typename cDbTraits::ePropId;
using eRelId = typename cDbTraits::eRelId;

using cObject = db::cObject<cDbTraits>;
using cRelationship = db::cRelationship<cDbTraits>;
using cIntrospector = db::cIntrospector<cDbTraits>;
using ePropertyType = typename cIntrospector::ePropertyType;
using eRelationshipType = typename cIntrospector::eRelationshipType;
using cDatabase = db::cDatabase<cDbTraits>;

'''

types_dict = {}
def generate_traits_header(path, types):
   for type in types:
      type.id = f"{type.name}"
      types_dict[type.id] = type
   for type in types:
      for item in type.contents:
         if item.is_rel:
            if not hasattr(item, "parent_ref"):
               item.parent = type.name
               item.name = type.name + "_" + item.to
               item.id = item.name
               item.parent_ref = False
               child_part = Rel(item.type, type.name);
               child_part.parent = item.to
               child_part.name = item.name
               child_part.id = child_part.name
               child_part.parent_ref = True
               types_dict[item.to].contents.append(child_part)
         else:
            item.id = f"{type.name}_{item.name}"
            item.data_type = item.type
            if item.data_type == "string":
               item.data_type = "std::string"
   with FileGen(path, "database_traits", ".h", True) as fg:
      fg.contents += f'''
#include "pch.h"

#include "db_vector.h"
#include "db_object.h"
#include "db_relationship.h"
#include "db_introspector.h"
#include "db_database.h"

struct cDbTraits {{
   template <typename T>
#if 1
   using alloc = std::allocator<T>;
#else
   using alloc = shm::alloc<T>;
#endif
   enum class eObjId {{
'''
      for type in types:
         fg.contents += f"{type.id},"

      fg.contents += f"_count}};enum class ePropId {{"

      for type in types:
         for item in type.contents:
            if not item.is_rel:
               fg.contents += f"{item.id},"

      fg.contents += f"_count}};enum class eRelId {{"

      for type in types:
         for item in type.contents:
            if item.is_rel and not item.parent_ref:
               fg.contents += f"{item.id},"

      fg.contents += f"_count}};static db::cIntrospector<cDbTraits> introspector;"
      fg.contents += f"}};\n\n"

def generate_traits_source(path, types):
   with FileGen(path, "database_traits", ".cpp", True) as fg:
      fg.contents += f'''
#include "pch.h"

#include "database.h"

using namespace std;

#include "database_traits_types.h"

#define OBJ_DESC(id) cIntrospector::cObjDesc{{#id, eObjId::##id, &cObject::factory<c##id>}}

#define PROP_DESC(id, type, proptype) cIntrospector::cPropDesc{{#id, ePropId::##type##_##id, ePropertyType::proptype, (cIntrospector::cPropValuePtr)&c##type##::m_##id}}

#define REL_DESC(id, type, parent, child) cIntrospector::cRelDesc{{#id, eRelationshipType::type, eRelId::##id, eObjId::parent, eObjId::child}}

cIntrospector cDbTraits::introspector = {{{{
'''
      for type in types:
         fg.contents += f'OBJ_DESC({type.name}),\n'
      fg.contents += f"""}},{{
"""

      for type in types:
         for item in type.contents:
            if not item.is_rel:
               fg.contents += f'PROP_DESC({item.name}, {type.name}, {item.type}),\n'

      fg.contents += f"""}},{{
"""
      for type in types:
         for item in type.contents:
            if item.is_rel:
               fg.contents += f'REL_DESC({item.name}, {item.type}, {item.parent}, {item.to}),\n'

      fg.contents += f"""}}
      }};

#undef REL_DESC
#undef PROP_DESC
#undef OBJ_DESC

"""
   pass

def generate(out_dir, types, enums):
   generate_traits_header(out_dir, types)
   generate_traits_types(out_dir)
   generate_traits_source(out_dir, types)
   generate_database_header(out_dir, types)
   for type in types:
      generate_backend_header(out_dir, type)
   pass
