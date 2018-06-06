#include "Horaire.hpp"

#include <stdlib.h>

// https://github.com/nlohmann/json

#define MAXDAY 3
#define MAXPERIOD 6




Periode::Periode()
{
   config_ = NULL;
   jour_ = -1;
   periode_ = -1;
}


void Periode::CreeCours(Configuration* config, int jour, int periode)
{
   jour_    = jour;
   periode_ = periode;
   config_  = config;

   std::vector<Cour*> combinaisonCours;
   for (auto cour = config_->getCours().begin(); cour != config_->getCours().end(); cour++)
   {
      recursiveCreeCoursPourUnePeriode(cour, combinaisonCours);
   }

   // Ajoute le case ou aucun cour n est donne pour cette periode
   std::vector<Cour*> combinaisonPasDeCour;
   combinaisonPasDeCour.push_back(config_->getCourVide().getReference());
   allCombinaison_.push_back(combinaisonPasDeCour);
}

// Function recursive pour cree les combinations de 3 cours pour une periode donnee
void Periode::recursiveCreeCoursPourUnePeriode(std::vector<Cour>::iterator cour, std::vector<Cour*> combinaisonCours)
{
   while (cour != config_->getCours().end())
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
            for(auto it : combinaisonCours)
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


ostream& operator<<(ostream& os, const Periode& data)
{
   os << "(" << data.jour_ << "," << data.periode_ << ") :" << endl;

   for(auto it : data.allCombinaison_)
   {
      for (auto it2 : it)
      {
         os  << it2->getName() << ", ";
      }
      os  << endl;
   }
   os << endl;

   return os;
}



Jour::Jour()
{
   jour_    = -1;
   config_ = NULL;
}

void Jour::Cree(Configuration* config, int jour)
{
   jour_   = jour;
   config_ = config;

   // Cree toute les periode et tout les combinaisons de chaque periode.
   for (int periodeIdx = 0; periodeIdx < MAXPERIOD; periodeIdx++)
   {
      periode_[periodeIdx].CreeCours(config, jour, periodeIdx);
   }

   FrequenceCours frequenceDuCour;
   std::unordered_map < int, std::vector<Cour*>> uneCombinaison;

   recursiveCree(0, frequenceDuCour, uneCombinaison);
}


void Jour::go1levelDeep(int periodeIndex,
                        std::vector<Cour*> chaqueCombinaison,
                        FrequenceCours frequenceDuCour,
                        std::unordered_map<int, CourList> uneCombinaison)
{
   // Compte la frequence des cour presents
   for (auto cour : chaqueCombinaison)
   {
      frequenceDuCour[cour]++;
      if (frequenceDuCour[cour] > cour->nombrePeriodeParCycle_)
      {
         // Cour donne trop souvent. donc on arrete ici
         // on ne vas pas un viveau plus bas car inutile.
         return;
      }
   }

   uneCombinaison[periodeIndex] = chaqueCombinaison;

   recursiveCree(periodeIndex + 1, frequenceDuCour, uneCombinaison);

}

void Jour::recursiveCree(int periodeIndex,
                         FrequenceCours frequenceDuCour,
                         std::unordered_map<int, CourList> uneCombinaison)
{
   if (periodeIndex < MAXPERIOD)
   {
      for (auto chaqueCombinaison:periode_[periodeIndex].getAllCombinaison())
      {
         go1levelDeep(periodeIndex,
                      chaqueCombinaison,
                      frequenceDuCour,
                      uneCombinaison);
      }
   } else
   {
      // On est au dernier niveau, il est temps de verifier si on garde ou non les combinaisons
      bool isCourTropSouvent = false;
      for (auto& cour : frequenceDuCour)
      {
         if (cour.second > cour.first->nombrePeriodeParCycle_)
         {
            // Failed le cour est donne trop souvent.
            isCourTropSouvent = true;
            break;
         }
      }

      if (isCourTropSouvent == false)
      {
         // On garde
         JourCombinaison onGarde = { frequenceDuCour, uneCombinaison };
         combinaison_.push_back(onGarde);
      }
   }
}

void Jour::displayCombinaison()
{
   for (auto uneComb : combinaison_)
   {
      cout << "-------------------" << endl;

      for (int periodeNb = 0; periodeNb < MAXPERIOD; periodeNb++)
      {
         cout << "    " << periodeNb << "- ";
         for (auto cour : uneComb.combinaison[periodeNb])
         {
            cout << std::left << std::setw(10) << cour->getName();
         }
         cout << endl;
      }
      cout << endl;
   }
}

JourList& Jour::getAllCombinaison()
{
   return combinaison_;
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
      jour_[jourIdx].Cree(config, jourIdx);
   }

   FrequenceCours frequenceDuCour;
   std::unordered_map<int, JourCombinaison*> uneCombinaison;

   recursiveCree(0, frequenceDuCour, uneCombinaison);
}

void Semaine::go1levelDeep(int jourIdx,
                           JourCombinaison &jourCombinaison,
                           FrequenceCours frequenceDuCour,
                           std::unordered_map<int, JourCombinaison*> uneCombinaison)
{
   // Compte la frequence des cours presents
   for (auto courFrequence : jourCombinaison.frequenceDesCours)
      frequenceDuCour[courFrequence.first] += courFrequence.second;

   uneCombinaison[jourIdx] = &jourCombinaison;

   recursiveCree(jourIdx + 1, frequenceDuCour, uneCombinaison);
}

