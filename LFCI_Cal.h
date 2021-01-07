/****************************************************************************\
*              structures de d�finition des variables                        *
 ****************************************************************************
* Le type des variables est stock� dans une cha�ne avec le code suivant:     *
*  *: Pointeur                                                               *
*  C: char                                                                   *
*  I: int                                                                    *
*  V: void                                                                   *
*  F: fonction                                                               *
*                                                                            *
*    Ces types constants ne sont utilis�s que pour cr�er des variables mais  *
*   sont repr�sent�s par leur type non constant respectif (la valeur dans    *
*   la pile peut changer mais pas leur repr�sentation en m�moire...)         *
*                                                                            *
*  E: Enum (int constant)                                                    *
*  T: Tableau (Ptr constant)                                                 *
*                                                                            *
*    Parmi les valeurs de repr�sentation, il existe aussi le type 'L' pour   *
*   les valeurs lit�rales...                                                 *
*    Il existe aussi d'autres types cr��s pour fa�iliter les calculs (voir   *
*   le fichiers LFCI_cal.cxx)                                                *
*                                                                            *
*  Ex: int i -> "I"                                                          *
*      char **x -> "**C": Ptr sur un ptr de char                             *
*      int x() -> "FI" : Fonction qui retourne un int                        *
*      int x[10] -> "TI": Tableau d'entier                                   *
*      int x[10][20] -> "TTI": Tableau de tableau d'entier                   *
*      char *x[10] -> "T*I": Tableau de pointeur de caract�re                *
*      int *x() -> "F*I" : Fonction qui retourne un ptr sur un int.          *
*                                                                            *
*       - types non support�s : Il est interdit d'utiliser des parenth�ses   *
*   dans la d�claration des types:                                           *
*      void (*x)() -> "*FV" interdit : Ptr sur une fonction qui ne retourne  *
*   rien.                                                                    *
*                                                                            *
 ****************************************************************************
*   Inclure "LFDStack.h" avant ce fichier si la classe dynamique de stockage *
*   est n�c�ssaire dans le fichier qui inclus celui-ci                       *
\****************************************************************************/

#ifndef LFCI_CAL_H
#define LFCI_CAL_H

#include <stdlib.h> // pour le prototype de malloc() & free()

#ifndef LFCINTER_H
#include "LFCInter.h"
#endif

#ifndef LCI_TOKEN_H
#include "Token.h"
#endif

    /*
     *      Repr�sentation des valeurs.
     *      En plus de la valeur elle m�me, il faut aussi garder sa signature...
     */

union _repval { // Les diff�rentes valeurs qui peuvent �tre repr�sent�es
    int entier;
    char caractere;
    void *ptr;      // '*' : Valeur du pointeur,
                    // 'F' : Pointe vers la _var_fonc repr�sentant la fonction
    long literal;   // Ce devrait �tre la m�me chose qu'un entier, sauf bien s�r pour certains OS d�cadants
                    // qui continuent � �tre en 16 bits alors que le mat�riel est depuis longtemp sur 32 bits
                    // Merci Micro Sucker !
};

class _rep {
public:
    _repval val; // La valeur par elle-m�me
    char type() const { return typebase; };

        /* Les constructeurs */
    _rep(): typebase('V') {};
    _rep(const int x, const char tp='I'):typebase(tp){ val.entier = x; };
    _rep(const long x) : typebase('L'){ val.literal = x; };
    _rep(const char x) : typebase('C'){ val.caractere = x; };
    _rep(void *x, const char tb = '*', const std::string vers = "V") : info(vers),typebase(tb){ val.ptr = x; };
        // Ce constructeur est aussi utilis� pour les fonctions

    std::string info;    // Contient diff�rentes informations suivant la valeur stock�e

    bool nonnull(const char *);  // Vrai si la valeur est non-nulle
protected:
    char typebase;  // Type de base de cette valeur

#ifdef LFDYNASTACK_H
friend _rep adr( LFDynaStack<_rep> &, _token & ); // Dans LFCI_cal.cxx
#endif
};

    /*
     *  Stockage des variables
     */

struct _var {
    _var(_var *asucc, const std::string &anom, const std::string &atype, const int ah=0);
    virtual ~_var() { if(succ) delete succ; };

    virtual void operator = (_rep )=0;
    virtual _rep repval(const char *)=0;    // Retourne la valeur
    virtual void *refval()=0;   // Retourne une ref�rence sur cette valeur
protected:
    void veriftype(_rep &);
#ifdef DEBUG
public: // succ doit �tre accessible lorsque l'on veut afficher la liste des variables.
#endif
    _var *succ;     // Variable suivante

public:
    std::string nom;     // Nom du symbole
    std::string type;    // Type de ce symbole
    int h;          // Hash code pour ce symbole

