/***************************************************************************\
*  LFCI_icl.cxx                                                             *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Définition de fonctions internes aux calculs.                            *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/
#include "LFCInter.h"
#include "LFCI_icl.h"

_rep adr( LFDynaStack<_rep> &pile, _token &ptr){ // Adresse de l'opérande : Référencement
    _rep t = pile.Pop();

    if(t.type() != 'R'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte de l'operateur '&', l'argument n'est ni une variable, ni une reference.\n";
        exit(5);
    }

    t.typebase = '*';

    return t;
}

_rep deref( LFDynaStack<_rep> &pile, _token &ptr){ // Contenu de l'adresse pointée : Déréférencement
    _rep t=conv(0,pile.Pop(),ptr); // Ce ne peut plus être une référence

    switch(t.type()){
    case 'L':
    case 'I':
        t = conv('*',t,ptr);
    case '*':
        return _rep(t.val.ptr,'R',t.info);
    default:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte des operateurs '*' ou '[]',\n"
             << "*E* Ligne " << calcligne(ptr) << ": l'argument n'est pas un pointeur.\n";
        exit(5);
    }
}

static _rep nonb( LFDynaStack<_rep> &pile, _token &ptr){ // Négation bit a bit ('~' unaire)
    _rep t=conv(0,pile.Pop(),ptr); // Ce ne peut plus être une référence

    switch(t.type()){
    case 'I':
        return ~t.val.entier;
    case 'C':
        return ~t.val.caractere;
    case 'L':
        return ~t.val.literal;
    case '*':   // Théoriquement, un - unaire sur un pointeur provoque une erreur
                // mais, comme il n'y a pas de cast, il est ici convertie en entier
        if(amsg.conv_nstd){
            std::cerr << "*R* Ligne " << calcligne(ptr) << ": Operation non standard en C, ~ unaire sur un pointeur (ici convertit en entier).\n";
            if(amsg.wrn_1) amsg.conv_nstd = false;
        }
        return ~conv('I',t,ptr).val.entier;
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, ~ unaire sur un type'" << t.type() << "'.\n";
        exit(5);
    }
}

static _rep non( LFDynaStack<_rep> &pile, _token &ptr){ // non unaire ('!')
    return !pile.Pop().nonnull(ptr);
}

static _rep rien( LFDynaStack<_rep> &pile, _token &){ // Opérateur nul (genre '+' unaire)
    return conv(0,pile.Pop(),NULL); // Juste une déréférenciation est nécéssaire.
}

static _rep neg( LFDynaStack<_rep> &pile, _token &ptr){ // Négation de la valeur ('-' unaire)
    _rep t=conv(0,pile.Pop(),ptr); // Ce ne peut plus être une référence

    switch(t.type()){
    case 'I':
        return -t.val.entier;
    case 'C':
        return -t.val.caractere;
    case 'L':
        return -t.val.literal;
    case '*':   // Théoriquement, un - unaire sur un pointeur provoque une erreur
                // mais, comme il n'y a pas de cast, il est ici converti en entier
        if(amsg.conv_nstd){
            std::cerr << "*R* Ligne " << calcligne(ptr) << ": Operation non standard en C, - unaire sur un pointeur (ici convertit en entier).\n";
            if(amsg.wrn_1) amsg.conv_nstd = false;
        }
        return -conv('I',t,ptr).val.entier;
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, - unaire sur un type'" << t.type() << "'.\n";
        exit(5);
    }
}

_rep plus( LFDynaStack<_rep> &pile, _token &ptr){ // Addition
    _rep y= conv(0,pile.Pop(),NULL), x= conv(0,pile.Pop(),NULL);

        /*
         * Réorganisation des données pour éviter de devoir multiplier les cas
         */
    if(x.type() == 'L' && y.type() != 'L'){ // Valeur litérale en seconde valeur
        _rep t=x; x=y; y=t;
    }

    if(y.type() == '*' && x.type() != '*'){ // Pointeur en premiere valeur
        _rep t=x; x=y; y=t;
    }

    if(y.type() == 'V') conv('I',y,ptr); // Uniquement pour afficher une erreur

    switch(x.type()){
    case 'V':
        return conv('I',x,ptr); // Uniquement pour afficher une erreur
    case 'L':
        return x.val.literal + y.val.literal;
    case 'I':
        switch(y.type()){
        case 'C':
        case 'L':
            y=conv('I',y,ptr);
        case 'I':
            return x.val.entier + y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <int> + <" << y.type() << ">.\n";
            exit(10);
        }
    case 'C':
        switch(y.type()){
        case 'L':
            y=conv('C',y,ptr);
        case 'C':
            return x.val.caractere + y.val.caractere;
        case 'I':
            return x.val.caractere + y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <char> + <" << y.type() << ">.\n";
            exit(10);
        }
    case '*':
        switch(y.type()){
        case '*':
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": <pointeur> + <pointeur> interdit.\n";
            exit(5);
        case 'C':
        case 'L':
            y=conv('I',y,ptr);
        case 'I':
            switch(x.info[0]){
            case '*':
                return _rep((void *)((void **)x.val.ptr + y.val.entier),'*',x.info);
            case 'I':
                return _rep((void *)((int *)x.val.ptr + y.val.entier),'*',x.info);
            case 'C':
                return _rep((void *)((char *)x.val.ptr + y.val.entier),'*',x.info);
            default:
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": impossible de faire de l'arithmètique avec un pointeur sur un type <" << x.info[0] << ">.\n";
                exit(5);
            }
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <pointeur> + <" << y.type() << ">.\n";
            exit(10);
        }
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Type <" << x.type() << "> inattendu pour une addition.\n";
        exit(10);
    }
}

