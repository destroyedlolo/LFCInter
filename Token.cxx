/***************************************************************************\
*  Token.cxx                                                                *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*   Méthodes de la classe '_token'                                          *
*                                                                           *
\************* Voir LFCInter.cxx pour plus d'informations ******************/

#include "Token.h"
#include <cstring>

#define isid(c) (isalnum((unsigned char)(c)) || ((c)=='_'))

static struct _motsclef {
    const char *nom;
    int type;   // valeur du symbole
    bool mda;   // Le message indiquant un warning a déjà été affiché (valide uniquement pour les mots clefs ignorés)
    int h;

    bool cmp(const char *, int, int);
} motsclef[] = {
    {">>="  ,smbl_sra,false,0}, // 0
    {"<<="  ,smbl_sla,false,0}, // 1
    {"+="  ,smbl_aa,false,0},   // 2
    {"-="  ,smbl_sa,false,0},   // 3
    {"*="  ,smbl_ma,false,0},   // 4
    {"/="  ,smbl_da,false,0},   // 5
    {"%="  ,smbl_mda,false,0},  // 6
    {"&="  ,smbl_anda,false,0}, // 7
    {"^="  ,smbl_xora,false,0}, // 8
    {"|="  ,smbl_ora,false,0},  // 9
    {">>"  ,smbl_sr,false,0},   // 10
    {"<<"  ,smbl_sl,false,0},   // 11
    {"++"  ,smbl_inc,false,0},  // 12
    {"--"  ,smbl_dec,false,0},  // 13
    {"&&"  ,smbl_land,false,0}, // 14
    {"||"  ,smbl_lor,false,0},  // 15
    {"<="  ,smbl_lle,false,0},  // 16
    {">="  ,smbl_lge,false,0},  // 17
    {"=="  ,smbl_lequ,false,0}, // 18
    {"!="  ,smbl_lne,false,0},  // 19
    {"/*"  ,smbl_debc,false,0}, // 20
    {"*/"  ,smbl_finc,false,0}, // 21
    {"//"  ,smbl_debcppc,false,0}, // 22
#define PREMIERMOTCLEF  23
    {"float"   ,-1 ,false,0},   // Ce doit être le premier des token alphabétic
    {"struct"  ,-1 ,false,0},
    {"union"   ,-1 ,false,0},
    {"typedef" ,-1 ,false,0},
    {"double"  ,-1 ,false,0},
    {"static"  ,-1 ,false,0},
    {"goto"  , /*smbl_goto*/ -1,false,0}, // J'aime les gotos mais ils compliquent les libérations de variables dynamiques
    {"register",-2 ,false,0},
    {"volatile",-2 ,false,0},
    {"short"   ,-2 ,false,0},
    {"const"   ,-2 ,false,0},
    {"long"    ,-2 ,false,0},
    {"extern"  ,-2 ,false,0},
    {"signed"  ,-2 ,false,0},
    {"unsigned",-2 ,false,0},
    {"auto"    ,-2 ,false,0},
    {"void"  ,smbl_void,false,0},
    {"int"   ,smbl_int,false,0},
    {"char"  ,smbl_char,false,0},
    {"for"   ,smbl_for,false,0},
    {"do"    ,smbl_do,false,0},
    {"while" ,smbl_while,false,0},
    {"switch",smbl_switch,false,0},
    {"case"  ,smbl_case,false,0},
    {"default"  ,smbl_default,false,0},
    {"if"    ,smbl_if,false,0},
    {"else"  ,smbl_else,false,0},
    {"break" ,smbl_break,false,0},
    {"continue",smbl_continue,false,0},
    {"return",smbl_return,false,0},
    {"exit"  ,smbl_exit,false,0},
    {"enum"  ,smbl_enum,false,0},
    {"printf",smbl_printf,false,0},
    {"sprintf",smbl_sprintf,false,0},
    {"scanf" ,smbl_scanf,false,0},
    {"putchar"  ,smbl_putchar,false,0},
    {"puts"  ,smbl_puts,false,0},
/*    {"sizeof",smbl_sizeof,false,0}, */
    {"gets",smbl_gets,false,0},
    {"getchar",smbl_getchar,false,0},
    {"flushstdout",smbl_flushstdout,false,0}, // Fonction non standard
    {"atoi",smbl_atoi,false,0},
    {"strcat",smbl_strcat,false,0},
    {"strchr",smbl_strchr,false,0},
    {"strcmp",smbl_strcmp,false,0},
    {"strcpy",smbl_strcpy,false,0},
    {"strerror",smbl_strerror,false,0},
    {"strlen",smbl_strlen,false,0},
    {"strncat",smbl_strncat,false,0},
    {"strncmp",smbl_strncmp,false,0},
    {"strncpy",smbl_strncpy,false,0},
    {"strrchr",smbl_strrchr,false,0},
    {"strdup",smbl_strdup,false,0},
    {"stricmp",smbl_stricmp,false,0},
    {"strcasecmp",smbl_strcasecmp,false,0},
    {"strnicmp",smbl_strnicmp,false,0},
    {"strncasecmp",smbl_strncasecmp,false,0},
    {"strpbrk",smbl_strpbrk,false,0},
    {"strstr",smbl_strstr,false,0},
    {"strcoll",smbl_strcoll,false,0},
    {"strcspn",smbl_strcspn,false,0},
    {"strspn",smbl_strspn,false,0},
    {"strtok",smbl_strtok,false,0},
    {"strtol",smbl_strtol,false,0},
#if 0
    {"swab",smbl_swab,false,0},
#endif
    {"memchr",smbl_memchr,false,0},
    {"memcmp",smbl_memcmp,false,0},
    {"memcpy",smbl_memcpy,false,0},
    {"memmove",smbl_memmove,false,0},
    {"memset",smbl_memset,false,0},
    {"memccpy",smbl_memccpy,false,0},
    {"system",smbl_system,false,0},
    {"isdigit",smbl_isdigit,false,0},
    {"islower",smbl_islower,false,0},
    {"isspace",smbl_isspace,false,0},
    {"ispunct",smbl_ispunct,false,0},
    {"isupper",smbl_isupper,false,0},
    {"isalpha",smbl_isalpha,false,0},
    {"isxdigit",smbl_isxdigit,false,0},
    {"isalnum",smbl_isalnum,false,0},
    {"isprint",smbl_isprint,false,0},
    {"isgraph",smbl_isgraph,false,0},
    {"iscntrl",smbl_iscntrl,false,0},
    {"isascii",smbl_isascii,false,0},
    {"isiso",smbl_isiso,false,0},
    {"toupper",smbl_toupper,false,0},
    {"tolower",smbl_tolower,false,0},
    {"toiso",smbl_toiso,false,0},
    {"time",smbl_time,false,0},
    {"ctime",smbl_ctime,false,0},
    {"clock",smbl_clock,false,0},
    {"sleep",smbl_sleep,false,0},
    {"realloc",smbl_realloc,false,0},
    {"malloc",smbl_malloc,false,0},
    {"free"  ,smbl_free,false,0},
    {NULL,0,false,0}
};

