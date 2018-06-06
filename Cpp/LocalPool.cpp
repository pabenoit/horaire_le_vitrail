#include "LocalPool.hpp"


LocalPool::LocalPool(json jsonLocalPool)
{
   for (auto localPool = jsonLocalPool.begin(); localPool != jsonLocalPool.end(); localPool++)
   {
      poolName_ = localPool.key();
      for (auto local : localPool.value())
      {
         locaux_.push_back(local);
      }
   }
}


std::size_t LocalPool::getNombreDeLocal()
{
   return locaux_.size();
}


bool LocalPool::isLocalInThisPool(string inLocal)
{
   return (std::find(locaux_.begin(), locaux_.end(), inLocal) != locaux_.end());
}


LocalPool* LocalPool::getReference() 
{ 
   return this;
}


string & LocalPool::getName()
{
   return poolName_;
}


ostream& operator<<(ostream& os, const LocalPool& data)
{
   os << data.poolName_ << ":  ";
   for (auto& local : data.locaux_)
   {
      os << local << ", ";
   }
   os << endl;

   return os;
}