static _rep moins( LFDynaStack<_rep> &pile, _token &ptr){ // Soustraction
    _rep y= conv(0,pile.Pop(),NULL), x= conv(0,pile.Pop(),NULL);

    if(y.type() == 'V') conv('I',y,ptr); // Uniquement pour afficher une erreur

    if(x.type() == 'L') x=conv(y.type(),x,ptr);

    if(y.type() == 'L') y=conv(x.type(),y,ptr);

    if(x.type() != '*' && y.type() == '*'){ // Hormis pour les comparaisons de pointeurs, la 2eme operande ne peut être qu'un 'I' ou 'C'
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Interdit de faire une soustraction avec un pointeur comme second operande.\n";
        exit(5);
    }

    switch(x.type()){
    case 'V':
        return conv('I',x,ptr); // Uniquement pour afficher une erreur
    case 'L': // Ce ne peut être que 2 opérandes litérales, du à la conversion plus haut
        return x.val.literal - y.val.literal;
    case 'I':
        switch(y.type()){
        case 'C':
            return x.val.entier - y.val.caractere;
        case 'I':
            return x.val.entier - y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <int> - <" << y.type() << ">.\n";
            exit(10);
        }
    case 'C':
        switch(y.type()){
        case 'C':
            return x.val.caractere - y.val.caractere;
        case 'I':
            return x.val.caractere - y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <char> - <" << y.type() << ">.\n";
            exit(10);
        }
    case '*': // C'est une comparaison de pointeur
        if(y.type() == '*'){
            if(x.info != y.info){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": On ne peut comparer que des pointeurs de meme type.\n";
                exit(10);
            }
            switch(x.info[0]){
            case '*':
                return _rep((void *)((void **)x.val.ptr - (void **)y.val.ptr),'*',x.info);
            case 'I':
                return _rep((void *)((int *)x.val.ptr - (int *)y.val.ptr),'*',x.info);
            case 'C':
                return _rep((void *)((char *)x.val.ptr - (char *)y.val.ptr),'*',x.info);
            default:
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": impossible de faire de l'arithmetique avec un pointeur sur un type <" << x.info[0] << ">.\n";
                exit(5);
            }
        } else {
            if(y.type() == 'L'){    // Les valeurs litérales doivent conserver toutes leurs précisions ... même sur les brouettes (OS 16 bits).
                switch(x.info[0]){
                case '*':
                    return _rep((void *)((void **)x.val.ptr - y.val.literal),'*',x.info);
                case 'I':
                    return _rep((void *)((int *)x.val.ptr - y.val.literal),'*',x.info);
                case 'C':
                    return _rep((void *)((char *)x.val.ptr - y.val.literal),'*',x.info);
                default:
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": impossible de faire de l'arithmetique avec un pointeur sur un type <" << x.info[0] << ">.\n";
                    exit(5);
                }
            } else {
                y=conv('I',y,ptr);

                switch(x.info[0]){
                case '*':
                    return _rep((void *)((void **)x.val.ptr - y.val.entier),'*',x.info);
                case 'I':
                    return _rep((void *)((int *)x.val.ptr - y.val.entier),'*',x.info);
                case 'C':
                                                  return _rep((void *)((char *)x.val.ptr - y.val.entier),'*',x.info);
                default:
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": impossible de faire de l'arithmetique avec un pointeur sur un type <" << x.info[0] << ">.\n";
                    exit(5);
                }
            }
        }
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Type <" << x.type() << "> inattendu pour une soustraction.\n";
        exit(10);
    }
}

