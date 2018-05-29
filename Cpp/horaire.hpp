#include <vector>
#include <unordered_map>

#include <iostream>
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;



class LocalPool
{
public:
   LocalPool(json jsonLocal);
   std::size_t getNombreDeLocal();
   bool isLocalInThisPool(string local);
   LocalPool* getReference() { return this;};
   void Display();
   string poolName_;

private:
   LocalPool() { };
   std::vector<string> locaux_;
};


class Prof
{
public:
   Prof(json jsonCour);
   string getName();
   bool isAvalable(pair<int, int> jourPeriode);
   Prof* getReference() { return this;};
   void Display();

private:
   Prof() { };

private:
   string name_;
   std::map<std::pair<int, int>, bool> horaire_;
//   std::vector<string> courName_;
   int nombre_POA_min_;
   int nombre_POA_max_;
};


class Cour
{
public:
   Cour() { };
   Cour(json jsonCour, std::vector<Prof>& prof, std::vector<LocalPool>& localPool);
   string getName();
   Cour* getReference() { return this;};
   virtual bool isAvalable(pair<int, int> jourPeriode);
   virtual void Display();
   virtual LocalPool* getLocalPool() { return localPool_;};

public:
   Prof *prof_;
   int nombrePeriodeParCycle_;

protected:
   string name_;
   std::map<std::pair<int, int>, bool> horaire_;
   LocalPool *localPool_;
   string local_;
};

class CourVide : public Cour
{
public:
   CourVide() { name_ = "Vide"; nombrePeriodeParCycle_ = 99999999;};
   bool isAvalable(pair<int, int> jourPeriode) { return true;};
   void Display();
   LocalPool* getLocalPool() { return NULL;};
};


class Configuration
{
public:
   Configuration() { };
   Configuration(const char fileName[]);
   void Display();

private:
   void readConfiguration(const char fileName[]);
   void initialize();

public:
   json myConfig_;
   std::vector<Cour> cours_;
   std::vector<Prof> prof_;
   std::vector<LocalPool> localPool_;
   CourVide coursVide_;
};

class Periode
{
public:
   Periode();
   void CreeCours(Configuration *config, int jour, int periode);
   void recursiveCreeCoursPourUnePeriode(std::vector<Cour>::iterator inCour, std::vector<Cour *> combinaisonCours);
   void Display();


private:
   int jour_;
   int periode_;
   Configuration *config_;

public:
   std::vector<std::vector<Cour*>> allCombinaison_;
};


class Jour
{
public:
   Jour();
   void Cree(Configuration *config, int jour);
   void recursiveCree(int level,
                      std::unordered_map<Cour *, int> frequenceDuCour,
                      std::unordered_map<int, std::vector<Cour*>> uneCombinaison);
   void go1levelDeep(int periodeNumber,
                     std::vector<Cour *>,
                     std::unordered_map<Cour *, int> frequenceDuCour,
                     std::unordered_map<int, std::vector<Cour*>> uneCombinaison);

   void DisplayCombinaison();


private:
   int jour_;
   Configuration *config_;
   std::map<int, Periode> periode_;
   std::vector<std::unordered_map<int, std::vector<Cour*>>> combinaison_;
   //std::vector<std::pair < std::unordered_map<Cour *, int> , std::unordered_map<int, std::vector<Cour*>>>> combinaison_;

};


class Semaine
{
public:
   Semaine();
   void Cree(Configuration *config);

private:
   Configuration *config_;

public:
   std::map<int, Jour> jour_;
};
