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

#prop types
point = 'point'
string = 'string'
integer = 'integer'
boolean = 'boolean'
real = 'real'
coord = 'coord'

types_dict = {}
enums_dict = {}

class Prop:
   def __init__(self, name, type):
      self.name = name
      self.type = type
      self.is_rel = False
      pass

   def generate_backend_includes(self, parent):
      text = ""
      if self.type == string:
         text += f'#include "db_string.h"\n'
      if self.type in enums_dict:
         text += f'#include "{self.type}.h"\n'
      return text

   def generate_backend_data(self, parent):
      text = f"// property {self.name}\n"
      if self.type in enums_dict:
         text += f"e{self.data_type}::value m_{self.name};\n"
      else:
         text += f"{self.data_type} m_{self.name};\n"
      return text

   def generate_backend_methods(self, parent):
      text = f"// property {self.name}\n"
      text += f"{self.o_data_type} get{self.name}() const{{return m_{self.name}{self.o_data_method};}}\n";
      text += f"""void set{self.name}({self.i_data_type}{self.i_data_method} val){{if (m_{self.name}!=val) {{
         before_propmodify(cDbTraits::ePropId::{self.id}, m_{self.name});m_{self.name}=val;
         after_propmodify(cDbTraits::ePropId::{self.id}, m_{self.name});}}}}\n\n"""
      return text

class Rel:
   def __init__(self, type, to, name = '', parent_name = '', parent_type = ''):
      self.type = type
      self.to = to
      self.name = name
      self.parent_name = parent_name
      self.parent_type = parent_type
      self.is_rel = True
      self.many = False
      self.to_many = False
      pass

   def key(self):
      return self.id + str(self.parent_ref)

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
      text = f"// relationship ({self.type}) {self.parent_type}->{self.name}\n";
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
         text += f"void includeParent{self.parent_name}(c{self.parent}& x){{include(cDbTraits::eRelId::{self.id}, (db::cObject<cDbTraits>&)x, true);}}\n";
         if self.to_many:
            text += f"void exclude{self.parent}(c{self.to}& x){{exclude(cDbTraits::eRelId::{self.id}, x, true);}}\n";
            text += f"auto {self.to}s(){{return db::cRelIterRange<cDbTraits, c{parent.name}, c{self.to}>(this, cDbTraits::eRelId::{self.id});}}\n";
            text += f"auto {self.to}s() const{{return db::cRelIterConstRange<cDbTraits, c{parent.name}, c{self.to}>(this, cDbTraits::eRelId::{self.id});}}\n";
         else:
            text += f"c{self.parent_type}* parent{self.parent_name}() const {{return (c{self.parent_type}*)&*parent(cDbTraits::eRelId::{self.id})\n;}}\n";
         prefix = ''
         if self.type == 'One2Many':
            prefix = 'Parent'
         text += f"size_t count{prefix}{self.parent_name}s() const{{return count(cDbTraits::eRelId::{self.id}, true);}}\n\n";
      return text

class Type:
   def __init__(self, name, contents, shape=False, plane=False):
      self.id = name
      self.name = name
      self.shape = shape
      self.plane = plane
      self.contents = contents
      pass

class Enum:
   def __init__(self, name, values):
      self.name = name
      self.values = values

