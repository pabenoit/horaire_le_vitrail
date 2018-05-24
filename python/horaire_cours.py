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
MAX_COURS_PAR_PERIODE =  3
MAX_PERIODE_PAR_JOUR  =  5
MAX_JOURS_SEMAINE     = 10
JOUR_DE_LA_SEMAINE = ["lundi","mardi","mercredi","jeudi","vendredi"]

FICHIER_HTML_SORTIER = "horaire.html"
FICHIER_CSS_SORTIER  = "horaire.css"
FICHIER_DONNEE_ENTREE  = "donnee_test1.json"


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
    """Le but de cette class est de fournir les methodes relative a un cour"""
    
    def get_titre(self, value):
        """
        Retourne le nom du cour
        """
        return value["titre"]

    def is_disponible(self, value, jour, periode):
        """
        Retourne True si le cour peux etre donnee pour cette journee et cette periode
        """
        return value["jour_disponible"][jour][periode]

    def is_mutuelle_exclusif(self, value, autre_cour):
        """
        Verifie si le cour peut etre donne en meme temps que les autres cours
        """
        for mutuelle_exclusif in value["cour_exclusif"]:
            if mutuelle_exclusif == autre_cour:
                return False
        return True


class Periode(Cour):
    """
    Le but cette class est de generer toute les combinaisons de cours possible
    pour une periode
    """

    def __init__(self, data, jour, numero_de_la_periode):
        self.toute_combinaisons_cours_par_periode = []
        self.jour = jour
        self.numero_de_la_periode = numero_de_la_periode
        self.data = data
        
    def recursive_cree_cours_periode(self, cour, une_combinaison, toute_combinaisons_cours_par_periode):
        """Function recursive pour cree les combinations de 3 cours pour une periode donnee

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
    """
    Le but de cette class, est de generer tout les combinaison de cours pour chaque periodes de la journee
    """

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
        """
        Genere gace a la fonction recursive recursive_jours_periode() toutes les combinaisons
        de cours possible pour cette journee
        """

        self.cree_chaque_periode_de_la_journee()

            no_periode = 0
            list_des_cours_jusqua_present_aujourhui = []
            une_combinaison = []

            self.recursive_jours_periode(no_periode, list_des_cours_jusqua_present_aujourhui, 
                                         une_combinaison, self.toute_combinaisons_periode_par_journee)

    def get_list_de_toute_possible_combinaision_de_cour_de_la_journee(self):
        """
        Retourne une list de list qui contien toute les combinaison de cours pour chaque poeriodes de la journee
        Les combinaison doivent etre generer avant en faisant appel a la methode cree_list_combinaison_des_periode_pour_la_journee()
        """
        return self.toute_combinaisons_periode_par_journee

    def display(self):
        """
        Permet d afficher toute les combinaison de cours possible pour une journee.
        Les combinaison doivent etre generer avant en faisant appel a la methode cree_list_combinaison_des_periode_pour_la_journee()
        """
        print("Jour {0}".format(self.jour))
        print("---------------")
        for combinaison in self.toute_combinaisons_periode_par_journee:
            print (" ")
            for periode in combinaison:
                print ("{0} | ".format(periode[1],'^20'),end='')


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


class Semaine_std:
    """Le but de cette class est de generer toute les combinaison possibles pour une semaine (horaire)
       le mot semaine fait reference a une semaine d ecole (10 jours dans notre cas et non une semaine
       du calendrier."""

    def __init__(self, data):
        self.data = data
        self.tout_combinaison_pour_la_semaine = []
        
    def get_tout_combinaison_pour_la_semaine(self):
        """Retourne la list de toutes le combinaisons possible l horaire"""
        return self.tout_combinaison_pour_la_semaine


    def cree_semaine(self):
        maxHoraire = 3
        self.tout_combinaison_pour_la_semaine = []

        jour_lundi = Jour(self.data, "lundi")
        jour_mardi = Jour(self.data, "mardi")
        jour_mercredi = Jour(self.data, "mercredi")
        jour_jeudi = Jour(self.data, "jeudi")
        jour_vendredi = Jour(self.data, "vendredi")

        jour_lundi.cree_list_combinaison_des_periode_pour_la_journee()
        jour_mardi.cree_list_combinaison_des_periode_pour_la_journee()
        jour_mercredi.cree_list_combinaison_des_periode_pour_la_journee()
        jour_jeudi.cree_list_combinaison_des_periode_pour_la_journee()
        jour_vendredi.cree_list_combinaison_des_periode_pour_la_journee()

        cmb_jour_lundi = jour_lundi.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()
        cmb_jour_mardi = jour_mardi.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()
        cmb_jour_mercredi = jour_mercredi.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()
        cmb_jour_jeudi = jour_jeudi.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()
        cmb_jour_vendredi = jour_vendredi.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()

        for jour1_combinaison in cmb_jour_lundi:
            cours_niveau1 = [item for sublist in jour1_combinaison for item in sublist[1]]

            for jour2_combinaison in cmb_jour_mardi:
                cours_niveau2 = [item for sublist in jour2_combinaison for item in sublist[1]]
                cours_niveau2.extend(cours_niveau1)

                for jour3_combinaison in cmb_jour_mercredi:
                    cours_niveau3 = [item for sublist in jour3_combinaison for item in sublist[1]]
                    cours_niveau3.extend(cours_niveau2)

                    for jour4_combinaison in cmb_jour_jeudi:
                        updateHtmlInProgress(len(cmb_jour_lundi)*len(cmb_jour_mardi)*len(cmb_jour_mercredi))
                        cours_niveau4 = [item for sublist in jour4_combinaison for item in sublist[1]]
                        cours_niveau4.extend(cours_niveau3)

                        for jour5_combinaison in cmb_jour_vendredi:
                            cours_niveau5 = [item for sublist in jour5_combinaison for item in sublist[1]]
                            cours_niveau5.extend(cours_niveau4)

                            for jour6_combinaison in cmb_jour_lundi:
                                cours_niveau6 = [item for sublist in jour6_combinaison for item in sublist[1]]
                                cours_niveau6.extend(cours_niveau5)

                                for jour7_combinaison in cmb_jour_mardi:
                                    cours_niveau7 = [item for sublist in jour7_combinaison for item in sublist[1]]
                                    cours_niveau7.extend(cours_niveau6)

                                    for jour8_combinaison in cmb_jour_mercredi:
                                        cours_niveau8 = [item for sublist in jour8_combinaison for item in sublist[1]]
                                        cours_niveau8.extend(cours_niveau7)

                                        for jour9_combinaison in cmb_jour_jeudi:
                                            cours_niveau9 = [item for sublist in jour9_combinaison for item in sublist[1]]
                                            cours_niveau9.extend(cours_niveau8)

                                            for jour10_combinaison in cmb_jour_vendredi:
                                                cours_niveau10 = [item for sublist in jour10_combinaison for item in sublist[1]]
                                                cours_niveau10.extend(cours_niveau9)

                                                count_present_chaque_cours = Counter(cours_niveau10)
                                                # Verifie que chaque cours est donner le bon nombre de fois
                                                for un_cour in self.data["cour"]:
                                                    if count_present_chaque_cours[un_cour["titre"]] != un_cour["nombre_periode_par_cycle"]:
                                                        break;
                                                self.tout_combinaison_pour_la_semaine.append([jour1_combinaison,
                                                                 jour2_combinaison,
                                                                 jour3_combinaison,
                                                                 jour4_combinaison,
                                                                 jour5_combinaison,
                                                                 jour6_combinaison,
                                                                 jour7_combinaison,
                                                                 jour8_combinaison,
                                                                 jour9_combinaison,
                                                                 jour10_combinaison])

                                                # Genere que 10 haraire maximum
                                                maxHoraire = maxHoraire-1
                                                if maxHoraire == 0:
                                                    return
        return

    def display(self):
        for combinaison in self.tout_combinaison_pour_la_semaine:
            print (" ----------- ")
            for periode in combinaison:
                for cour in periode:
                    print ("{0} | ".format(cour[1],'^20'),end='')
                print (" ")
                
    def htmlPage(self):
        """Genere une page HTML avec des TAB qui contient tout les horaire possible."""

        self.creePageCss()
        
        file1 = open('file1.dummy','w')
        file2 = open('file2.dummy','w')
        file3 = open('file3.dummy','w')
        file4 = open('file4.dummy','w')

        file1.write("<html>\n<head>\n<link rel='stylesheet' type='text/css' href={0}>\n".format(FICHIER_CSS_SORTIER))
        file1.write("<script>\nfunction change_tab(id)\n {\n  document.getElementById('page_content').innerHTML=document.getElementById(id+'_desc').innerHTML;")
        file2.write('''   document.getElementById(id).className="selected";
 }
</script>
<style>
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

<div id="main_content">

''')
        # Genere la TAB de chaque page web
        for idx, combinaison in enumerate(self.tout_combinaison_pour_la_semaine):
            file1.write('   document.getElementById("page{}").className="notselected";\n'.format(idx+1))
            if idx == 0:
                file2.write(' <li class="selected" id="page1" onclick="change_tab(this.id);">Page1</li>\n')
            else:
                file2.write(' <li class="notselected" id="page{0}" onclick="change_tab(this.id);">Page{1}</li>\n'.format(idx+1,idx+1))
            file3.write(" <div class='hidden_desc'")
            file3.write(' id="page{0}_desc">\n  <h2>Page {0}</h2>\n<h2>HTML Table</h2>\n\n<table>\n'.format(idx+1))
            file3.write('<tr><th>Periode #</th><th>Lundi</th><th>Mardi</th><th>Mercredi</th><th>Jeudi</th><th>Vendredi</th><th>Lundi</th><th>Mardi</th><th>Mercredi</th><th>Jeudi</th><th>Vendredi</th></tr>\n')
            for noPeriode in range(0, 5):
                file3.write("  <tr>\n    <td>{0}</td>\n".format(noPeriode+1))
                for noJour in range(0, 10):
                    file3.write("    <td>")
                    for no, cour in enumerate(combinaison[noJour][noPeriode][1]):
                        if no != 0:
                            file3.write("<BR>")
                        file3.write("{0}".format(cour))
                    file3.write("</td>\n")
                file3.write("  </tr>\n")
            file3.write('</table>\n </div>\n \n')
        file4.write('</div>\n \n</body>\n</html>\n')
        file2.write('\n')

        # Genere la section TAB default de la page HTML, c est le meme que la page 1 
        combinaison = self.tout_combinaison_pour_la_semaine[0]
        file4.write(' <div id="page_content">\n')
        file4.write(' <h2>Page 1</h2>\n <h2>HTML Table</h2>\n\n<table>\n')
        file4.write('<tr><th>Periode #</th><th>Lundi</th><th>Mardi</th><th>Mercredi</th><th>Jeudi</th><th>Vendredi</th><th>Lundi</th><th>Mardi</th><th>Mercredi</th><th>Jeudi</th><th>Vendredi</th></tr>\n')
        for noPeriode in range(0, 5):
            file4.write("  <tr>\n    <td>{0}</td>\n".format(noPeriode+1))
            for noJour in range(0, 10):
                file4.write("    <td>")
                for no, cour in enumerate(combinaison[noJour][noPeriode][1]):
                    if no != 0:
                            file4.write("<BR>")
                    file4.write("{0}".format(cour))
                file4.write("</td>\n")
            file4.write("  </tr>\n")
        file4.write('</table>\n </div>\n \n')
        file4.write('</div>\n \n</body>\n</html>\n')

        file1.close()
        file2.close()
        file3.close()
        file4.close()

        # Merge les 4 fichier temporaire en une seul page WEB.
        file1 = open('file1.dummy')
        file2 = open('file2.dummy')
        file3 = open('file3.dummy')
        file4 = open('file4.dummy')
        file_html = open(FICHIER_HTML_SORTIER,'w')
        file_html.write(file1.read())
        file_html.write(file2.read())
        file_html.write(file3.read())
        file_html.write(file4.read())
        file1.close()
        file2.close()
        file3.close()
        file4.close()
        file_html.close()

        return

    def creePageCss(self):
        """Genere la pas CSS requie par la page HTMLe."""

        fichierCss = open(FICHIER_CSS_SORTIER,'w')
        fichierCss.write('''
body
{
 background-color:#2E3B0B;
 margin:0px auto;
 padding:0px;
 font-family:helvetica;
 height:2000px;
}
h1
{
 text-align:center;
 font-size:35px;
 margin-top:60px;
 color:#BEF781;
}
h1 p
{
 text-align:center;
 margin:0px;
 font-size:18px;
 text-decoration:underline;
 color:white;
}
#main_content
{
 margin-top:50px;
 width:1500px;
 margin-left:250px;
}
#main_content li
{
 display:inline;
 list-style-type:none;
 background-color:#688A08;
 padding:10px;
 border-radius:5px 5px 0px 0px;
 color:#292A0A;
 font-weight:bold;
 cursor:pointer;
}
#main_content li.notselected
{
 background-color:#688A08;
 color:#292A0A;	
}
#main_content li.selected
{
 background-color:#D0F5A9;
 color:#292A0A;	
}
#main_content .hidden_desc
{
 display:none;
 visibility:hidden;
}
#main_content #page_content
{
 background-color:#D0F5A9;
 padding:10px;
 margin-top:9px;
 border-radius:0px 5px 5px 5px;
 color:#2E2E2E;
 line-height: 1.6em;
 word-spacing:4px;
}
''')
        fichierCss.close()
        return


# --- Obsolite.
# --- Cette class a ete remplace par la class Semaine_std
# --- L'idee etais d utilise la recusiviter pour generer l horaire, mais je n ai pas eu le temps de finir.
# --- donc pour l instant l implementation sequenciel Semaine_std a ete retenue.
#
# class Semaine:
#     def __init__(self, data):
#         self.data = data
#         self.semaine = []
#         self.tout_combinaison_pour_la_semaine = []
# 
#     def cree_semaine(self):
#         for jour_de_semaine in JOUR_DE_LA_SEMAINE:
#             jour = Jour(self.data, jour_de_semaine)
#             jour.cree_list_combinaison_des_periode_pour_la_journee()
#             #print(jour.get_list_de_toute_possible_combinaision_de_cour_de_la_journee())
#             self.semaine.append(jour)
# 
#     def cree_list_combinaison_pour_une_semaine(self):
#         # Maintenant cree l arbre de tout les combinaisons possibles pour la semaine.
#         une_combinaison = []
#         list_des_cours_jusqua_present = []
#         no_jour = 0
#         self.resursif_horaire_jours_periode(no_jour, list_des_cours_jusqua_present, une_combinaison, self.tout_combinaison_pour_la_semaine)
# 
#     def get_tout_combinaison_pour_la_semaine(self):
#         return self.tout_combinaison_pour_la_semaine
# 
#     def display(self):
#         for combinaison in self.tout_combinaison_pour_la_semaine:
#             print (" ----------- ")
#             for periode in combinaison:
#                 print ("Periode:{}".format(periode))
#                 for cour in periode:
#                     print ("{0} | ".format(cour,'^20'),end='')
# 
# 
#     def resursif_horaire_jours_periode(self, no_jour, list_des_cours_jusqua_present, une_combinaison, on_garde ):
#         """Cette fonction cree toute les combinaison valide pour l horaire"""
# 
#         if no_jour < MAX_JOURS_SEMAINE:
#             jour = self.semaine[no_jour%5]
#             print ("Jour #" +str(no_jour) +jour.jour )
# 
#             # On regarde chaque periode de la journee
#             tmp = jour.get_list_de_toute_possible_combinaision_de_cour_de_la_journee()
#             for index, jour_combinaison in enumerate(tmp):
#                 print ("index {0}".format(index))
#                 pprint (jour_combinaison)
#                 # Ajoute les nouveaux cours a la list des cours qui sont donnee dans la journee
#                 for cours in jour_combinaison:
#                     list_des_cours_jusqua_present.extend(cours[1])
#     
#                 # Compte combien de fois est present chaque cours
#                 # Le format de sortie est ({u'math202': 1, u'science': 1, u'francais': 1, u'geo': 1, u'math101': 1}
#                 count_present_chaque_cours = Counter(list_des_cours_jusqua_present)
# 
#                 # Verifier que aucun cours n est donne trop de fois
#                 for un_cour in self.data["cour"]:
#                     if count_present_chaque_cours[un_cour["titre"]] > un_cour["nombre_periode_par_cycle"]:
#                         # Le cours vas etre donne trop souvent, on arrete l arbre ici
#                         return
# 
#                 # Fait une copie
#                 nouvelle_combinaison = list(une_combinaison)
#                 
#                 # Ajoute le nouveau cour a la copie
#                 nouvelle_combinaison.append((index, jour_combinaison))
#                 
#                 copy_jour = deepcopy(no_jour)
#                 copy_jour += 1
# 
# 
#                 self.resursif_horaire_jours_periode(copy_jour, list_des_cours_jusqua_present, nouvelle_combinaison, on_garde)
# 
#         # Au dernier niveau et tout les condition respecter alors on garde cette combinason gagnate.
#         on_garde.append(une_combinaison)
# 
#         return

def updateHtmlInProgress(progression):

    file_html = open(FICHIER_HTML_SORTIER, 'w')
    file_html.write("In progress: {0}%".format(progression))
    file_html.close()

    return

# MAIN
data = lire(FICHIER_DONNEE_ENTREE)

# # GENERE UNE JOURNEE
horaire = Semaine_std(data)
horaire.cree_semaine()
horaire.htmlPage()
 
# afficherProf(data)
# afficherCours(data)
# afficherPOA(data)

print ("FIN")
