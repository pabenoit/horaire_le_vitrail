from __future__ import print_function
import json
from pprint import pprint
import sys
import logging
from collections import Counter
from copy import deepcopy

# Defenir les constantes utiliser dans le programme
# NOMBRE_DE_PRERIODE : Indique le nombre de periode dans une journee.
# NOMBRE_DE_JOURS    : Indique que l horaire est sur combien de jours.
#MAX_COURS_PAR_PERIODE =  3
#MAX_PERIODE_PAR_JOUR  =  8
#MAX_JOURS_SEMAINE     = 10
MAX_COURS_PAR_PERIODE =  3
MAX_PERIODE_PAR_JOUR  =  4
MAX_JOURS_SEMAINE     = 4
JOUR_DE_LA_SEMAINE = {"lundi","mardi","mercredi","jeudi","vendredi"}


def lire(fichier):
    """Cette fonction lit le fichier de donnee contenant toutes les informations 
       pour les cours qui doivent etre present dans l horaire, leur frequences
       et la list des professeurs qui vont les donner avec leur disponibilites.
       Voir le fichier example.json pour plus d'information
       """
    try:
        with open(fichier) as f:
            return json.load(f)
    except ValueError as e:
        print('invalid json: %s' % e)
        return None  # or: raise


def afficherProf(donnees):
    """Cette fonction affiche toute les informations relative aux professeurs"""
    print ("LES Professeurs")
    print ("---------------")
    for prof in donnees["professeur"]:
        print ("Nom: " + prof["nom"])

        for jour, dispo in prof["jour_disponible"].iteritems():
            ligne = "   " + jour.ljust(15) + " : "
            for periode in dispo:
                if periode == True:
                    ligne = ligne + "x,"
                else:
                    ligne = ligne + "-,"

            print (ligne)

        print ('POA Min=' + str(prof["nombre_POA_min"]) + ', Max = ' + str(prof["nombre_POA_max"]))
        print ("")


def afficherCours(donnees):
    """ Cette fontion affiche toute les informations relative aux cours qui doivent
        etre inclue dans l horaire."""
    print ("Les Cours")
    print ("---------")
    for cour in donnees["cour"]:
        print ("Titre                    : {}".format(cour["titre"]))
        print ("Profeseur                : {}".format(cour["prof"]))
        print ("Simultane                : {}".format(cour["simultane"]))
        print ("nombre periode par cycle : {}".format(cour["nombre_periode_par_cycle"]))

        ligne = ""
        for exclusif in cour["cour_exclusif"]:
            ligne = ligne + exclusif + ", "
        print ("  cour mutuellement exclusif: " + ligne)

        for jour, dispo in cour["jour_disponible"].iteritems():
            ligne = "   " + jour.ljust(15) + " : "
            for periode in dispo:
                if periode == True:
                    ligne = ligne + "x,"
                else:
                    ligne = ligne + "-,"

            print (ligne)

        print ("")


def afficherPOA(donnees):
    """ Cette fonction affiche tout les informations relative au POA. """
    print ("Les POA")
    print ("-------")
    print
    ligne = "Periode:      "
    for n in range(1, MAX_PERIODE_PAR_JOUR + 1):
        ligne = ligne + "  #" + str(n) + "  "
    print (ligne)
    print ("------------------------------------------------------------")

    for jour, data in donnees["POA"].iteritems():
        ligne = ""
        for periode in data:
            ligne = ligne + str(periode["min"]) + "-" + str(periode["max"]) + " , "
        print (jour.ljust(15) + ligne)


class Cour:

    def get_titre(self, value):
        return value["titre"]

    def is_disponible(self, value, jour, periode):
        return value["jour_disponible"][jour][periode]

    def is_mutuelle_exclusif(self, value, autre_cour):
        for mutuelle_exclusif in value["cour_exclusif"]:
            if mutuelle_exclusif == autre_cour:
                return False
        return True