def process_types(types, enums):
   for enum in enums:
      enums_dict[enum.name] = enum
   for type in types:
      types_dict[type.name] = type
   for type in types:
      for item in type.contents:
         if not item.is_rel:
            # property
            if not hasattr(item, 'id'):
               item.id = f'{type.name}_{item.name}'
            item.data_type = item.type
            if item.data_type in enums_dict:
               item.i_data_type = f'e{item.data_type}::value'
               item.o_data_type = item.i_data_type
               item.i_data_method = ''
               item.o_data_method = item.i_data_method
            elif item.data_type == integer:
               item.data_type = 'int'
               item.i_data_type = item.data_type
               item.o_data_type = item.data_type
               item.i_data_method = ''
               item.o_data_method = ''
            elif item.data_type == boolean:
               item.data_type = 'bool'
               item.i_data_type = item.data_type
               item.o_data_type = item.data_type
               item.i_data_method = ''
               item.o_data_method = ''
            elif item.data_type == real:
               item.data_type = 'double'
               item.i_data_type = item.data_type
               item.o_data_type = item.data_type
               item.i_data_method = ''
               item.o_data_method = ''
            elif item.data_type == coord:
               item.data_type = 'geom::coord_t'
               item.i_data_type = item.data_type
               item.o_data_type = item.data_type
               item.i_data_method = ''
               item.o_data_method = ''
            elif item.data_type == string:
               item.data_type = 'db::string<char, cDbTraits>'
               item.i_data_type = 'const char *'
               item.o_data_type = 'const char *'
               item.i_data_method = ''
               item.o_data_method = '.c_str()'
            elif item.data_type == point:
               item.data_type = 'geom::cPoint'
               item.i_data_type = 'const geom::cPoint'
               item.o_data_type = 'const geom::cPoint'
               item.i_data_method = '&'
               item.o_data_method = ''
            else:
               item.i_data_type = item.data_type
               item.o_data_type = 'const ' + item.data_type
               item.i_data_method = '&'
               item.o_data_method = ''
            continue
         # relationship
         if not hasattr(item, "parent_ref"):
            item.parent_ref = False;
            item.parent = type.name
            if item.parent_type == '':
               item.parent_type = type.name
            if item.parent_name == '':
               if item.name != '':
                  item.parent_name = item.name + type.name
               else:
                  item.parent_name = item.parent_type
            if item.name == '':
               item.name = item.to
            if not hasattr(item, "id"):
               item.id = f"{item.parent_type}_{item.name}"
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
            child_part.id = item.parent_type + "_" + item.name
            child_part.parent = type.name
            child_part.parent_name = item.parent_name
            child_part.parent_type = item.parent_type

            item_to_contents = types_dict[item.to].contents
            keys = {i.key() for i in item_to_contents if i.is_rel and hasattr(i, 'id')}
            if not child_part.key() in keys:
               item_to_contents.append(child_part)

class FileGen:
   def __enter__(self):
      return self

   def __init__(self, directory, class_name, extension, overwrite = True):
      self.directory = Path(directory.replace('"', "")) / "generated"
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

def generate_backend_type_header(path, type):
   with FileGen(path, type.name, ".h", True) as fg:
      fg.contents += f'#include "database_traits.h"\n\n'
      for item in type.contents:
         fg.contents += item.generate_backend_includes(type)

      fg.contents += f"""

      class c{type.name}: public db::cObject<cDbTraits>{{public:

"""
      if type.shape:
         fg.contents += f"typename cDbTraits::cShapePtr m_shape = nullptr;\n"

      if type.plane:
         fg.contents += f"typename cDbTraits::cPlanePtr m_plane = nullptr;\n"

      for item in type.contents:
         fg.contents += item.generate_backend_data(type)

      fg.contents += f"""
         public:
            c{type.name}(typename cDbTraits::uid_t uid): cObject(cDbTraits::eObjId::{type.id}, uid){{}}
            ~c{type.name}(){{}}

"""
      if type.shape:
         fg.contents += f"cGeomImplBase* getShape() {{return &*m_shape;}}\n"
         fg.contents += f"void setShape(cGeomImplBase* shape) {{m_shape = shape;}}\n\n"

      if type.plane:
         fg.contents += f"cPlaneBase* getPlane() const {{return &*m_plane;}}\n"
         fg.contents += f"void setPlane(cPlaneBase* plane) {{m_plane = plane;}}\n\n"

      for item in type.contents:
         fg.contents += item.generate_backend_methods(type)
      fg.contents += f"}};\n"
      pass
   pass

def generate_backend_enum_header(path, enum):
   with FileGen(path, enum.name, ".h", True) as fg:

      fg.contents += f"""

      struct e{enum.name}{{enum value{{"""
      values = ''
      for item in enum.values:
         if values != '': values += ','
         values += item
      fg.contents += values

      fg.contents += f"}};static const size_t count = {len(enum.values)};static inline const char * const string[] = {{"
      values = ''
      for item in enum.values:
         values += '"' + item + '",'
      fg.contents += values
      fg.contents += f"nullptr}};}};\n"
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
         fg.contents += f'auto create{type.name}(){{return (c{type.name}*)&*create(eObjId::{type.name});}}\n'
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