bool _motsclef::cmp(const char *avec, int lenavec, int havec){
         if(!h) h= calchash(nom); // Le code hash n'était pas encore calculé

         if(h!=havec)    // Les codes hashs ne correpondent pas
                  return false;

                  // Pour gagner du temps, on compare les chaînes tout en calculant leur longueur
         register int l;
         for(l=0; l<lenavec ; l++)
                  if(nom[l] != avec[l]) return false; // Une lettre ne correspond pas

                  // On a comparé toutes les lettres sur la longueur d'avec: nom doit être fini
         return !nom[l];
}

void _token::construit( bool ign ){
/* Construit les éléments de l'objet en fonction de ptr.
 *      . Décode le token qui est actuellement pointé.
 *      . Affiche les éventuels warnings associés aux mots clefs ignorés
 *      . Affiche les erreurs associés aux mots clefs non supportés
 *  -> ign indique si les messages doivent être ignorés (dans des commentaires)
 */

    if(!ptr){ // Le pointeur n'est pas valide
        val = len = hash = 0;
        return;
    }

    for(; *ptr; ptr++) // On ignore tous les caractères de contrôle
        if((unsigned char)(*ptr)>' ') break;
/*        if((unsigned char)(*ptr)>' ' && *ptr != '\\') break; */

    if(!isalpha((unsigned char)(*ptr)) && *ptr !='_' ){ // On lit autre chose qu'une lettre
        register int i;
        hash = 0;
        for(i=0;motsclef[i].type>=0;i++){ // Ne lit que les symboles spéciaux
            len = strlen(motsclef[i].nom);
            if(!strncmp(motsclef[i].nom,ptr,len)){ // On a trouvé !
                val = motsclef[i].type;
                return;
            }
        }
        len = 1; val = *ptr;
        return;
    } else { // Ou un ID ou un mot clef
        for(len=1; isid( *(ptr+len) ); len++); // Calcul de la longueur du token

        hash = calchash(obj().c_str());
        register int i;

        for( i=PREMIERMOTCLEF; motsclef[i].nom; i++ ){
            if( motsclef[i].cmp( ptr, len, hash )){ // On a trouvé un mot clef
                if((val = motsclef[i].type)<0 && !ign){
                    if(val == -2 && !motsclef[i].mda){
                        std::cerr << "*R* Ligne " << calcligne(ptr) << ":  Le mot clef '" << motsclef[i].nom << "' est ignoré.\n";
                        motsclef[i].mda=true;
                    } else { // -1 donc erreur
                        std::cerr << "*R* Ligne " << calcligne(ptr) << ":  Le mot clef '" << motsclef[i].nom << "' n'est pas supporté par ce programme.\n";
                        exit(5);
                    }
                }
                return;
            }
        }
        val = smbl_id;
    }
}