class Periode(Cour):
    def __init__(self, data, jour, numero_de_la_periode):
        self.toute_combinaisons_cours_par_periode = []
        self.jour = jour
        self.numero_de_la_periode = numero_de_la_periode
        self.data = data
        
    def recursive_cree_cours_periode(self, cour, une_combinaison, toute_combinaisons_cours_par_periode):
        """Fonction recursive pour cree les combinaisons de 3 cours pour une periode donnee
    
        La sortie est dans toute_combinaisons_cours_par_periode sous ce format:
    
                                                <-- Une list de groupe de cours possibles
        [[u'science', u'math101', u'math202'],  <-- Un groupe de cours possibles
         [u'science', u'math101', u'francais'], <-- Un groupe de cours possibles
         [u'science', u'math101', u'geo'],      <-- Un groupe de cours possibles
         [u'science', u'math101']               <-- Un groupe de cours possibles
        ]
    
        """
    
        logging.debug("")
        logging.debug("START nombre_de_cour=" + str(len(une_combinaison)))
        # for cour_titre in une_combinaison:
        logging.debug("%s" ,une_combinaison)

        #Optimisation, si toute les classe sont pleinne, alors ca sert a rien d essayer d ajouter des cours.
        if len(une_combinaison) < MAX_COURS_PAR_PERIODE:
            for next_cour, value in enumerate(self.data["cour"][cour:]):
                next_cour = next_cour+cour
                # Verifie si respect les conditions
                # On commence en specifiant que l'on respect toute les conditions
                # puis on verifie une a une les condition et si une condition n est pas respecte
                # alors on change le flas a False
                condition_respecter = True
    
                # Condition #1 Le meme cour ne peut pas etre plus d une fois par periode
                for cour_titre in une_combinaison:
                    if cour_titre == value["titre"]:
                        condition_respecter = False
    
                # Condition #2 Verifier si le cour peut etre donne a cette periode
                if not value["jour_disponible"][self.jour][self.numero_de_la_periode]:
                    condition_respecter = False
    
                # Condition #3 Les cour mutuellement exclusif ne sont pas permit
                #intersection des mutuellement exclusif avec une combinaison doit etre vide
                if (set(value["cour_exclusif"]) & set(une_combinaison)):
                        condition_respecter = False
    
                # Si toutes les conditions son recpecter vas un niveau plus bas
                # C est ici que la magie recursive se fait, on recommance la meme fonction
                # pour aller un niveau plus bas dans l arbre
                if condition_respecter:
                    # Fait une copie
                    nouvelle_combinaison = list(une_combinaison)
                    # Ajoute le nouveau cour a la copie
                    nouvelle_combinaison.append(value["titre"])

                    copy_next_cour = deepcopy(next_cour)
                    copy_next_cour += 1
                    self.recursive_cree_cours_periode(copy_next_cour, nouvelle_combinaison,
                                  toute_combinaisons_cours_par_periode)

        # Sois au dernier niveau ou si on ne peut pas ajouter le nouveau cours alors on arrete l arbre a ce niveau
        toute_combinaisons_cours_par_periode.append(une_combinaison)
        return

    def cree_toute_les_combinaison(self):
            self.une_combinaison = []
            cour = 0;

            self.recursive_cree_cours_periode(cour, self.une_combinaison, self.toute_combinaisons_cours_par_periode)

    def get_toute_combinaisons_cours(self):
        return self.toute_combinaisons_cours_par_periode



class Jour:
    def __init__(self, data, jour):
        self.list_des_periode = []
        self.jour = jour
        self.data = data
        self.toute_combinaisons_periode_par_journee = []

    def add(self, group):
        self.list_des_periode.append(group)

    def get_list_des_periodes(self):
        return self.list_des_periode

    def cree_chaque_periode_de_la_journee(self):
        for periode in range(0, MAX_PERIODE_PAR_JOUR):
            la_periode = Periode( self.data, self.jour, periode)
            la_periode.cree_toute_les_combinaison()

            self.add(la_periode.get_toute_combinaisons_cours())


    def cree_list_combinaison_des_periode_pour_la_journee(self):

            self.cree_chaque_periode_de_la_journee()

            no_periode = 0
            list_des_cours_jusqua_present_aujourhui = []
            une_combinaison = []

            self.recursive_jours_periode(no_periode, list_des_cours_jusqua_present_aujourhui, 
                                         une_combinaison, self.toute_combinaisons_periode_par_journee)

    def get_list_de_toute_possible_combinaision_de_cour_de_la_journee(self):
        return self.toute_combinaisons_periode_par_journee


    def recursive_jours_periode(self, no_periode, list_des_cours_jusqua_present_aujourhui,
                      une_combinaison, toute_combinaisons_periode_par_journee):
        """Cette fonction cree toute les combinaisons valides pour la journee"""

        if no_periode < MAX_PERIODE_PAR_JOUR:
            une_periode = self.get_list_des_periodes()[no_periode]
            for index, une_combinaisons_cours in enumerate(une_periode):
                # ex: une_combinaisons_cours = [u'science', u'math101', u'math202']

                # Un cour ne peux pas etre donne 2 fois dans la meme journee.
                # Si l'intersection est non vide alors le cours est present plus d'une fois.
                # On passe au suivent alors

                # Une optimisation possible: Quand on as deja tout les cours, rien ne suffit de continuer a dessandre dans l abre car on vas rien obtenir de plus

                if (not(set(une_combinaisons_cours) & set(list_des_cours_jusqua_present_aujourhui))):
                    
                    # Toutes les conditions sont respectees
                    # Ajoute les nouveaux cours a la list des cours qui sont donnees dans la journee
                    nouvelle_list = list(list_des_cours_jusqua_present_aujourhui)
                    nouvelle_list.extend(une_combinaisons_cours)

                    # Fait une copie
                    nouvelle_combinaison = list(une_combinaison)

                    # Ajoute le nouveau cour a la copie
                    nouvelle_combinaison.append((no_periode, une_combinaisons_cours))
                    copy_periode = deepcopy(no_periode)
                    copy_periode += 1

                    self.recursive_jours_periode(copy_periode, nouvelle_list, 
                                  nouvelle_combinaison,  toute_combinaisons_periode_par_journee)

        # Au dernier niveau et tout les conditions respectees alors on garde cette combinaison gagnate.
        if no_periode == MAX_PERIODE_PAR_JOUR:
            toute_combinaisons_periode_par_journee.append(une_combinaison)
        return



