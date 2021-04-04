#pragma once

struct cHoleAttachment
   : public iAttachment
{
   list<shm::unique_offset_ptr<iPolygon>> m_holes;

   cHoleAttachment()
   {
   }
   ~cHoleAttachment()
   {
   }
   int id() const override
   {
      return AttachmentType_Hole;
   }
};