std::string _token::obj(){
    if(!ptr) return "";

    return std::string(ptr,
#ifdef STRCONSTOFF
        0,
#endif
        len);
}

_token _token::operator ++ (){ // Préfixe
/* Vas au token suivant :  Les commentaires sont ignorés...
 * Pour accélérer les choses, les commentaires ne sont pas décodés, mais
 * uniquement lus.
 */
    if(!ptr || !len) return *this; // Une des infos de base est invalide
    ptr += len;

autre:
    construit();

    if(val == smbl_debcppc){ // Lecture d'un commentaire C++
        len = 0;
        for(; *ptr != '\n' && *ptr; ptr++);
        ptr++;
        goto autre;
    } else if(val == smbl_debc){ // Lecture d'un commentaire C
        int niveau; // Nombre de commentaires imbriqués
        ptr += len; // Saute le début de commentaire
        for( niveau=1; niveau; ptr++ ){
            switch(*ptr){
            case '/':
                if(ptr[1]=='*'){ // C'est un commentaire imbriqué
                    niveau++;
                    ptr+=2;
                }
                break;
            case '*':
                if(ptr[1]=='/'){ // Fin d'un commentaire
                    niveau--;
                    ptr+=2;
                }
                break;
            case 0:
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": " << niveau
                     << " commentaire" << std::string((niveau>1)?"s":" ")
                     << " n'" << std::string((niveau>1)?"ont":"a")
                     << " pas été fermé" << std::string((niveau>1)?"s":" ") << ".\n";
                exit(5);
            }
        }
        goto autre;
    }

    return *this;
}

_token _token::operator ++ (int){ // suffixe
/* Vas au token suivant :  Les commentaires sont ignorés...
 * Pour accélérer les choses, les commentaires ne sont pas décodés, mais
 * uniquement lus.
 */
    _token bidon = *this;
    ++*this;
    return bidon;
}

