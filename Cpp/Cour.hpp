#ifndef INC_Cour_H
#define INC_Cour_H

#include <iostream>
#include <utility>

#include "LocalPool.hpp"
#include "Prof.hpp"
#include "json.hpp"
using json = nlohmann::json;

using namespace std;


class Cour {
public:
   Cour() { };
   Cour(json jsonCour, std::vector<Prof>& prof, std::vector<LocalPool>& localPool);

   string getName();
   Cour* getReference();
   virtual bool isAvalable(pair<int, int> jourPeriode);
   virtual void display();
   virtual LocalPool* getLocalPool();

public:
   Prof* prof_;
   int nombrePeriodeParCycle_;

protected:
   string name_;
   std::map<std::pair<int, int>, bool> horaire_;
   LocalPool* localPool_;
   string local_;
};


class CourVide : public Cour {
public:
   CourVide();

   bool isAvalable(pair<int, int> jourPeriode);
   void display();
   LocalPool* getLocalPool();
};


#endif