static _rep mul( LFDynaStack<_rep> &pile, _token &ptr){ // Multiplication
    _rep y= conv(0,pile.Pop(),NULL), x= conv(0,pile.Pop(),NULL);

    if(x.type() == '*' || y.type() == '*'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de faire une multiplication avec un pointeur.\n";
        exit(5);
    }

        /*
         * Réorganisation des données pour éviter de devoir multiplier les cas
         */
    if(x.type() == 'L' && y.type() != 'L'){ // Valeur litérale en seconde valeur
        _rep t=x; x=y; y=t;
    }

    if(y.type() == 'V') conv('I',y,ptr); // Uniquement pour afficher une erreur

    switch(x.type()){
    case 'V':
        return conv('I',x,ptr); // Uniquement pour afficher une erreur
    case 'L':
        return x.val.literal * y.val.literal;
    case 'I':
        switch(y.type()){
        case 'C':
        case 'L':
            y=conv('I',y,ptr);
        case 'I':
            return x.val.entier * y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <int> * <" << y.type() << ">.\n";
            exit(10);
        }
    case 'C':
        switch(y.type()){
        case 'L':
            y=conv('C',y,ptr);
        case 'C':
            return x.val.caractere * y.val.caractere;
        case 'I':
            return x.val.caractere * y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <char> * <" << y.type() << ">.\n";
            exit(10);
        }
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Type <" << x.type() << "> inattendu pour une multiplication.\n";
        exit(10);
    }
}

static _rep div( LFDynaStack<_rep> &pile, _token &ptr){ // Division
    _rep y= conv(0,pile.Pop(),NULL), x= conv(0,pile.Pop(),NULL);

    if(x.type() == '*' || y.type() == '*'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de faire une division avec un pointeur.\n";
        exit(5);
    }

    if(x.type() == 'L') x=conv(y.type(),x,ptr);

    if(y.type() == 'V') conv('I',y,ptr); // Uniquement pour afficher une erreur

    if(!y.nonnull(ptr)){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Division par 0.\n";
        exit(5);
    }

    switch(x.type()){
    case 'V':
        return conv('I',x,ptr); // Uniquement pour afficher une erreur
    case 'L':
        return x.val.literal / y.val.literal;
    case 'I':
        switch(y.type()){
        case 'C':
        case 'L':
            y=conv('I',y,ptr);
        case 'I':
            return x.val.entier / y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <int> / <" << y.type() << ">.\n";
            exit(10);
        }
    case 'C':
        switch(y.type()){
        case 'L':
            y=conv('C',y,ptr);
        case 'C':
            return x.val.caractere / y.val.caractere;
        case 'I':
            return x.val.caractere / y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <char> / <" << y.type() << ">.\n";
            exit(10);
        }
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Type <" << x.type() << "> inattendu pour une division.\n";
        exit(10);
    }
}

static _rep mod( LFDynaStack<_rep> &pile, _token &ptr){ // Modulo
    _rep y= conv(0,pile.Pop(),NULL), x= conv(0,pile.Pop(),NULL);

    if(x.type() == '*' || y.type() == '*'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de faire un modulo avec un pointeur.\n";
        exit(5);
    }

    if(x.type() == 'L') x=conv(y.type(),x,ptr);

    if(y.type() == 'V') conv('I',y,ptr); // Uniquement pour afficher une erreur

    if(!y.nonnull(ptr)){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Modulo par 0.\n";
        exit(5);
    }

    switch(x.type()){
    case 'V':
        return conv('I',x,ptr); // Uniquement pour afficher une erreur
    case 'L':
        return x.val.literal % y.val.literal;
    case 'I':
        switch(y.type()){
        case 'C':
        case 'L':
            y=conv('I',y,ptr);
        case 'I':
            return x.val.entier % y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <int> % <" << y.type() << ">.\n";
            exit(10);
        }
    case 'C':
        switch(y.type()){
        case 'L':
            y=conv('C',y,ptr);
        case 'C':
            return x.val.caractere % y.val.caractere;
        case 'I':
            return x.val.caractere % y.val.entier;
        default:
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Operation inattendue, <char> % <" << y.type() << ">.\n";
            exit(10);
        }
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Type <" << x.type() << "> inattendu pour un modulo.\n";
        exit(10);
    }
}