class Semaine:
    def __init__(self, data):
        self.data = data
        self.semaine = []
        self.tout_combinaison_pour_la_semaine = []

    def cree_semaine(self):
        for jour_de_semaine in JOUR_DE_LA_SEMAINE:
            jour = Jour(self.data, jour_de_semaine)
            jour.cree_list_combinaison_des_periode_pour_la_journee()
            #print(jour.get_list_de_toute_possible_combinaision_de_cour_de_la_journee())
            self.semaine.append(jour)

    def cree_list_combinaison_pour_une_semaine(self):
        # Maintenant cree l arbre de tout les combinaisons possibles pour la semaine.
        une_combinaison = []
        list_des_cours_jusqua_present = []
        no_jour = 0
        self.resursif_horaire_jours_periode(no_jour, list_des_cours_jusqua_present, une_combinaison, self.tout_combinaison_pour_la_semaine)

    def get_tout_combinaison_pour_la_semaine(self):
        return self.tout_combinaison_pour_la_semaine


    def resursif_horaire_jours_periode(self, no_jour, list_des_cours_jusqua_present, une_combinaison, on_garde ):
        """Cette fonction cree toute les combinaison valide pour l horaire"""

        if no_jour < MAX_JOURS_SEMAINE:
            jour = self.semaine[no_jour%5]
            print ("Jour #" +str(no_jour) +jour.jour )

            # On regarde chaque periode de la journee
            for index, jour_combinaison in enumerate(jour.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()):

                pprint (jour_combinaison)
                # Ajoute les nouveaux cours a la list des cours qui sont donnee dans la journee
                for cours in jour_combinaison:
                    list_des_cours_jusqua_present.extend(cours[1])
    
                # Compte combien de fois est present chaque cours
                # Le format de sortie est ({u'math202': 1, u'science': 1, u'francais': 1, u'geo': 1, u'math101': 1}
                count_present_chaque_cours = Counter(list_des_cours_jusqua_present)

                # Verifier que aucun cours n est donne trop de fois
                for un_cour in self.data["cour"]:
                    if count_present_chaque_cours[un_cour["titre"]] > un_cour["nombre_periode_par_cycle"]:
                        print (un_cour["titre"] + " " +str(count_present_chaque_cours[un_cour["titre"]]) +">" + str(un_cour["nombre_periode_par_cycle"]))
                        pprint (count_present_chaque_cours)
                        # Le cours vas etre donne trop souvent, on arrete l arbre ici
                        return


                # Fait une copie
                nouvelle_combinaison = list(une_combinaison)
                
                # Ajoute le nouveau cour a la copie
                nouvelle_combinaison.append((index, jour_combinaison))
                
                copy_jour = deepcopy(no_jour)
                copy_jour += 1


                self.resursif_horaire_jours_periode(copy_jour, list_des_cours_jusqua_present, une_combinaison, on_garde)

        # Au dernier niveau et tout les condition respecter alors on garde cette combinason gagnate.
        print ("On garde !!!")
        on_garde.append(nouvelle_combinaison)

        return



# MAIN
data = lire('donnee _test1.json')
horaire = Semaine(data)
horaire.cree_semaine()
horaire.cree_list_combinaison_pour_une_semaine()
pprint (horaire.get_tout_combinaison_pour_la_semaine())
 
# afficherProf(data)
# afficherCours(data)
# afficherPOA(data)

print ("FIN")
