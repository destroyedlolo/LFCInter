/****************************************************************************\
*  LFCInter                                                                  *
*      � LFSoft 1995-96                                                      *
*                                                                            *
*  Interpr�teur C (r�duit), �crit pour qu'il soit portable...                *
*                                                                            *
*                   Voir le fichier de documentation                         *
*                                                                            *
*   .Le d�veloppement a �t� fait sur                                         *
*   - un AMIGA 4000 (68040, 14Mo de RAM, DD 120 Mo, CD-ROM x4, WB et KS 3.0) *
*   - un AMIGA 1000 (68010, 8Mo de RAM, DD 52 Mo, WB et KS 2.1)              *
*   - un MicroVAXII (16 Mo de RAM, deux RD54, sous ULTRIX 4.2)               *
*   en utilisant GCC 2.7.0 du CD Fresh Fish 10.                              *
*                                                                            *
*    La version utilis�e pour la d�monstration a �t� compil�e avec le        *
*   BORLAND C++ 4.52 car elle devait se faire sur un PC portable 386sx (j'ai *
*   pas les moyens de m'offrir un Amiga portable, sniff).                    *
*                                                                            *
 ******************************  Historique *********************************
*  02/12/1995  D�but du d�veloppement en C.                                  *
*  22/12/1995  Passage au C++ pour faciliter le traitement des cha�nes.      *
*  10/01/1996  Premi�re compilation avec le BORLAND (Certaines modifications *
*              ont �t� n�c�ssaires du fait que certaines #include n'ont pas  *
*              le m�me nom, (merci le MS-DOS), et que ce compilateur est     *
*              moins puissant que GCC). De plus, certaines fonctions ne      *
*              correspondent pas � celle de GCC, genre les constructeurs des *
*              'string'...                                                   *
*                       -----------------------------------                  *
*   01/04/1996  R��criture compl�te pour utiliser l'objet '_token' pour la   *
*               lecture des �l�ments du fichier source -> D�codage de la     *
*               syntaxe plus simple et traitement des erreurs plus portable. *
*                       -----------------------------------                  *
*   29/04/1996  Premi�re compilation sur le VAX: Modif dans                  *
*               /gnu/lib/g++-include/std.h qui pause probl�me lorsque        *
*               cstring.h est inclu (fonction const pour GCC et non const    *
*               pour les includes syst�me).                                  *
*                       -----------------------------------                  *
*   07/06/1996  Derniere compilation pour la pr�sentation avec, encore,      *
*               quelques corrections de buggues...                           *
*                                                                            *
\****************************************************************************/

#include "LFCInter.h"
#include "Token.h"
#include "LFDStack.h"
#include "LFCI_Cal.h"
#include <fstream>
#include <cstring>

#ifdef __BCPLUSPLUS__
extern unsigned _stklen = (unsigned) 40000; // Sinon la pile par d�faut ne suffit pas � la r�curcivit�
#endif

extern const char *cmpversion; // Date et heure de compilation

struct _amsg amsg;  // Messages � afficher
static std::string buff; // Source � interpr�ter

int calcligne( const char *ptr ){
/* Renvoie le num�ro de la ligne dans laquelle se trouve le pointeur
 */
    if(!ptr) return 0;

    int num=1;
    register const char *x = buff.c_str();

    while( ptr > x && *x ){
        if(*x=='\n') num++;
        x++;
    }
    return num;
}

int calchash( const char *s ){
/*  Calcul le code hash pour la cha�ne pass�e en argument.
 *  La valeur retourn�e ne peut �tre NULLE.
 */
    register short int i=0;
    register int val=0;

    for(;*s;i++,s++)
        val += *s << (i % 4);

    return val?val:1;
}