static _rep incpre( LFDynaStack<_rep> &pile, _token &ptr){ // Pré-incrémentation
    _rep x=pile[pile.length()];

    if(x.type() != 'R'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte de l'operateur '++', l'argument n'est ni une variable, ni une reference.\n";
        exit(5);
    }

    pile.Push(_rep((long) 1));
    _rep t = plus(pile,ptr);    // Incrémente la valeur
    RefAffecte(x,t,ptr);           // et la réaffecte sur l'objet

    return t;                   // et on la retourne
}

static _rep decpre( LFDynaStack<_rep> &pile, _token &ptr){ // Pré-décrémentation
    _rep x=pile[pile.length()];

    if(x.type() != 'R'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte de l'operateur '++', l'argument n'est ni une variable, ni une reference.\n";
        exit(5);
    }

    pile.Push(_rep((long) 1));
    _rep t = moins(pile,ptr);    // Décrémente la valeur
    RefAffecte(x,t,ptr);            // et la réaffecte sur l'objet

    return t;                   // et on la retourne
}

static _rep incpos( LFDynaStack<_rep> &pile, _token &ptr){ // Post-incrémentation
    _rep x=pile[pile.length()],
         y = conv(0,x,NULL);

    if(x.type() != 'R'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte de l'operateur '++', l'argument n'est ni une variable, ni une reference.\n";
        exit(5);
    }

    pile.Push(_rep((long) 1));
    _rep t = plus(pile,ptr);    // Incrémente la valeur
    RefAffecte(x,t,ptr);           // et la réaffecte sur l'objet

    return y;                   // Mais retourne la valeur initiale
}

static _rep decpos( LFDynaStack<_rep> &pile, _token &ptr){ // Post-décrémentation
    _rep x=pile[pile.length()],
         y = conv(0,x,NULL);

    if(x.type() != 'R'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte de l'operateur '++', l'argument n'est ni une variable, ni une reference.\n";
        exit(5);
    }

    pile.Push(_rep((long) 1));
    _rep t = moins(pile,ptr);    // Décrémente la valeur
    RefAffecte(x,t,ptr);           // et la réaffecte sur l'objet

    return y;                   // Mais retourne la valeur initiale
}

static _rep sr( LFDynaStack<_rep> &pile, _token &){ // '>>'
    _rep y= conv('I',pile.Pop(),NULL), x= conv('I',pile.Pop(),NULL);

    return x.val.entier >> y.val.entier;
}

static _rep sl( LFDynaStack<_rep> &pile, _token &){ // '<<'
    _rep y= conv('I',pile.Pop(),NULL), x= conv('I',pile.Pop(),NULL);

    return x.val.entier << y.val.entier;
}

static _rep inferieur( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier < y.val.entier;
}

static _rep supperieur( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier > y.val.entier;
}

static _rep inf_egal( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier <= y.val.entier;
}

static _rep sup_egal( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier >= y.val.entier;
}

static _rep egual( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv(0,pile.Pop(),NULL), x= conv(0,pile.Pop(),NULL);

    if(y.type() == '*' && x.type() != '*'){ // Pointeur en premiere valeur
        _rep t=x; x=y; y=t;
    }

    if(x.type() == '*'){
        if(y.type() == '*')
            return x.val.ptr == y.val.ptr;
        else
            return (long)x.val.ptr == (long)conv('I',y,ptr).val.entier;
    } else
        return conv('I',x,ptr).val.entier == conv('I',y,ptr).val.entier;
}

static _rep diff( LFDynaStack<_rep> &pile, _token &ptr){
    return !egual(pile,ptr).val.entier;
}

static _rep et( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier & y.val.entier;
}

static _rep xou( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier ^ y.val.entier;
}

static _rep iou( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier | y.val.entier;
}

static _rep let( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier && y.val.entier;
}

static _rep lou( LFDynaStack<_rep> &pile, _token &ptr){
    _rep y= conv('I',pile.Pop(),ptr), x= conv('I',pile.Pop(),ptr);

    return x.val.entier || y.val.entier;
}

    /*
     *  Liste des opérateurs reconnus.
     *
     * NOTE: Cette liste ne contient que les opérateurs de priorité >= à -8
     * c'est à dire tous les opérateurs unaires et binaires. '?:' et les
     * opérateurs d'affectation sont directement décodés par les fonctions
     * conditionnel() et affectation()
     */

