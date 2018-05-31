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
   string & getName(){return poolName_;};
   friend ostream& operator<<(ostream& os, const LocalPool& data);

private:
   LocalPool() { };

private:
   string poolName_;
   std::vector<string> locaux_;
};


class Prof
{
public:
   Prof(json jsonCour);

   string getName();
   bool isAvalable(pair<int, int> jourPeriode);
   Prof* getReference() { return this;};
   void display();

private:
   Prof() { };

public:
   string name_;
   std::map<std::pair<int, int>, bool> horaire_;
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
   virtual void display();
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
   void display();
   LocalPool* getLocalPool() { return NULL;};
};


class Configuration
{
public:
   Configuration() { };
   Configuration(const char fileName[]);

   void display();
   std::vector<Cour>      & getCours(){return cours_;};
   std::vector<Prof>      & getProf(){return prof_;};
   std::vector<LocalPool> & getLocal(){return localPool_;};
   CourVide               & getCourVide(){return coursVide_;};

private:
   void readConfiguration(const char fileName[]);
   void initialize();

private:
   json myConfig_;
   std::vector<Cour> cours_;
   std::vector<Prof> prof_;
   std::vector<LocalPool> localPool_;
   CourVide coursVide_;
};


using CourList = std::vector<Cour*>;
using PeriodeList = std::vector<CourList>;
using FrequenceCours = std::unordered_map<Cour *, int>;

struct JourCombinaison
{
   FrequenceCours frequenceDesCours;
   std::unordered_map<int /*Periode*/, CourList> combinaison;
};

using JourList = std::vector<JourCombinaison>;



class Periode
{
public:
   Periode();

   void CreeCours(Configuration *config, int jour, int periode);
   friend ostream& operator<<(ostream& os, const Periode& data);
   PeriodeList& getAllCombinaison(){return allCombinaison_;};

private:
   void recursiveCreeCoursPourUnePeriode(std::vector<Cour>::iterator inCour, CourList combinaisonCours);

private:
   int jour_;
   int periode_;
   Configuration *config_;
   PeriodeList allCombinaison_;
};



class Jour
{
public:
   Jour();

   void Cree(Configuration *config, int jour);
   void displayCombinaison();
   JourList& getAllCombinaison(){return combinaison_;};

private:
   void recursiveCree(int level,
                      FrequenceCours frequenceDuCour,
                      std::unordered_map<int, std::vector<Cour*>> uneCombinaison);
   void go1levelDeep(int periodeNumber,
                     std::vector<Cour *>,
                     FrequenceCours frequenceDuCour,
                     std::unordered_map<int, std::vector<Cour*>> uneCombinaison);

private:
   int jour_;
   Configuration *config_;
   std::map<int, Periode> periode_;
   JourList combinaison_;
};


class Semaine
{
public:
   Semaine();

   void Cree(Configuration *config);
   void displayAllHoraire(void);
   std::size_t getNombreHoraires(void) {return combinaison_.size();};
   void genererHtml(void);

private:
   void recursiveCree(int jourIdx,
                      FrequenceCours frequenceDuCour,
                      std::unordered_map < int, JourCombinaison *> uneCombinaison);
   void go1levelDeep(int jourIdx,
                     JourCombinaison &jourCombinaison,
                     FrequenceCours frequenceDuCour,
                     std::unordered_map < int, JourCombinaison *> uneCombinaison);

private:
   Configuration *config_;
   std::vector<std::unordered_map < int, JourCombinaison *>> combinaison_;
   std::map<int, Jour> jour_;
};


void display(CourList in);
void display(PeriodeList in);
void display(FrequenceCours in);
void display(JourCombinaison in);
void display(JourList in);
void display(std::unordered_map<int /*Periode*/, CourList> in);
void display(std::vector<std::unordered_map < int, JourCombinaison *>> in);
void display(std::unordered_map < int, JourCombinaison *> in);