int main(const int ac, const char **av){
    int pgm;
    if(ac < 2){
        std::cerr << "LFCInter 1.0 (" << cmpversion << "): Un interpr�teur C...\n"
           "\t� LFSoft 1995-96\nSyntaxe :\n\tLFCInter [options_LFCInter] Pgm_c [options_Pgm_c]\n"
           "avec comme \"options_LFCInter\" :\n"
           "   -un     : Affiche les remarques qu'une seule fois.\n"
           "   -ncip   : Pas de remarque \"Conversion d'entier en pointeur\".\n"
           "   -ncpi   : Pas de remarque \"Conversion de pointeur en entier\".\n"
           "   -nprec  : Pas de remarque \"Perte de pr�cision possible\".\n"
           "   -tprec  : Test en temps r��l les pertes de pr�cision.\n"
           "   -fatal  : Test en temps r��l les pertes de pr�cision lors de conversion ptr->int.\n"
           "   -tnstd  : Affiche les op�rations non-standard en C normal.\n"
           "\n";
        exit(5);
    }

    for(pgm=1;pgm<ac;pgm++){
        if(av[pgm][0] != '-') break; // Ce n'est plus une option
        if(!strcmp(av[pgm],"-ncip"))
            amsg.conv_int_ptr = false;
        else if(!strcmp(av[pgm],"-ncpi"))
            amsg.conv_ptr_int = false;
        else if(!strcmp(av[pgm],"-nprec"))
            amsg.perte_precis = false;
        else if(!strcmp(av[pgm],"-tprec"))
            amsg.rt_perte_prec = true;
        else if(!strcmp(av[pgm],"-fatal"))
            amsg.rt_fatal_ptr = true;
        else if(!strcmp(av[pgm],"-un"))
            amsg.wrn_1 = true;
        else if(!strcmp(av[pgm],"-tnstd"))
            amsg.conv_nstd = true;
        else {
            std::cerr << "Option \"" << av[pgm] << "\" inconnue : Tappez \"LFCInter\" pour la liste des options reconnues.\n";
            exit(10);
        }
    }

    if(pgm==ac){
        std::cerr << "*F* Le fichier source n'a pas �t� pass� en argument.\n";
        exit(10);
    }

    DEB(std::cerr << "\n*D* Interpr�tation du fichier \"" << av[pgm] << "\".\n\n"; );

    std::ifstream src(av[pgm]);
    if(!src){
        std::cerr << "*F* Impossible d'ouvrir le fichier source\n";
        exit(10);
    }

    getline(src,buff,'\0'); // Lecture du fichier en entier (car un 0 ne peut pas se trouver dans un fichier ASCII)
    src.close();

    _token ptr(buff.c_str()); // Lecture des symboles globaux
    while(*ptr){
        std::string id,type;
        void *mem;

        if(lecdesc( ptr , id, type, 0, &mem)){
            const char tbase = type[type.length()-1]; // Type de base de cette d�finition

            if(type[0] == 'F'){ // C'est une fonction
                if( *ptr == '{' /*}*/ ){
                    ptr.saute(); // Saute le corps de la fonction
                    ajoute_symbole(&tds_gbl,id,type,ptr,mem); // Cr�ation de l'objet dans la table des symboles
                } else
                    std::cerr << "*R* Ligne " << calcligne(ptr) << ": Prototype de fonction ignor�.\n";
            } else FOREVER {    // Autre chose qu'une fonction
                _var *variable = ajoute_symbole(&tds_gbl,id,type,ptr,mem); // Cr�ation de l'objet dans la table des symboles

                if(*ptr == '='){ // Il y a une affectation imm�diate...
                    ptr++;
                    VarAffecte(variable, eval(ptr,NULL,true),ptr);
                }

                if(*ptr == ','){ // Autre d�claration
                    ptr++;
                    if(!lecdesc(ptr, id, type, tbase , &mem)){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, une d�claration �tait attendue.\n";
                        exit(5);
                    }

                    if(type[0]=='F'){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Une fonction ne peut �tre d�clar�e lors d'une d�finition multiple.\n";
                        exit(5);
                    }

                    // L'objet est ajout� � la table au d�but de la boucle
                } else if(*ptr == ';'){ // On a fini de lire cette d�claration
                    ptr++;
                    break;
                } else {
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe" << ADEB("(main:1)" << ) ".\n";
                    exit(5);
                }
            }
        } else if(*ptr == ';')
            ptr++;
        else if(*ptr){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, ';' �tait attendue. '"
                 << ptr.obj() << "' lu\n";
            exit(5);
        }
    }

    DEB(
        std::cerr << "*D* Liste des symboles globaux:\n";
        _var *smbl;
        for(smbl=tds_gbl; smbl ; smbl = smbl->succ)
            std::cerr << "*D*\t" << smbl->nom << ":" << smbl->type << "\n";
    );

    LFDynaStack<_rep> arg;
    arg.Push(_rep(ac-pgm)); // Pousse le nombre d'arguments
    arg.Push(_rep((void *)(av + pgm),'*', std::string("*C"))); // Et les arguments eux-m�mes

    _var_fonc *fmain = (_var_fonc *)trouve_symbole(tds_gbl,std::string("main"));
    if(!fmain){
        std::cerr << "*E* Votre source ne contient pas de fonction main()\n";
        exit(5);
    }

    if(fmain->type[0] != 'F'){
        std::cerr << "*E* Le symbole 'main' trouv� n'est pas une fonction.\n";
        exit(5);
    }

    if(fmain->type != "FI")
        std::cerr << "*A* Ligne " << calcligne((const char *)fmain->val) << ": main() devrait renvoyer un entier.\n";

    execfonc(fmain,arg);
}
