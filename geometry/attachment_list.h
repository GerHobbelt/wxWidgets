#pragma once

#include <ranges>

struct cAttachmentList
{
   using iAttachment = geom::iAttachment;
   using ptr_type = unique_ptr<iAttachment>;

   forward_list<ptr_type> m_attachment;

   struct same_id
   {
      int m_id;
      same_id(int id)
         : m_id(id)
      {
      }
      bool operator ()(const ptr_type& attachment)
      {
         int id = attachment->id();
         return id == m_id;
      }
   };

   iAttachment* get(int id) const
   {
      auto p = ranges::find_if(m_attachment, same_id(id));
      if (p != m_attachment.end()) {
         return p->get();
      }
      return nullptr;
   }
   bool add(iAttachment* new_attachment)
   {
      if (get(new_attachment->id())) {
         return false;
      }
      m_attachment.emplace_front(new_attachment);
      return true;
   }
   bool add(ptr_type& new_attachment)
   {
      if (get(new_attachment->id())) {
         return false;
      }
      m_attachment.push_front(move(new_attachment));
      return true;
   }
   bool remove(int id)
   {
      return !!m_attachment.remove_if(same_id(id));
   }
};
