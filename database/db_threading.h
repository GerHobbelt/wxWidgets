#pragma once

namespace db {

using namespace std;

template <class Traits>
struct cRWLock //TBD
{
   cRWLock(cObject<Traits>* obj)
   {
   }
   ~cRWLock()
   {
   }

   void lock()
   {
   }
   void unlock()
   {
   }
   void upgrade()
   {
   }
   void downgrade()
   {
   }
};

template <class T>
struct cLockGuard
{
   cLockGuard(cRWLock<T>& lock)
   {
   }
   ~cLockGuard()
   {
   }
   void upgrade()
   {
   }
   void downgrade()
   {
   }
};

} // namespace db