def generate_traits_header(path, types, export_sym):
   with FileGen(path, "database_traits", ".h", True) as fg:
      fg.contents += f'''
#include "shared_memory.h"

#include "db_database.h"

'''
      export = ''
      if export_sym != '':
         export = f'{export_sym}_API'
         fg.contents += f'''
#include "symbol_export.h"

#ifdef {export_sym}_EXPORTS
   #define {export_sym}_API SYMBOL_EXPORT
#else
   #define {export_sym}_API SYMBOL_IMPORT
#endif

'''
      fg.contents += f'''
#pragma warning(disable: 4251)
struct {export} cDbTraits {{
   template <typename T>
#if 0
   using alloc = std::allocator<T>;
#else
   using alloc = shm::alloc<T>;
#endif

   template <typename T>
   using alloc_traits = std::allocator_traits<alloc<T>>;

   using uid_t = int;
   using cShapePtr = typename alloc_traits<cGeomImplBase>::pointer;
   using cPlanePtr = typename alloc_traits<cPlaneBase>::pointer;

   enum class eObjId: uint16_t {{Object,
'''
      for type in types:
         fg.contents += f"{type.id},"

      fg.contents += f"_count}};enum class ePropId: uint16_t {{"

      for type in types:
         for item in type.contents:
            if not item.is_rel:
               fg.contents += f"{item.id},"

      fg.contents += f"_count}};enum class eRelId: uint16_t {{"

      rel_ids = set()
      for type in types:
         for item in type.contents:
            if item.is_rel and not item.parent_ref and not item.id in rel_ids:
               fg.contents += f"{item.id},"
               rel_ids.add(item.id)

      fg.contents += f"_count}};static int s_objcount;using cIntrospector=db::cIntrospector<cDbTraits>;static cIntrospector introspector;"
      fg.contents += f"}};\n\n"

def generate_traits_source(path, types):
   with FileGen(path, "database_traits", ".cpp", True) as fg:
      fg.contents += f'''
#include "pch.h"

#include "database.h"

using namespace std;

#include "database_traits_types.h"

#define OBJ_DESC(id) cIntrospector::cObjDesc{{#id, eObjId::##id, &cObject::construct<c##id>, &cObject::destruct<c##id>, &cObject::page_factory<c##id>, &cObject::page_disposer<c##id>}}

#define PROP_DESC(id, type, proptype) cIntrospector::cPropDesc{{#id, ePropId::##type##_##id, ePropertyType::proptype, (intptr_t)&((c##type##*)0)->m_##id}}

#define REL_DESC(id, type, parent, child) cIntrospector::cRelDesc{{#id, eRelationshipType::type, eRelId::##id, eObjId::parent, eObjId::child}}

int cDbTraits::s_objcount = 0;

cIntrospector cDbTraits::introspector = {{{{
'''
      for type in types:
         fg.contents += f'OBJ_DESC({type.name}),\n'
      fg.contents += f"""}},{{
"""

      for type in types:
         for item in type.contents:
            if not item.is_rel:
               value_type = item.type
               if item.name in enums_dict:
                  value_type = 'integer'
               fg.contents += f'PROP_DESC({item.name}, {type.name}, {value_type}),\n'

      fg.contents += f"""}},{{
"""
      rel_ids = set()
      for type in types:
         for item in type.contents:
            if item.is_rel and not item.parent_ref and not item.id in rel_ids:
               fg.contents += f'REL_DESC({item.id}, {item.type}, {item.parent_type}, {item.to}),\n'
               rel_ids.add(item.id)

      fg.contents += f"""}}
      }};

#undef REL_DESC
#undef PROP_DESC
#undef OBJ_DESC

"""
   pass

def generate(out_dir, types, enums, export_sym = ''):
   process_types(types, enums)
   generate_traits_header(out_dir, types, export_sym)
   generate_traits_types(out_dir)
   generate_traits_source(out_dir, types)
   generate_database_header(out_dir, types)
   for type in types:
      generate_backend_type_header(out_dir, type)
   pass
   for enum in enums:
      generate_backend_enum_header(out_dir, enum)
   pass