const struct _operateur operateur[] = {
    {smbl_inc, incpos, 3, true, false}, // '++'; en réalité une priorité de 3 est inutile par rapport aux opérateurs
    {smbl_dec, decpos, 3, true, false}, // '--'; de devant mais c'est pour respecter la BNF...
    {'[' /*]*/, NULL, 3, true, false},  // uniquement pour lectunaire()
    {'&', adr, 2, true, true},
    {'*', deref, 2, true, true},
    {'+', rien, 2, true, true},
    {'-', neg, 2, true, true},
    {'~', nonb, 2, true, true},
    {'!', non, 2, true, true},
    {smbl_inc, incpre, 2, true, true}, // '++'
    {smbl_dec, decpre, 2, true, true}, // '--'
    {'*', mul, 1, false},
    {'/', div, 1, false},
    {'%', mod, 1, false},
    {'+', plus, 0, false},
    {'-', moins, 0, false},
    {smbl_sr, sr, -1, false},   // '>>'
    {smbl_sl, sl, -1, false},   // '<<'
    {'<',inferieur, -2, false},
    {'>',supperieur, -2, false},
    {smbl_lle,inf_egal, -2, false}, // '<='
    {smbl_lge,sup_egal, -2, false}, // '>='
    {smbl_lequ,egual, -3, false},   // '=='
    {smbl_lne,diff, -3, false},     // '!='
    {'&',et, -4, false},
    {'^',xou, -5, false},
    {'|',iou, -6, false},
    {smbl_land,let, -7, false}, // '&&'
    {smbl_lor,lou, -8, false},  // '||
    {0, 0}
};

_rep affectation(_token &ptr, _tablesmb *table_locale){
/* Lecture d'une expression d'"affectation".
 * Voir eval() pour plus d'informations sur les arguments.
 *
 *  BNF restreinte de l'expression attendu par cette fonction:
 *       reponse ::= reférence {oppaff exp_cond}
 *       reponse ::= exp_cond
 */
    LFDynaStack<_rep> valeurs;  // Pile des valeurs
    LFDynaStack<int> operateurs;   // Pile des opérateurs

    FOREVER{
        valeurs.Push( conditionnel(ptr,table_locale) );

        if((*ptr>=smbl_sra && *ptr<=smbl_ora)
          || *ptr == '='){
            if(valeurs[valeurs.length()].type() != 'R'){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Valeur de gauche d'une affectation non modifiable.\n";
#ifdef DEBUG
                std::cerr << "*E* type :" << valeurs[valeurs.length()].type() << ".\n";
#endif
                exit(5);
            }
            operateurs.Push(*ptr);
            ptr++;
        } else  // Il n'y a plus d'affectation
            break;
    }

    while(operateurs.length()>=0){ // Exécution des calculs
        _rep sur=valeurs[valeurs.length()-1]; // copy de la référence
        switch(operateurs.Pop()){
        case '=':{
                _rep y=valeurs.Pop(), // valeur à affectée
                     x=valeurs.Pop(); // objet à affecté

                valeurs.Push( RefAffecte(x,y,ptr) ); // Affectation et place la valeur convertie sur la pile
            } break;
        case smbl_sra: // ">>="
            valeurs.Push( RefAffecte( sur, sr(valeurs,ptr), ptr) );
            break;
        case smbl_sla: // "<<="
            valeurs.Push( RefAffecte(sur, sl(valeurs,ptr), ptr) );
            break;
        case smbl_aa: // "+="
            valeurs.Push( RefAffecte(sur, plus(valeurs,ptr), ptr) );
            break;
        case smbl_sa: // "-="
            valeurs.Push( RefAffecte(sur, moins(valeurs,ptr), ptr) );
            break;
        case smbl_ma: // "*="
            valeurs.Push( RefAffecte(sur, mul(valeurs,ptr), ptr) );
            break;
        case smbl_da: // "/="
            valeurs.Push( RefAffecte(sur, div(valeurs,ptr), ptr) );
            break;
        case smbl_mda: // "%="
            valeurs.Push( RefAffecte(sur, mod(valeurs,ptr), ptr) );
            break;
        case smbl_anda: // "&="
            valeurs.Push( RefAffecte(sur, et(valeurs,ptr), ptr) );
            break;
        case smbl_xora: // "^="
            valeurs.Push( RefAffecte(sur, xou(valeurs,ptr), ptr) );
            break;
        case smbl_ora:// "|="
            valeurs.Push( RefAffecte(sur, iou(valeurs,ptr), ptr) );
            break;
        }
    }

    return valeurs.Pop();
}
