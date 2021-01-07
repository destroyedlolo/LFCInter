/***************************************************************************\
*  LFCI_var.cxx                                                             *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Gestion des tables de symboles.                                          *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#include "LFCI_Cal.h"

    /* Détermine si une valeur est NULLE ou non */
bool _rep::nonnull(const char *ptr){
    _rep x= *this;

    if(type() == 'R') x=conv(0,x,ptr);

    switch(x.type()){
    case '*':
        return (bool)x.val.ptr;
    case 'C':
        return (bool)x.val.caractere;
    case 'I':
        return (bool)x.val.entier;
    case 'L':
        return (bool)x.val.literal;
    case 'V':
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Tentative d'evaluer un 'void'.\n";
        exit(5);
    default:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Evaluation sur <" << x.type() << "> inattendu.\n";
        exit(10);
    }
}

         /* Contructeur de _var: Si le code hash est NUL, il est recalculer */
_var::_var(_var *asucc, const std::string &anom, const std::string &atype, const int ah) :
         succ(asucc),nom(anom),type(atype), h(ah?ah:calchash(anom.c_str())){};

void _var::veriftype(_rep &x){
/* Vérifie que les types de base sont identiques */
    if(x.type() != type[0]){
        std::cerr << "*F* Opération sur une variable et une valeur de type différent ('" << x.type() << "' et '" << type[0] << ").\n";
        exit(5);
    }
}

void _var_fonc::operator = ( _rep ){
    std::cerr << "*F* Tentative d'affectation sur un symbole de fonction.\n";
    exit(5);
}

_rep _var_fonc::repval(const char *ptr){
    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Tentative d'obtenir la valeur d'une fonction.\n";
    exit(5);
}

    /* Gestions des symboles */
_var *tds_gbl = NULL; // Table des symboles globale

_var *trouve_symbole( _var *tbl, const std::string &nom, int h){
/*  Trouve un symbole dans la table passée en argument.
 *  <- tbl : Tête de la table dans laquelle doit se faire la recherche.
 *  <- nom : Nom du symbole à rechercher.
 *  <- h : éventuellement son code hash.
 *  -> NULL si le symbole ne peut être trouvé.
 */
    if(!h) h = calchash(nom.c_str());

    for(; tbl; tbl = tbl->succ)
        if(h == tbl->h) // Le code hash correspond
            if(nom == tbl->nom) // et le nom aussi
                return tbl;

    return NULL;
}

_var *trouve_symbole( _tablesmb *tbl, const std::string &nom, int h ){
/*  Trouve un symbole dans la table passée en argument et dans les tables parentes
 *  <- tbl : Tête de la table dans laquelle doit se faire la recherche.
 *  <- nom : Nom du symbole à rechercher.
 *  <- h : éventuellement son code hash.
 *  -> NULL si le symbole ne peut être trouvé.
 */
    _var *ret;

    if(!h) h = calchash(nom.c_str());

    for(; tbl; tbl = tbl->parent)
        if((ret = trouve_symbole(tbl->var,nom,h))) // Est-il dans la table courante ?
            return ret;

    return trouve_symbole(tds_gbl,nom,h);   // Derniere chance : La table globale...
}

_var *ajoute_symbole( _var **tbl, const std::string &nom, const std::string &type, const char *ptr, void *res, int h){
/* Ajoute un symbole dans la table passée en argument.
 *  <- ptr n'est utilisé que pour afficher la ligne ~ à laquelle la définition
 *  a eu lieu.
 *  -> renvoie un pointeur sur la variable créée.
 */

    if(!h) h = calchash(nom.c_str());

    if(trouve_symbole(*tbl,nom,h)){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": La variable '" << nom
             << "' est déjà définie dans ce bloc...\n";
        exit(5);
    }

    _var *ret;
    switch(type[0]){
        case 'I':
            ret = new _var_int(*tbl,nom,type,h);
            break;
        case 'C':
            ret = new _var_char(*tbl,nom,type,h);
            break;
        case 'F':
            ret = new _var_fonc(*tbl,nom,type,h,res);
            break;
        case '*':
            ret = new _var_ptr(*tbl,nom,type,h,(_resallouee *)res);
            break;
        case 'T':
            if(!res){
                std::cerr << "*F* Ligne " << calcligne(ptr) << ": Bizard, resource NULLE lors de la création d'un tableau.\n";
                exit(10);
            } else {
                    // 'T' est utilisé pour différencier les tableaux mais,
                    // en réalité, l'identifieur est un pointeur.
                std::string ntype='*' + type.substr(1);

                ret = new _var_ptr(*tbl,nom,ntype,h,(_resallouee *)res);
                ((_var_ptr *)ret)->val=((_resallouee *)res)->data; // La valeur du pointeur représentant le pointeur est stockée dans la resource 'racine'
            }
            break;
        default:
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de créer une variable de type '" << type << "'\n";
            exit(5);
         }

    if(!ret){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Pas assez de mémoire pour créer une nouvelle variable.\n";
        exit(5);
    }
         return *tbl=ret;
}

