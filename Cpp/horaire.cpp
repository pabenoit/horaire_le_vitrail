#include "horaire.hpp"

#include <stdlib.h>
// https://github.com/nlohmann/json

#define MAXDAY 6
#define MAXPERIOD 8


Cour::Cour(json jsonCour, std::vector<Prof>& prof, std::vector<LocalPool>& localPool)
{
   name_ = jsonCour["titre"];
   nombrePeriodeParCycle_ = jsonCour["nombre_periode_par_cycle"];
   local_ = jsonCour["local"];

   // link to the professeur list
   prof_ = NULL;
   for (auto& it:prof)
   {
      if (it.getName() == jsonCour["prof"])
      {
         prof_ = it.getReference();
      }
   }

   // link to the room list
   localPool_ = NULL;
   for (auto& it:localPool)
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

void Cour::Display()
{
   cout << "Cour: " << name_ << endl;

   cout << "  Nombre: " << nombrePeriodeParCycle_ << endl;
   cout << "  Local: ";
   if (localPool_)   cout << localPool_->poolName_ << " : ";
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

void CourVide::Display()
{
   cout << "Cour: " << name_ << endl;
}

////////////////////////////////////////////////////
Prof::Prof(json jsonProf)
{
   name_ = jsonProf["nom"];
   nombre_POA_min_ = jsonProf["nombre_POA_min"];
   nombre_POA_max_ = jsonProf["nombre_POA_max"];

// for (auto cour: jsonProf["cour"])
// {
//    courName_.push_back(cour);
// }

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

void Prof::Display()
{
   cout << "Prof: " << name_ << endl;
   cout << "  POA: " << nombre_POA_min_ << " - " << nombre_POA_max_ << endl;
   cout << "  Matiere: ";

// for (auto &it:courName_)
// cout << it << ", ";
// cout << endl;

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

//////////////////////////////////////////////
LocalPool::LocalPool(json jsonLocalPool)
{
   for (auto localPool = jsonLocalPool.begin(); localPool != jsonLocalPool.end(); localPool++)
   {
      poolName_ = localPool.key();
      for (auto local: localPool.value())
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

void LocalPool::Display()
{
   cout << poolName_ << ":  ";
   for (auto& local: locaux_)
   {
      cout << local << ", ";
   }
   cout << endl;
}

Configuration::Configuration(const char fileName[])
{
   readConfiguration(fileName);
   initialize();
}

void Configuration::readConfiguration(const char fileName[])
{
   ifstream file(fileName);
   if (file.fail())
   {
      exit(EXIT_FAILURE);
   }
   file >> myConfig_;
}

void Configuration::initialize()
{

   for (auto &unProf:myConfig_["professeur"])
   {
      prof_.push_back(Prof(unProf));
   }

   for (auto &unlocal:myConfig_["local"])
   {
      localPool_.push_back(LocalPool(unlocal));
   }

   for (auto &unCour:myConfig_["cour"])
   {
      cours_.push_back(Cour(unCour, prof_, localPool_));
   }
}


void Configuration::Display()
{
   for (auto& it:prof_)
   it.Display();

   for (auto& it:localPool_)
   it.Display();

   for (auto& it:cours_)
   it.Display();

}


Periode::Periode()
{
   config_ = NULL;
   jour_ = -1;
   periode_ = -1;
}


void Periode::CreeCours(Configuration * config, int jour, int periode)
{
   jour_    = jour;
   periode_ = periode;
   config_  = config;

   std::vector<Cour *> combinaisonCours;
   for (auto cour = config_->cours_.begin(); cour != config_->cours_.end(); cour++)
   {
      recursiveCreeCoursPourUnePeriode(cour, combinaisonCours);
   }

   // Ajoute le case ou aucun cour n est donne pour cette periode
   std::vector<Cour *> combinaisonPasDeCour;
   combinaisonPasDeCour.push_back(config_->coursVide_.getReference());
   allCombinaison_.push_back(combinaisonPasDeCour);
}

// Function recursive pour cree les combinations de 3 cours pour une periode donnee
void Periode::recursiveCreeCoursPourUnePeriode(std::vector<Cour>::iterator cour, std::vector<Cour *> combinaisonCours)
{
   while (cour != config_->cours_.end())
   {
      bool recursif = false;

      // Verifier si le cour peut etre donne a cette periode
      if (cour->isAvalable(std::make_pair(jour_, periode_)))
      {
         bool found = false;

         // Verifier si le professeur est disponible a cette periode
         if (cour->prof_->isAvalable(std::make_pair(jour_, periode_)))
         {
            // Verifier si le meme professeur donne les 2 cours
            for(auto it:combinaisonCours)
            {
               if (it->prof_ == cour->prof_)
               {
                  found = true;
               }
            }

            if (found == false)
            {
               // Si toutes les conditions son recpecter vas un niveau plus bas
               // C est ici que la magie recursive se fait, on recommance la meme fonction
               // pour aller un niveau plus bas dans l arbre
               combinaisonCours.push_back(cour->getReference());
               allCombinaison_.push_back(combinaisonCours);
               recursif = true;
            }
         }
      }

      cour++;

      if (recursif == false)
      {
         this->recursiveCreeCoursPourUnePeriode(cour, combinaisonCours);
      }
   }

   return;
}

void Periode::Display()
{
   cout << "(" << jour_ << "," << periode_ << ") :" << endl;

   for(auto it:allCombinaison_)
   {
      for (auto it2:it)
      {
         cout << it2->getName() << ", ";
      }
      cout << endl;
   }
   cout << endl;
}

Jour::Jour()
{
   jour_    = -1;
   config_ = NULL;
}

void Jour::Cree(Configuration *config, int jour)
{
   jour_   = jour;
   config_ = config;

   for (int periodeIdx = 0; periodeIdx < MAXPERIOD; periodeIdx++)
   {
      Periode periode;
      periode.CreeCours(config, jour, periodeIdx);
      periode_[periodeIdx] = periode;
   }

   std::unordered_map<Cour *, int> frequenceDuCour;
   std::unordered_map < int, std::vector<Cour* >> uneCombinaison;
   int periodeNumber = 0;

   recursiveCree(periodeNumber, frequenceDuCour, uneCombinaison);
}


void Jour::go1levelDeep(int periodeNumber,
                        std::vector<Cour *> chaqueCombinaison,
                        std::unordered_map<Cour *, int> frequenceDuCour,
                        std::unordered_map<int, std::vector<Cour*>> uneCombinaison)
{
   // Compte la frequence des cour presents
   for (auto cour:chaqueCombinaison)
   {
      frequenceDuCour[cour]++;
      if (frequenceDuCour[cour] > cour->nombrePeriodeParCycle_)
      {
         // Cour donne trop souvent. donc on arrete ici
         // on ne vas pas un viveau plus bas car inutile.
         return;
      }
   }

   uneCombinaison[periodeNumber] = chaqueCombinaison;

   recursiveCree(periodeNumber+1, frequenceDuCour, uneCombinaison);

}

void Jour::recursiveCree(int periodeNumber,
                         std::unordered_map<Cour *, int> frequenceDuCour,
                         std::unordered_map<int, std::vector<Cour*>> uneCombinaison)
{
   if (periodeNumber < MAXPERIOD)
   {
      for (auto chaqueCombinaison:periode_[periodeNumber].allCombinaison_)
      {
         go1levelDeep(periodeNumber,
                      chaqueCombinaison,
                      frequenceDuCour,
                      uneCombinaison);
      }
   } else
   {
      // On est au dernier niveau, il est temps de verifier si on garde ou non les combinaisons
      bool courTropSouvent = false;
      for (auto& cour: frequenceDuCour)
      {
         if (cour.second > cour.first->nombrePeriodeParCycle_)
         {
            // Failed le cour est donne trop souvent.
            courTropSouvent = true;
            break;
         }
      }

      if (courTropSouvent == false)
      {
         //std::pair < std::unordered_map<Cour *, int> , std::unordered_map<int, std::vector<Cour*>> > toKeep(frequenceDuCour, uneCombinaison);
         // On garde
         combinaison_.push_back(uneCombinaison);
      }
   }
}

void Jour::DisplayCombinaison()
{
   for (auto uneComb:combinaison_)
   {
      cout << "-------------------" << endl;

      for (int periodeNb = 0; periodeNb < MAXPERIOD; periodeNb++)
      {
         cout << "    " << periodeNb << "- ";
         for (auto cour:uneComb[periodeNb])
         {
            cout << std::left << std::setw(10) << cour->getName();
         }
         cout << endl;
      }
      cout << endl;
   }
}


Semaine::Semaine()
{
   config_ = NULL;
}

void Semaine::Cree(Configuration *config)
{
   config_ = config;

   for (int jourIdx = 0; jourIdx < MAXDAY; jourIdx++)
   {
      Jour jour;
      jour.Cree(config, jourIdx);
      jour_[jourIdx] = jour;
   }
#if 0
   std::unordered_map<Cour *, int> frequenceDuCour;
   std::unordered_map <std::unordered_map < int, std::vector<Cour* >>> uneCombinaison;
   int jourIdx = 0;

   recursiveCree(jourIdx, frequenceDuCour, uneCombinaison);
#endif
}
#if 0

void Jour::go1levelDeep(int periodeNumber,
                        std::vector<Cour *> chaqueCombinaison,
                        std::unordered_map<Cour *, int> frequenceDuCour,
                        std::unordered_map<int, std::vector<Cour*>> uneCombinaison)
{
   // Compte la frequence des cour presents
   for (auto cour:chaqueCombinaison)
   {
      frequenceDuCour[cour]++;
      if (frequenceDuCour[cour] > cour->nombrePeriodeParCycle_)
      {
         // Cour donne trop souvent. donc on arrete ici
         // on ne vas pas un viveau plus bas car inutile.
         return;
      }
   }

   uneCombinaison[periodeNumber] = chaqueCombinaison;

   recursiveCree(periodeNumber+1, frequenceDuCour, uneCombinaison);

}

void Jour::recursiveCree(int jourIdx,
                         std::unordered_map<Cour *, int> frequenceDuCour,
                         std::unordered_map<std::unordered_map<int, std::vector<Cour*>>> uneCombinaison)
{
   if (jourIdx < MAXDAY)
   {
      for (auto chaqueCombinaison:periode_[jourIdx].allCombinaison_)
      {
         go1levelDeep(jourIdx,
                      chaqueCombinaison,
                      frequenceDuCour,
                      uneCombinaison);
      }
   } else
   {
      // Verifier tout les cours
      bool isFrequenceCourOk = true;
      for (auto cour::config_->cours_)
      {
         if (cour.nombrePeriodeParCycle_ != frequenceDuCour[cour.getReference()])
         {
            isFrequenceCourOk = false;
            break;
         }
      }

      if (isFrequenceCourOk == true)
      {
         // On garde
         combinaison_.push_back(uneCombinaison);
      }
   }
}

#endif

int main(int argc, char *argv[])
{

   if (argc < 2)
   {
      cout << "Please provid Configuration file name" << endl;
   }

   class Configuration config(argv[1]);

   Semaine semaine;
   Jour jour;
   semaine.Cree(&config);
   semaine.jour_[1].DisplayCombinaison();
}