void Semaine::recursiveCree(int jourIdx,
                            FrequenceCours frequenceDuCour,
                            std::unordered_map<int, JourCombinaison*> uneCombinaison)
{
   if (jourIdx < MAXDAY)
   {
      for (auto &jourCombinaison : jour_[jourIdx].getAllCombinaison())
      {
         go1levelDeep(jourIdx,
                      jourCombinaison,
                      frequenceDuCour,
                      uneCombinaison);

      }
   } else
   {
      static int counter=0;
      counter++;
      if (counter%100000 == 0)
      {
         cout << counter << endl;
      }

      // Verifier tout les cours
      bool isFrequenceCourOk = true;
      for (auto& cour : config_->getCours())
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
         cout << "+";
         combinaison_.push_back(uneCombinaison);
      }
   }
}

void Semaine::displayAllHoraire(void)
{
   display(combinaison_);
}

std::size_t Semaine::getNombreHoraires(void) 
{
   return combinaison_.size();
}


void Semaine::genererHtml(void)
{
    std::ofstream htmlFile("output.html");

   string header = R"(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>

/* <link rel="stylesheet" type="text/css" href="horaire_output.css"> */

body {font-family: Arial;}

/* Style the tab */
.tab {
    overflow: hidden;
    border: 1px solid #ccc;
    background-color: #f1f1f1;
}

/* Style the buttons inside the tab */
.tab button {
    background-color: inherit;
    float: left;
    border: none;
    outline: none;
    cursor: pointer;
    padding: 14px 16px;
    transition: 0.3s;
    font-size: 17px;
}

/* Change background color of buttons on hover */
.tab button:hover {
    background-color: #ddd;
}

/* Create an active/current tablink class */
.tab button.active {
    background-color: #ccc;
}

/* Style the tab content */
.tabcontent {
    display: none;
    padding: 6px 12px;
    border: 1px solid #ccc;
    border-top: none;
}

table {
    font-family: arial, sans-serif;
    border-collapse: collapse;
    width: 100%;
}

td, th {
    border: 1px solid #dddddd;
    text-align: left;
    padding: 8px;
}

tr:nth-child(even) {
    background-color: #dddddd;
}


</style>
</head>
<body>

)";


       string footer = R"(
<script>
function openHoraire(evt, horaireName) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(horaireName).style.display = "block";
    evt.currentTarget.className += " active";
}

// Get the element with id="defaultOpen" and click on it
document.getElementById("defaultOpen").click();
</script>
</body>
</html> 
)";

   htmlFile << header;

   htmlFile << endl << R"(<div class="tab">)" << endl;
   for (int index=0; index < combinaison_.size(); index++)
   {
      if (index ==0)
         htmlFile << R"(  <button class="tablinks" onclick="openHoraire(event, 'index)" << index << R"(') " id="defaultOpen">index)" << index << R"(</button> )" << endl;
      else
         htmlFile << R"(  <button class="tablinks" onclick="openHoraire(event, 'index)" << index << R"(') ">index)" << index << R"(</button> )" << endl;
   }
   htmlFile << "</div>" << endl << endl;

   int index = 0;
   for (auto unHoraire : combinaison_)
   {
      htmlFile << R"(<div id="index)" << index << R"(" class="tabcontent">)" << endl;
      htmlFile << "  <h3>" << index << "</h3>" << endl;
      htmlFile << "<table>" << endl;
      htmlFile << "  <caption>test:</caption>" << endl;

      // Entete du tableau
      htmlFile << "  <tr>" << endl;
      htmlFile << "    <td></td>" << endl;
      for (int jourIdx = 0; jourIdx < MAXDAY; jourIdx++)
         htmlFile << R"(    <th scope="col">)" << jourIdx << "</th>" << endl;
      htmlFile << "  </tr>" << endl;

      // Tableau
      for (int periodeIdx = 0; periodeIdx < MAXPERIOD; periodeIdx++)
      {
         htmlFile << "  <tr>" << endl;
         htmlFile << R"(    <th scope="row">)" << periodeIdx << R"(</th>" )" << endl;
         for (int jourIdx = 0; jourIdx < MAXDAY; jourIdx++)
         {
            htmlFile << "    <td>";
            for (auto cour : unHoraire[jourIdx]->combinaison[periodeIdx])
               htmlFile << cour->getName() << "<br>";
            htmlFile << "</td>" << endl;
         }
         htmlFile << "  </tr>" << endl;
      }

      htmlFile << "</table>" << endl;
      htmlFile << "</div>" << endl << endl;

      index++;
   }

   htmlFile << footer;
}


int main(int argc, char* argv[])
{

   if (argc < 2)
   {
      cout << "Please provid Configuration file name" << endl;
   }

   class Configuration config(argv[1]);

   Semaine semaine;
   Jour jour;
   semaine.Cree(&config);
//   semaine.displayAllHoraire();
   semaine.genererHtml();
}



void display(CourList in)
{
   for (auto cour : in)
   {
      cout << setw(10) << std::left << cour->getName() << ",";
   }
   cout << endl;
}

void display(PeriodeList in)
{
   for (auto courList : in)
   {
      display(courList);
   }
}

void display(FrequenceCours in)
{
   for (auto data : in)
   {
      cout << std::left << data.first->getName() << "(" << data.second << ") ";
   }
   cout << endl;
}

void display(JourCombinaison in)
{
//   display(in.frequenceDesCours);
   display(in.combinaison);
}

void display(JourList in)
{
   for (auto jourCombinaison : in)
   {
      display(jourCombinaison); 
   }
}

void display (std::unordered_map<int /*Periode*/, CourList> in)
{
   for (auto data : in)
   {
      cout << setw(10) << std::left << "  Periode " << setw(2) << data.first;
      display(data.second);
   }
}

void display(std::vector<std::unordered_map < int, JourCombinaison* >> in)
{
   for (auto data : in)
   {
      display(data);
   }
}

void display(std::unordered_map < int, JourCombinaison*> in)
{
   for (auto data : in)
   {
      cout << "Jour " << data.first << endl;
      display(*(data.second));
   }
}

