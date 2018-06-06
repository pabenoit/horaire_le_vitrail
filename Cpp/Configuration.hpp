#ifndef INC_Configuration_H
#define INC_Configuration_H

#include "Cour.hpp"

class Configuration {
public:
   Configuration() { };
   Configuration(const char fileName[]);

   void display();
   std::vector<Cour>      & getCours();
   std::vector<Prof>      & getProf();
   std::vector<LocalPool> & getLocal();
   CourVide               & getCourVide();

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

#endif