_rep conv(const char type, const _rep &val, const char *ptr){
/* Convertie la valeur 'val' en une valeur de type 'type'.
 * si 'type' est nul, seule une éventuelle déréférenciation est faite.
 */
    _rep ret;
    _rep tval;

    if(val.type() == 'R'){  // C'est une référence: Il faut la résoudre...
        switch(val.info[0]){
        case 'I':
            tval = *((int *)val.val.ptr);
            break;
        case 'C':
            tval = *((char *)val.val.ptr);
            break;
        case 'L':
            tval = *((long *)val.val.ptr);
            break;
        case '*':
            tval = _rep(*((void **)val.val.ptr),'*',val.info.substr(1));
            break;
        case 'T':
            tval = _rep(*((void **)val.val.ptr),'T',val.info.substr(1));
            break;
        }
    } else
        tval = val;

    if(!type) return tval;

    if(tval.type() == 'V' || type == 'V'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Tentative de conversion de ou vers une valeur 'void'.\n";
        exit(5);
    }

    if( tval.type() == type ) // Pas de conversion
        return tval;

    switch(type){
    case 'I':
        switch(tval.type()){
        case '*':
/**** Ne fonctionnera pas sur un system récent
            if(amsg.conv_ptr_int){
                std::cerr << "*R* Ligne " << calcligne(ptr) << ": Conversion de pointeur en entier.\n";
                if(amsg.wrn_1) amsg.conv_ptr_int = false;
            }
            ret = (int)tval.val.ptr;

            if(amsg.rt_fatal_ptr)
                if( (long)ret.val.entier != (long)tval.val.ptr ){
****/
                    std::cerr << "*F*  Ligne " << calcligne(ptr) << ": Perte de précision fatale lors d'une conversion de pointeur en entier.\n";
                    exit(20);
/***
                }
***/
            break;
/********** Inutile
        case 'F':
            if(amsg.conv_ptr_int){
                std::cerr << "*R* Ligne " << calcligne(ptr) << ": Conversion de pointeur en entier.\n";
                if(amsg.wrn_1) amsg.conv_ptr_int = false;
            }
            ret = (int)tval.val.ptr;
            break;
**********/
        case 'C':
            ret = (int)tval.val.caractere;
            break;
        case 'L':
            ret = (int)tval.val.literal;
            break;
        default: // Autre conversion
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Conversion inattendue '" << tval.type() << "'-> int.\n";
            exit(5);
        }
        break;
    case 'C':
        switch(tval.type()){
        case '*':   // Je ne sais pas si le C ANSI considère ces
    /*  case 'T':*/ // conversions comme des erreurs mais elles sont
    /*  case 'F':*/ // de toutes façons parfaitement ... ridicules.
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Conversion ptr -> char interdite.\n";
            exit(5);
        case 'I':
            if(amsg.perte_precis){
                std::cerr << "*R* Ligne " << calcligne(ptr) << ": Perte de précision possible.\n";
                if(amsg.wrn_1) amsg.perte_precis = false;
            }
            ret = (char)tval.val.entier;
            if(amsg.rt_perte_prec){
                if((int)ret.val.caractere != tval.val.entier)
                    std::cerr << "*R* Ligne " << calcligne(ptr) << ": Il y eu une perte de précision (" << tval.val.entier << " -> " << (int)ret.val.caractere << ").\n";
            }
            break;
        case 'L':
            ret =  (char)tval.val.literal;
            if(amsg.rt_perte_prec){
                if(ret.val.caractere != tval.val.literal)
                    std::cerr << "*R* Ligne " << calcligne(ptr) << ": Il y eu une perte de précision (" << tval.val.literal << " -> " << (int)ret.val.caractere << ").\n";
            }
            break;
        default: // Autre conversion
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Conversion inattendue '" << tval.type() << "'-> char.\n";
            exit(5);
        }
        break;
    case '*':
        switch(tval.type()){
        case 'T':
            ret = _rep( tval.val.ptr, '*', tval.info);
            break;
/********* Inutile
        case 'F':
            ret = (void *) tval.val.ptr;
            break;
*******/
        case 'C':
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Conversion char -> ptr interdite.\n";
            exit(5);
        case 'I':
            if(amsg.conv_int_ptr){
                std::cerr << "*R* Ligne " << calcligne(ptr) << ": Conversion d'entier en pointeur.\n";
                if(amsg.wrn_1) amsg.conv_int_ptr = false;
            }
            ret = (void *)tval.val.entier;
            break;
        case 'L':
            ret = (void *)tval.val.literal;
            break;
        default: // Autre conversion
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Conversion inattendue '" << tval.type() << "'-> pointeur.\n";
            exit(5);
        }
        break;
    default: // Ce n'est pas un des types reconnus
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Conversion incorrecte (affectation sur une constante ?).\n";
#ifdef DEBUG
        std::cerr << "*E* Conversion vers le type '" << type << "'.\n";
#endif
        exit(5);
    }

    return ret;
}

void VarAffecte( _var *var, _rep val, const char *ptr ){
/* Affecte la valeur 'val' à la variable 'var'.
 * 'ptr' ne sert qu'a afficher la ligne à laquelle se produit une erreur
 */
         if(!var) return;

         *var = conv(var->type[0], val, ptr); // Convertit la valeur dans le type de la variable
}

_rep RefAffecte( _rep sur, _rep val, const char *ptr ){
/* Affecte une valeur dans 'sur' qui est une référence...
 * 'ptr' ne sert qu'a afficher la ligne à laquelle ce produit une erreur
 */
    if(sur.type() != 'R'){
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Tentative de faire une affectation sur autre chose qu'une référence <" << sur.type() << ">.\n";
        exit(10);
    }

    val = conv(sur.info[0],val,ptr);

    switch(val.type()){
    case 'C':
        *((char *)sur.val.ptr) = val.val.caractere;
        break;
    case 'I':
        *((int *)sur.val.ptr) = val.val.entier;
        break;
    case '*':
        *((void **)sur.val.ptr) = val.val.ptr;
        break;
    default:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Affectation inattendu sur <" << val.type() << "> (affectation sur une constante ?).\n";
        exit(5);
    }

    return val;
}