void _token::saute(const char *sep){
/* Recherche la fin d'un bloc débutant sans chercher à le
 * comprendre mais en respectant les règles du C !
 * Si le premier caractère rencontré est:
 *  '{' : Saute le bloc <d'instructions> jusqu'au '}' fermant (inclus)
 *  '(' : Saute le bloc <d'arguments> jusqu'au ')' fermant (inclus)
 * sinon : Saute jusqu'au 'sep' suivant (inclus)
 *
 *  Basés sur les sources d'LFLocalize 0.4ß © LFSoft 1994-95
 */

    struct {
        unsigned short int  com,        // Commentaires '/* */'
                            blocf,      // Blocs '{}'
                            blocc;      // Blocs '()'
        char                car;        // Caractère déterminant le mode
        unsigned            lcom :1;    // Commentaires C++ '//'
        unsigned            slt :1;     // Le dernier caractère est '/'
        unsigned            star :1;    // Le dernier caractère '*'
        unsigned            ignore :1;  // Le dernier caractère '\'
        unsigned            instr :1;   // Lecture d'une chaîne ""
        unsigned            inchar :1;  // Lecture d'une définition de caractère ''
    } status;

    status.com = status.blocf = status.blocc = 0;
    status.lcom = status.slt = status.star = 0;
    status.ignore = status.instr = status.inchar = 0;

    if(*ptr == '{' /* } */ || *ptr == '(' /* ) */)
        status.car = *ptr++;
    else
        status.car = 0;

    while(*ptr){
        if(status.instr) switch(*ptr){ // Lecture d'une chaîne
            case '\n':
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Chaîne non terminée !\n";
                exit(5);
            case '\\':
                status.ignore ^= 1;
                break;
            case '"':
                if(status.ignore)
                    status.ignore = 0;
                else
                    status.instr = 0;
                break;
            default:
                status.ignore = 0;
        } else if(status.inchar) switch(*ptr){ // Lecture d'une chaîne
            case '\n':
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": caractère '' non terminée !\n";
                exit(5);
            case '\\':
                status.ignore ^= 1;
                break;
            case '\'':
                if(status.ignore)
                    status.ignore = 0;
                else
                    status.inchar = 0;
                break;
            default:
                status.ignore = 0;
        } else if(status.lcom){ // Lecture d'un commentaire C++
            if( *ptr == '\n' ) status.lcom = 0;
        } else if(status.com){ // Lecture d'un commentaire
            if( *ptr == '*' ){
                if(status.slt){ // Imbrication ?
                    status.slt = 0;
                    status.com++;
                    status.star = 0;
                } else
                    status.star = 1;
            } else if( *ptr == '/' ){
                if(status.star){    // Fermeture
                    status.star = 0; status.com--;
                } else
                    status.slt = 1;
            } else {
                status.star = 0; status.slt = 0;
            }
        } else {    // Autre
            if(status.ignore)
                status.ignore = 0;
            else switch(*ptr){
            case '/':
                if(status.slt){ // Commentaire C++ ?
                    status.slt = 0; status.lcom = 1;
                } else if(status.star) {
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Fermeture superflue d'un commentaire.\n";
                    exit(5);
                } else
                    status.slt = 1;
                break;
            case '*':
                if(status.slt){ // Nouveau commentaire ?
                    status.com++; status.slt =0;
                } else
                    status.star = 1;
                break;
            case '\\':
                status.ignore = 1; status.star=status.slt=0;
                break;
            case '\'':
                status.inchar = 1;status.star=status.slt=0;
                break;
            case '"':
                status.instr = 1;status.star=status.slt=0;
                break;
            case '{': /* } */
                status.blocf++;status.star=status.slt=0;
                break;
            case '(': /* ) */
                status.blocc++;status.star=status.slt=0;
                break;
/* { */     case '}':
                if(!status.blocf){
                    if(status.blocc){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur d'imbrication de blocs '{}' et '()'.\n";
                        exit(5);
                    }

                    if(status.car == '{' /*}*/ || strchr(sep,*ptr)){ // Le bloc est sauté
                        construit(); (*this)++; return;
                    } else {
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Fermeture superflue d'un bloc '{}'.\n";
                        exit(5);
                    }
                } else
                    --status.blocf;
                status.star=0;status.slt=0;
                break;
/* ( */         case ')':
                if(!status.blocc){
                    if(status.blocf){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur d'imbrication de blocs '{}' et '()'.\n";
                        exit(5);
                    }

                    if(status.car == '(' /*)*/ || strchr(sep,*ptr)){ // Le bloc est sauté
                        construit(); (*this)++; return;
                     } else {
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Fermeture superflu d'un bloc '()'.\n";
                        exit(5);
                    }
                } else
                    --status.blocc;
                status.star=0;status.slt=0;
                break;
            default:
                if(strchr(sep,*ptr)){ // Est-ce un des séparateurs recherchés ?
                    if(!status.blocc && !status.blocf && status.car != '{' /* } */ && status.car !='(' /* ) */){
                        construit(); (*this)++; return;
                    }
                }
                status.star=0;status.slt=0;
            }
        }
        ptr++;
    }
    construit();
}
