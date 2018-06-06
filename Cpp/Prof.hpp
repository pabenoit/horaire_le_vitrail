#ifndef INC_Prof_H
#define INC_Prof_H

#include <iostream>
#include <string>
#include <utility>

#include "json.hpp"
using json = nlohmann::json;

using namespace std;


class Prof {
public:
   Prof(json jsonCour);

   string getName();
   bool isAvalable(pair<int, int> jourPeriode);
   Prof* getReference();
   void display();

private:
   Prof() { };

public:
   string name_;
   std::map<std::pair<int, int>, bool> horaire_;
   int nombre_POA_min_;
   int nombre_POA_max_;
};

#endif
