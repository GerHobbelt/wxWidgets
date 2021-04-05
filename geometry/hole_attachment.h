#pragma once

interface iGeomImpl;

struct cHoleAttachment
   : public iAttachment
{
   list<iGeomImpl*> m_holes;

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
