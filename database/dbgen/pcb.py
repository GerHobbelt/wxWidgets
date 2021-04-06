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
many2many = 'Many2Many'

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
   def __init__(self, type, to, name = '', parent_name = ''):
      self.type = type
      self.to = to
      self.name = name
      self.parent_name = parent_name
      self.is_rel = True
      self.many = False
      self.to_many = False
      pass

   def generate_backend_includes(self, parent):
      if (not self.parent_ref):
         retval = f'#include "{self.to}.h"\n'
      else:
         retval = f'class c{self.parent};'
      if not hasattr(parent, 'generated_forwards'):
         parent.generated_forwards = set()
      if not retval in parent.generated_forwards:
         parent.generated_forwards.add(retval)
         return retval
      return ''


   def generate_backend_data(self, parent):
      return ""

   def generate_backend_methods(self, parent):
      text = f"// relationship ({self.type}) {self.parent}->{self.name}\n";
      if (not self.parent_ref):
         text += f"void include{self.name}(c{self.to}& x){{include(cDbTraits::eRelId::{self.id}, x);}}\n";
         if self.to_many:
            text += f"void exclude{self.name}(c{self.to}& x){{exclude(cDbTraits::eRelId::{self.id}, x);}}\n";
            text += f"auto {self.to}s(){{return db::cRelIterRange<cDbTraits, c{parent.name}, c{self.to}>(this, cDbTraits::eRelId::{self.id});}}\n";
            text += f"auto {self.to}s() const{{return db::cRelIterConstRange<cDbTraits, c{parent.name}, c{self.to}>(this, cDbTraits::eRelId::{self.id});}}\n";
         else:
            text += f"void remove{self.name}(){{exclude(cDbTraits::eRelId::{self.id});}}\n";
         text += f"size_t count{self.name}s() const{{return count(cDbTraits::eRelId::{self.id});}}\n\n";
      else:
         text += f"void include{self.parent_name}(c{self.parent}& x){{include(cDbTraits::eRelId::{self.id}, (db::cObject<cDbTraits>&)x);}}\n";
         if self.to_many:
            text += f"void exclude{self.parent}(c{self.to}& x){{exclude(cDbTraits::eRelId::{self.id}, x);}}\n";
            text += f"auto {self.to}s(){{return db::cRelIterRange<cDbTraits, c{parent.name}, c{self.to}>(this, cDbTraits::eRelId::{self.id});}}\n";
            text += f"auto {self.to}s() const{{return db::cRelIterConstRange<cDbTraits, c{parent.name}, c{self.to}>(this, cDbTraits::eRelId::{self.id});}}\n";
         else:
            text += f"c{self.parent}* parent{self.parent_name}() const {{return (c{self.parent}*)parent(cDbTraits::eRelId::{self.id});}}\n";
         text += f"size_t count{self.parent_name}s() const{{return count(cDbTraits::eRelId::{self.id});}}\n\n";
      return text

class Type:
   def __init__(self, name, contents, shape=False):
      self.id = name
      self.name = name
      self.shape = shape
      self.contents = contents
      pass

class Enum:
   def __init__(self, name, values):
      self.name = name
      self.values = values

