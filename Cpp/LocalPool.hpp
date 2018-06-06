#ifndef INC_LocalPool_H
#define INC_LocalPool_H

#include <iostream>
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;



class LocalPool {
public:
   LocalPool(json jsonLocal);

   std::size_t getNombreDeLocal();
   bool isLocalInThisPool(string local);
   LocalPool* getReference();
   string & getName();
   friend ostream& operator<<(ostream& os, const LocalPool& data);

private:
   LocalPool() { };

private:
   string poolName_;
   std::vector<string> locaux_;
};



#endif
