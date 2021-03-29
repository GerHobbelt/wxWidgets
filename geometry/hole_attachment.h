#pragma once

struct cHoleAttachment
   : public iAttachment
{
   list<unique_ptr<iPolygon>> m_holes;

   cHoleAttachment()
   {
   }
   int id() const override
   {
      return AttachmentType_Hole;
   }
};