types_dict = {}
def process_types(types, enums):
   for type in types:
      types_dict[type.name] = type
   for type in types:
      for item in type.contents:
         if not item.is_rel:
            # property
            if not hasattr(item, "id"):
               item.id = f"{type.name}_{item.name}"
            item.data_type = item.type
            if item.data_type == "string":
               item.data_type = "std::string"
            continue
         # relationship
         if not hasattr(item, "parent_ref"):
            item.parent_ref = False;
            item.parent = type.name
            if item.parent_name == '':
               if item.name != '':
                  item.parent_name = item.name + type.name
               else:
                  item.parent_name = type.name
            if item.name == '':
               item.name = item.to
            if not hasattr(item, "id"):
               item.id = f"{type.name}_{item.name}"
            child_part = Rel(item.type, item.to, item.name);
            if item.type == many2many:
               item.many = True
               item.to_many = True
               child_part.many = True
               child_part.to_many = True
            elif item.type == one2many:
               item.to_many = True
               child_part.many = True
            if child_part.name == '':
               child_part.name = child_part.to
            child_part.parent_ref = True;
            child_part.id = type.name + "_" + item.name
            child_part.parent = type.name
            child_part.parent_name = item.parent_name
            types_dict[item.to].contents.append(child_part)

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
      for item in type.contents:
         fg.contents += item.generate_backend_includes(type)

      fg.contents += f"""

      class c{type.name}: public db::cObject<cDbTraits>{{public:

"""
      for item in type.contents:
         fg.contents += item.generate_backend_data(type)

      fg.contents += f"""
         public:
            c{type.name}(): cObject(cDbTraits::eObjId::{type.id}){{}}
            ~c{type.name}(){{}}

"""
      for item in type.contents:
         fg.contents += item.generate_backend_methods(type)
      fg.contents += f"}};\n"
      pass
   pass

def generate_database_header(path, types):
   with FileGen(path, "database", ".h", True) as fg:
      for type in types:
         fg.contents += f'''
#include "{type.name}.h"

'''
      fg.contents += f'''
class cDatabase : public db::cDatabase<cDbTraits>
{{
public:

'''
      for type in types:
         fg.contents += f'auto create{type.name}(){{return (c{type.name}*)create(eObjId::{type.name});}}\n'
         fg.contents += f'auto {type.name}s(){{return typename db::cDatabase<cDbTraits>::iterator_range<c{type.name}>(this, eObjId::{type.name});}}\n'
         fg.contents += f'auto {type.name}s()const{{return typename db::cDatabase<cDbTraits>::const_iterator_range<c{type.name}>(this, eObjId::{type.name});}}\n'
      fg.contents += f'''
}};

'''

def generate_traits_types(path):
   with FileGen(path, "database_traits_types", ".h", True) as fg:
      fg.contents += f'''
using eObjId = typename cDbTraits::eObjId;
using ePropId = typename cDbTraits::ePropId;
using eRelId = typename cDbTraits::eRelId;

using cObject = db::cObject<cDbTraits>;
using cRelationship = db::cRelationship<cDbTraits>;
using cIntrospector = db::cIntrospector<cDbTraits>;
using ePropertyType = typename cIntrospector::ePropertyType;
using eRelationshipType = typename cIntrospector::eRelationshipType;

'''

def generate_traits_header(path, types):
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

   using uid_t = int;

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

      fg.contents += f"_count}};using cIntrospector=db::cIntrospector<cDbTraits>;static cIntrospector introspector;"
      fg.contents += f"}};\n\n"

def generate_traits_source(path, types):
   with FileGen(path, "database_traits", ".cpp", True) as fg:
      fg.contents += f'''
#include "pch.h"

#include "database.h"

using namespace std;

#include "database_traits_types.h"

#define OBJ_DESC(id) cIntrospector::cObjDesc{{#id, eObjId::##id, &cObject::factory<c##id>, &cObject::disposer<c##id>}}

#define PROP_DESC(id, type, proptype) cIntrospector::cPropDesc{{#id, ePropId::##type##_##id, ePropertyType::proptype, (intptr_t)&((c##type##*)0)->m_##id}}

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
            if item.is_rel and not item.parent_ref:
               fg.contents += f'REL_DESC({item.id}, {item.type}, {item.parent}, {item.to}),\n'

      fg.contents += f"""}}
      }};

#undef REL_DESC
#undef PROP_DESC
#undef OBJ_DESC

"""
   pass

def generate(out_dir, types, enums):
   process_types(types, enums)
   generate_traits_header(out_dir, types)
   generate_traits_types(out_dir)
   generate_traits_source(out_dir, types)
   generate_database_header(out_dir, types)
   for type in types:
      generate_backend_header(out_dir, type)
   pass