    friend _var *trouve_symbole( _var *, const std::string &, int);
};

struct _var_int: _var {   // Symbole d'entier ou d'enum
    _var_int(_var *asucc, const std::string &anom, const std::string &atype, const int ah=0):
        _var(asucc, anom, atype, ah){};

    int val;

    void operator =( _rep x ){ veriftype(x); val = x.val.entier; };
    _rep repval( const char * ){ return val; };
    void *refval(){ return (void *)&val; };
};

struct _var_char: _var {   // Symbole de caract�re
    _var_char(_var *asucc, const std::string &anom, const std::string &atype, const int ah=0):
        _var(asucc, anom, atype, ah){};

    char val;

    void operator =( _rep x){ veriftype(x); val = x.val.caractere; };
    _rep repval( const char * ){ return val; };
    void *refval(){ return (void *)&val; };
};

struct _var_fonc: _var {   // Symbole de fonction. val contient l'adresse des param�tres
    _var_fonc(_var *asucc, const std::string &anom, const std::string &atype, const int ah=0, void *mem=NULL):
        _var(asucc, anom, atype, ah){ val=mem; };

    void *val;

    void operator =( _rep x);
    _rep repval(const char *);
    void *refval(){ return (void *)&val; };
};

class _resallouee {
/* Il peut �tre n�c�ssaire d'allouer des resources avec un pointeur
 * (par exemple s'il s'agit d'un tableau). Gr�ce � cette classe, les
 * allocations sont m�moris�es et seront lib�r�es lors de la destruction
 * du pointeur, m�me si sa valeur a chang�.
 */
    void *data;         // Pointeur sur les donn�es allou�es (doient avoir �t� allou�es avec malloc() ).
    _resallouee *succ;  // Donn�es suivantes...

public:
    _resallouee( _resallouee *asucc, void *adt ) : data(adt), succ(asucc) {};
    ~_resallouee() {
        if(data) free(data);
        if(succ) delete succ;
    };

    friend _var *ajoute_symbole( _var **, const std::string &, const std::string &, const char *, void *, int);
};

struct _var_ptr: _var {   // Symbole de pointeur ou d'un tableau
    _var_ptr(_var *asucc, const std::string &anom, const std::string &atype, const int ah=0, _resallouee *ares=NULL):
        _var(asucc, anom, atype, ah), res(ares){};
    ~_var_ptr() { if(res) delete res; };

    void *val;
    _resallouee *res;

    void operator =( _rep x){ veriftype(x); val =  x.val.ptr; };
    _rep repval(const char * ){ return _rep(val,'*',type.substr(1)); };
    void *refval(){ return (void *)&val; };
};

    /*
     *  Table des symboles: Chaque bloc {} re�oit une table
     */

struct _tablesmb {
    _tablesmb( _tablesmb *aparent=NULL ){ var = NULL; parent = aparent; };
    ~_tablesmb() { if(var) delete var; };

    _var *var;          // Liste des symboles
    _tablesmb *parent;  // Tds pr�cedente
};

    /*
     *  Prototypes des fonctions
     */
    /* Fonctions de LFCI_var.cxx de gestions des symboles */
extern _var *tds_gbl;
extern _var *trouve_symbole( _var *, const std::string &, int h=0 );
extern _var *trouve_symbole( _tablesmb *, const std::string &, int h=0);
extern _var *ajoute_symbole( _var **, const std::string &, const std::string &, const char *, void *res=NULL, int h=0);
extern void VarAffecte( _var *, _rep, const char *);
extern _rep RefAffecte( _rep, _rep, const char *);
extern _rep conv(const char, const _rep &, const char *);

extern bool lecdesc( _token &, std::string &, std::string &, char , void **,_tablesmb *table_locale = NULL);

#ifdef LFDYNASTACK_H

    /* Fonctions de LFCI_cal.cxx de calculs */
extern _rep eval(_token &, _tablesmb *table_locale = NULL, bool un=false);
extern _rep lectcar(const char **);

    /* Fonctions d'executions */
extern _rep execfonc( _var_fonc *fonc, LFDynaStack<_rep> &args );

    /* Lancement de l'ex�cution d'une fonction */
extern _rep lancefonc(_token &ptr, _var_fonc *fonc, _tablesmb *table_locale = NULL);
extern _rep interne(_token &ptr,_tablesmb *table_locale = NULL);
#endif

#endif

