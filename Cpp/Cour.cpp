#include "Cour.hpp"

#define MAXDAY 3
#define MAXPERIOD 6



Cour::Cour(json jsonCour, std::vector<Prof>& prof, std::vector<LocalPool>& localPool)
{
   name_ = jsonCour["titre"];
   nombrePeriodeParCycle_ = jsonCour["nombre_periode_par_cycle"];
   local_ = jsonCour["local"];

   // link to the professeur list
   prof_ = NULL;
   for (auto& it : prof)
   {
      if (it.getName() == jsonCour["prof"])
      {
         prof_ = it.getReference();
      }
   }

   // link to the room list
   localPool_ = NULL;
   for (auto& it : localPool)
   {
      if (it.isLocalInThisPool(jsonCour["local"]))
      {
         localPool_ = it.getReference();
      }
   }

   for (auto jour = jsonCour["jour_disponible"].begin(); jour != jsonCour["jour_disponible"].end(); jour++)
   {
      json& val = jour.value();
      const std::string& key = jour.key();

      for (int periodeIdx = 0; periodeIdx < val.size(); periodeIdx++)
      {
         horaire_[std::make_pair(std::stoi(key), periodeIdx)] = val[periodeIdx];
      }
   }
}

string Cour::getName()
{
   return name_;
}

bool Cour::isAvalable(pair<int, int> jourPeriode)
{
   return horaire_[jourPeriode];
}

void Cour::display()
{
   cout << "Cour: " << name_ << endl;

   cout << "  Nombre: " << nombrePeriodeParCycle_ << endl;
   cout << "  Local: ";
   if (localPool_)   cout << localPool_->getName() << " : ";
   cout << local_ << endl;
   cout << "  Prof: " << prof_->getName() << endl;

   for (int day = 1; day <= MAXDAY; day++)
   {
      for (int period = 0; period < MAXPERIOD; period++)
      {
         cout << (horaire_[std::make_pair(day, period)] ? "x" : "-") << ",";
      }
      cout << endl;
   }
   cout << endl;
}

void CourVide::display()
{
   cout << "Cour: " << name_ << endl;
}

Cour* Cour::getReference() 
{ 
   return this;
}

LocalPool* Cour::getLocalPool() 
{ 
   return localPool_;
}



CourVide::CourVide() 
{ 
   name_ = "Vide"; 
   nombrePeriodeParCycle_ = 99999999;
}

bool CourVide::isAvalable(pair<int, int> jourPeriode) 
{ 
   return true;
}

LocalPool* CourVide::getLocalPool() 
{ 
   return NULL;
}


