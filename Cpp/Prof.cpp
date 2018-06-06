#include "Prof.hpp"

#define MAXDAY 3
#define MAXPERIOD 6



Prof::Prof(json jsonProf)
{
   name_ = jsonProf["nom"];
   nombre_POA_min_ = jsonProf["nombre_POA_min"];
   nombre_POA_max_ = jsonProf["nombre_POA_max"];

   for (auto jour = jsonProf["jour_disponible"].begin(); jour != jsonProf["jour_disponible"].end(); jour++)
   {
      json& val = jour.value();
      const std::string& key = jour.key();

      for (int periodeIdx = 0; periodeIdx < val.size(); periodeIdx++)
      {
         horaire_[std::make_pair(std::stoi(key), periodeIdx)] = val[periodeIdx];
      }
   }
}

string Prof::getName()
{
   return name_;
}

bool Prof::isAvalable(pair<int, int> jourPeriode)
{
   return horaire_[jourPeriode];
}

void Prof::display()
{
   cout << "Prof: " << name_ << endl;
   cout << "  POA: " << nombre_POA_min_ << " - " << nombre_POA_max_ << endl;
   cout << "  Matiere: ";

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

Prof* Prof::getReference() 
{ 
   return this;
}


