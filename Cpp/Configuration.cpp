#include "Configuration.hpp"


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

   for (auto &unProf : myConfig_["professeur"])
      prof_.push_back(Prof(unProf));

   for (auto &unlocal : myConfig_["local"])
      localPool_.push_back(LocalPool(unlocal));

   for (auto &unCour : myConfig_["cour"])
      cours_.push_back(Cour(unCour, prof_, localPool_));
}


void Configuration::display()
{
   for (auto& it : prof_)
      it.display();

   for (auto& it : localPool_)
      cout << it;

   for (auto& it : cours_)
      it.display();
}

std::vector<Cour> & Configuration::getCours()
{
   return cours_;
}

std::vector<Prof> & Configuration::getProf()
{
   return prof_;
}

std::vector<LocalPool> & Configuration::getLocal()
{
   return localPool_;
}

CourVide & Configuration::getCourVide()
{
   return coursVide_;
}


