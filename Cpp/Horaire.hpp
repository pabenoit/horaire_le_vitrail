#ifndef INC_horaire_H
#define INC_horaire_H


#include <vector>
#include <unordered_map>

#include <iostream>
#include <fstream>
#include "json.hpp"
#include "Configuration.hpp"

using namespace std;
using json = nlohmann::json;


using CourList = std::vector<Cour*>;
using PeriodeList = std::vector<CourList>;
using FrequenceCours = std::unordered_map<Cour*, int>;

struct JourCombinaison
{
   FrequenceCours frequenceDesCours;
   std::unordered_map<int /*Periode*/, CourList> combinaison;
};

using JourList = std::vector<JourCombinaison>;



class Periode {
public:
   Periode();

   void CreeCours(Configuration* config, int jour, int periode);
   friend ostream& operator<<(ostream& os, const Periode& data);
   PeriodeList& getAllCombinaison(){return allCombinaison_;};

private:
   void recursiveCreeCoursPourUnePeriode(std::vector<Cour>::iterator inCour, 
                                         CourList combinaisonCours);

private:
   int jour_;
   int periode_;
   Configuration* config_;
   PeriodeList allCombinaison_;
};



class Jour {
public:
   Jour();

   void Cree(Configuration* config, int jour);
   void displayCombinaison();
   JourList& getAllCombinaison();

private:
   void recursiveCree(int level,
                      FrequenceCours frequenceDuCour,
                      std::unordered_map<int, std::vector<Cour*>> uneCombinaison);
   void go1levelDeep(int periodeNumber,
                     std::vector<Cour*>,
                     FrequenceCours frequenceDuCour,
                     std::unordered_map<int, std::vector<Cour*>> uneCombinaison);

private:
   int jour_;
   Configuration* config_;
   std::map<int, Periode> periode_;
   JourList combinaison_;
};


class Semaine {
public:
   Semaine();

   void Cree(Configuration* config);
   void displayAllHoraire(void);
   std::size_t getNombreHoraires(void);
   void genererHtml(void);

private:
   void recursiveCree(int jourIdx,
                      FrequenceCours frequenceDuCour,
                      std::unordered_map < int, JourCombinaison*> uneCombinaison);
   void go1levelDeep(int jourIdx,
                     JourCombinaison &jourCombinaison,
                     FrequenceCours frequenceDuCour,
                     std::unordered_map < int, JourCombinaison*> uneCombinaison);

private:
   Configuration *config_;
   std::vector<std::unordered_map < int, JourCombinaison*>> combinaison_;
   std::map<int, Jour> jour_;
};


void display(CourList in);
void display(PeriodeList in);
void display(FrequenceCours in);
void display(JourCombinaison in);
void display(JourList in);
void display(std::unordered_map<int /*Periode*/, CourList> in);
void display(std::vector<std::unordered_map < int, JourCombinaison*>> in);
void display(std::unordered_map < int, JourCombinaison*> in);

#endif
