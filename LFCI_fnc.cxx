/***************************************************************************\
*  LFCI_fnc.cxx                                                             *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Execution des fonctions internes et construction des arguments.          *
*                                                                           *
*   Note: Dans ce fichier il y a des caratères '(',')','{','}'... mis en    *
*   commentaire. C'est uniquement pour permettre à mon éditeur de tester    *
*   les blocs...                                                            *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#include "LFCInter.h"
#include "Token.h"
#include "LFDStack.h"
#include "LFCI_Cal.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

static void litargs(_token &ptr, _tablesmb *table_locale, LFDynaStack<_rep> &args, int nbreargs, const char *fonc){
/* Lit un nombre fixe d'arguments.
 *  -> ptr : pointe sur le premier argument,
 *  -> args : Pile qui contiendra les arguments lus,
 *  -> nbreargs : Nombre d'arguments à lire,
 *  -> fonc : Nom de la fonction dont on lit les arguments...
 */
    if(*ptr != /*(*/ ')') FOREVER { // Lecture des arguments
        args.Push( eval(ptr,table_locale,true) );
        if(*ptr == ',')
            ptr++;
        else if(*ptr == /*(*/ ')') // C'était le dernier argument
            break;
        else {
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe pour " << fonc << ADEB( " (litargs:1)" << ) " !\n";
            exit(5);
        }
    }

    if( nbreargs > args.length()+1 ){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Pas assez d'arguments ont été fournis pour " << fonc << ADEB( " (litargs:2)" << ) " !\n";
        exit(5);
    } else if( nbreargs < args.length()+1 )
        std::cerr << "*A* Ligne " << calcligne(ptr) << ": Trop d'arguments ont été fournis pour " << fonc << ADEB( " (litargs:3)" << ) " !\n";
}

_rep interne(_token &ptr,_tablesmb *table_locale){
/* Execute une fonction émulée en interne...
 *  <- ptr : pointe sur le nom de la fonction à émuler
 *     table_locale : table locale au moment du passage des arguments
 *  -> ptr : apres le #(# ')' de fermeture des arguments
 */
    LFDynaStack<_rep> args;
    int idfonc = *ptr++;
    _rep ret;

    if(*ptr++ != '(' /*)*/){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu\n" /*)*/
                "*E*  ... ou tentative pour obtenir l'adresse d'une fonction interne !\n";
        exit(5);
    }

    switch(idfonc){
    case smbl_printf:{
            _rep fmt; //Format de la chaîne à afficher
            fmt=conv('*',eval(ptr, table_locale,true),ptr);
            while(*ptr == ','){ // Lecture des autres arguments
                ptr++;
                _rep t=eval(ptr, table_locale,true);
                args.Push(t);
            }

            const char *x = (const char *)fmt.val.ptr;
            int na=0; // Numéro de l'argument lu

            while(*x){
                switch(*x){
                case '%':{
                        std::string format;
                        format='%';

                        switch(*++x){
                        case '-':
                        case '+':
                        case ' ':
                        case '#':
                        case '0':
                            format += *x++;
                        }

                        if(*x=='*'){
                            int i=conv('I',args[na++],ptr).val.entier;
                            char t[10];
                            sprintf(t,"%d",i);
                            format += t;
                            x++;
                        } else while(isdigit(*x))
                            format += *x++;

                        if(*x=='.'){
                            format += *x++;
                            if(*x=='*'){
                                int i=conv('I',args[na++],ptr).val.entier;
                                char t[10];
                                sprintf(t,"%d",i);
                                format += t;
                                x++;
                            } else while(isdigit(*x))
                                format += *x++;
                        }

                        if(*x=='h' || *x=='l' || *x=='L') // Modificateur (sans intérêt car il n'y a pas de short et de long)
                            x++;

                        format += *x;

                        switch(*x){
                        case 'c':
                            ret=printf(format.c_str(),conv('C',args[na++],ptr).val.caractere);
                            break;
                        case 'd':
                        case 'i':
                        case 'o':
                        case 'u':
                        case 'x':
                        case 'X':
                            ret=printf(format.c_str(),conv('I',args[na++],ptr).val.entier);
                            break;
                        case 'n':
                        case 'p':
                        case 's':
                            ret=printf(format.c_str(),conv('*',args[na++],ptr).val.ptr);
                            break;
                        case 'e':
                        case 'E':
                        case 'f':
                        case 'G':
                        case 'g':
                        default:
                            putchar(*x);
                        }
                    }
                    break;
                default:
                    putchar(*x);
                }
                x++;
            }
        }
        break;
    case smbl_sprintf:{
            char *chaine; // Chaîne à affecter
            _rep fmt; //Format de la chaîne à afficher

            chaine = (char *)conv('*',eval(ptr, table_locale,true),ptr).val.ptr; // Lecture du pointeur sur la chaîne
            if(*ptr != ','){
                std::cerr << "*E* Ligne " << calcligne(ptr) << " Fonction sprintf(): Il manque des arguments.\n";
                exit(5);
            }
            ptr++;

            fmt=conv('*',eval(ptr, table_locale,true),ptr);

            while(*ptr == ','){ // Lecture des autres arguments
                ptr++;
                _rep t=eval(ptr, table_locale,true);
                args.Push(t);
            }

            std::string res;

            const char *x = (const char *)fmt.val.ptr;
            int na=0; // Numéro de l'argument lu

            while(*x){
                switch(*x){
                case '%':{
                        std::string format("%");
                        int lmt=0; // Si !=0 si une longueur mini a été demandée
    // /!\ si les flotants sont utilisables, il faut aussi tenir compte des '%.?'

                        switch(*++x){
                        case '-':
                        case '+':
                        case ' ':
                        case '#':
                        case '0':
                            format += *x++;
                        }

                        if(*x=='*'){
                            lmt=conv('I',args[na++],ptr).val.entier;
                            char t[10];
                            sprintf(t,"%d",lmt);
                            format += t;
                            x++;
                        } else while(isdigit(*x)){
                            lmt = lmt*10 + *x-'0';
                            format += *x++;
                        }

                        if(*x=='.'){
                            format += *x++;
                            if(*x=='*'){
                                int i=conv('I',args[na++],ptr).val.entier;
                                char t[10];
                                sprintf(t,"%d",i);
                                format += t;
                                x++;
                            } else while(isdigit(*x))
                                format += *x++;
                        }

                        if(*x=='h' || *x=='l' || *x=='L') // Modificateur (sans intérêt car il n'y a pas de short et de long)
                            x++;

                        format += *x;

                        char *tmp=0;

                        switch(*x){
                        case 'c':
                            if(lmt)
                                tmp = new char[lmt+ 5];
                            else
                                tmp = new char[5];

                            if(!tmp){
                                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de mémoire.\n";
                                exit(5);
                            }

                            sprintf(tmp,format.c_str(),conv('C',args[na++],ptr).val.caractere);
                            break;

                        case 'd':
                        case 'i':
                        case 'u':
                            {
                                int l=sizeof(int)/2*5; // Longueur théorique
                                if(lmt<l)
                                    lmt=l;
                            }
                            tmp = new char[lmt+5];

                            if(!tmp){
                                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de mémoire.\n";
                                exit(5);
                            }

                            goto suite_sprintf_int;

                        case 'o':
                            {
                                int l=sizeof(int)*3; // Longueur théorique
                                if(lmt<l)
                                    lmt=l;
                            }
                            tmp = new char[lmt+5];

                            if(!tmp){
                                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de mémoire.\n";
                                exit(5);
                            }

                            goto suite_sprintf_int;

                        case 'x':
                        case 'X':
                            {
                                int l=sizeof(int)*2; // Longueur théorique
                                if(lmt<l)
                                    lmt=l;
                            }
                            tmp = new char[lmt+5];

                            if(!tmp){
                                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de mémoire.\n";
                                exit(5);
                            }

            suite_sprintf_int:
                            sprintf(tmp,format.c_str(),conv('I',args[na++],ptr).val.entier);
                            break;

                        case 'n':
    /*
     *  Contrairement aux autres spécificateurs de conversion, le %n n'est pas
     * utilisé pour afficher une valeur, mais affecte un entier, dont l'adresse
     * est passée en paramêtre avec le nombre de caractères déja stockés.
     */
                            if(args[na].type() != 'R'){
                                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Utilisation incorrecte du spécificateur '%n',\n"
                                     << "*E* Ligne " << calcligne(ptr) << "l'argument " << na << " de sprintf() n'est ni une variable, ni une reference.\n";
                                exit(5);
                            } else
                                RefAffecte(args[na++],(long)res.length(),ptr);
                            break;

                        case 'p':
                            {
                                int l=sizeof(void *)*2; // Longueur théorique
                                if(lmt<l)
                                    lmt=l;
                            }
                            tmp = new char[lmt+5];

                            if(!tmp){
                                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de mémoire.\n";
                                exit(5);
                            }

                            sprintf(tmp,format.c_str(),conv('*',args[na++],ptr).val.ptr);
                            break;
                        case 's':
                            {
                                const char *t = (char *)conv('*',args[na++],ptr).val.ptr;
                                int l=strlen(t); // Longueur théorique
                                if(lmt<l)
                                    lmt=l;

                                tmp = new char[lmt+5];

                                if(!tmp){
                                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de mémoire.\n";
                                    exit(5);
                                }
                                sprintf(tmp,format.c_str(),t);
                            }
                            break;
                        case 'e':
                        case 'E':
                        case 'f':
                        case 'G':
                        case 'g':
                        default:
                            res += *x;
                        }
                        if(tmp){
                            res += tmp;
                            delete[] tmp;
                            tmp = 0;
                        }
                    }
                    break;
                default:
                    res += *x;
                }
                x++;
            }
            strcpy(chaine, res.c_str());
        }
        break;
    case smbl_putchar:
        litargs(ptr, table_locale,  args, 1, "putchar");
        ret=conv('I',args[0],ptr);
        ret=putchar(ret.val.entier);
        break;
    case smbl_puts:
        litargs(ptr, table_locale,  args, 1, "puts");
        ret=puts((char *)conv('*',args[0],ptr).val.ptr);
        break;
    case smbl_gets:
#if 0
        litargs(ptr, table_locale,  args, 1, "gets");
        ret=gets((char *)conv('*',args[0],ptr).val.ptr);
#else
	std::cerr << "gets() desactive\n";
	exit(20);
#endif
        break;
    case smbl_getchar:
        litargs(ptr, table_locale,  args, 0, "getchar");
        ret=getchar();
        break;
    case smbl_flushstdout:
    // Cette fonction n'est pas standard mais pallie au manque de "fichier" dans
    // la version 1.0 de l'interpréteur...
        litargs(ptr, table_locale,  args, 0, "flushstdout");
        fflush(stdout);
        break;
    case smbl_atoi:
        litargs(ptr, table_locale,  args, 1, "atoi");
        ret = atoi((char *)conv('*',args[0],ptr).val.ptr);
        break;
    case smbl_strcat:
        litargs(ptr, table_locale,  args, 2, "strcat");
        ret = _rep( (void *)strcat( (char *)conv('*',args[0],ptr).val.ptr,
                      (char *)conv('*',args[1],ptr).val.ptr ),
                    '*',std::string("C") );
        break;
    case smbl_strchr:
        litargs(ptr, table_locale,  args, 2, "strchr");
        ret = _rep( (void *)strchr( (char *)conv('*',args[0],ptr).val.ptr,
                                            conv('I',args[1],ptr).val.entier ),
                    '*',std::string("C") );
        break;
     case smbl_strcmp:
         litargs(ptr, table_locale,  args, 2, "strcmp");
         ret = strcmp( (char *)conv('*',args[0],ptr).val.ptr,
                       (char *)conv('*',args[1],ptr).val.ptr );
         break;
    case smbl_strcpy:
        litargs(ptr, table_locale,  args, 2, "strcpy");
        ret = _rep( (void *)strcpy( (char *)conv('*',args[0],ptr).val.ptr,
                                    (char *)conv('*',args[1],ptr).val.ptr ),
                    '*',std::string("C") );
        break;
    case smbl_strerror:
        litargs(ptr, table_locale,  args, 1, "strerror");
        ret = _rep( (void *)strerror(conv('I',args[0],ptr).val.entier),
                    '*',std::string("C") );
        break;
    case smbl_strlen:
        litargs(ptr, table_locale,  args, 1, "strlen");
        ret=(int)strlen((char *)conv('*',args[0],ptr).val.ptr);
        break;
    case smbl_strncat:
        litargs(ptr, table_locale,  args, 3, "strncat");
        ret = _rep( (void *)strncat( (char *)conv('*',args[0],ptr).val.ptr,
                      (char *)conv('*',args[1],ptr).val.ptr,
                      conv('I',args[2],ptr).val.entier ),
                    '*',std::string("C") );
        break;
    case smbl_strncmp:
        litargs(ptr, table_locale,  args, 3, "strncmp");
        ret = strncmp( (char *)conv('*',args[0],ptr).val.ptr,
                      (char *)conv('*',args[1],ptr).val.ptr,
                      conv('I',args[2],ptr).val.entier );
        break;
    case smbl_strncpy:
        litargs(ptr, table_locale,  args, 3, "strncpy");
        ret = _rep( (void *)strncpy( (char *)conv('*',args[0],ptr).val.ptr,
                                    (char *)conv('*',args[1],ptr).val.ptr,
                                    conv('I',args[2],ptr).val.entier ),
                    '*',std::string("C") );
        break;
    case smbl_strrchr:
        litargs(ptr, table_locale,  args, 2, "strrchr");
        ret = _rep( (void *)strrchr( (char *)conv('*',args[0],ptr).val.ptr,
                                            conv('I',args[1],ptr).val.entier ),
                    '*',std::string("C") );
        break;
    case smbl_strdup:
        litargs(ptr, table_locale,  args, 1, "strdup");
        ret=_rep( (void *)strdup((char *)conv('*',args[0],ptr).val.ptr),
                    '*',std::string("C") );
        break;
    case smbl_stricmp:
    case smbl_strcasecmp:
         litargs(ptr, table_locale,  args, 2, "strcasecmp");
         ret = strcasecmp( (char *)conv('*',args[0],ptr).val.ptr,
                       (char *)conv('*',args[1],ptr).val.ptr );
         break;
    case smbl_strnicmp:
    case smbl_strncasecmp:
        litargs(ptr, table_locale,  args, 3, "strncasecmp");
        ret = strncasecmp( (char *)conv('*',args[0],ptr).val.ptr,
                      (char *)conv('*',args[1],ptr).val.ptr,
                      conv('I',args[2],ptr).val.entier );
        break;
    case smbl_strpbrk:
        litargs(ptr, table_locale,  args, 2, "strpbrk");
        ret = _rep( (void *)strpbrk( (char *)conv('*',args[0],ptr).val.ptr,
                                     (char *)conv('*',args[1],ptr).val.ptr),
                    '*',std::string("C") );
        break;
    case smbl_strstr:
        litargs(ptr, table_locale,  args, 2, "strstr");
        ret = _rep( (void *)strstr( (char *)conv('*',args[0],ptr).val.ptr,
                                     (char *)conv('*',args[1],ptr).val.ptr),
                    '*',std::string("C") );
        break;
    case smbl_strcoll:
         litargs(ptr, table_locale,  args, 2, "strcoll");
         ret = strcoll( (char *)conv('*',args[0],ptr).val.ptr,
                       (char *)conv('*',args[1],ptr).val.ptr );
         break;
    case smbl_strcspn:
         litargs(ptr, table_locale,  args, 2, "strcspn");
         ret = (int)strcspn( (char *)conv('*',args[0],ptr).val.ptr,
                        (char *)conv('*',args[1],ptr).val.ptr );
         break;
    case smbl_strspn:
         litargs(ptr, table_locale,  args, 2, "strspn");
         ret = (int)strspn( (char *)conv('*',args[0],ptr).val.ptr,
                       (char *)conv('*',args[1],ptr).val.ptr );
         break;
    case smbl_strtok:
        litargs(ptr, table_locale,  args, 2, "strtok");
        ret = _rep( (void *)strtok( (char *)conv('*',args[0],ptr).val.ptr,
                                    (char *)conv('*',args[1],ptr).val.ptr),
                    '*',std::string("C") );
        break;
    case smbl_strtol:
        litargs(ptr, table_locale,  args, 3, "strtol");
        ret = (long)strtol( (char *)conv('*',args[0],ptr).val.ptr,
                            (char **)conv('*',args[1],ptr).val.ptr,
                            conv('I',args[2],ptr).val.entier );
        break;
#if 0
    case smbl_swab:
        litargs(ptr, table_locale,  args, 3, "swab");
#ifdef AMIGA
/* buggue des includes de GCC ? Pour le borland & pour le compilateur natif du
 * VAX cette fonction devrait prendre des 'char *' comme arguments...
 */
        swab( (const void *)conv('*',args[0],ptr).val.ptr,
              (void *)conv('*',args[1],ptr).val.ptr,
              conv('I',args[2],ptr).val.entier);
#else
        swab( (char *)conv('*',args[0],ptr).val.ptr,
              (char *)conv('*',args[1],ptr).val.ptr,
              conv('I',args[2],ptr).val.entier);
#endif
        break;
#endif
    case smbl_memchr:
        litargs(ptr, table_locale,  args, 3, "memchr");
        ret = _rep( (void *)memchr( (void *)conv('*',args[0],ptr).val.ptr,
                                            conv('I',args[1],ptr).val.entier,
                                            conv('I',args[2],ptr).val.entier ),
                    '*',std::string("V") );
        break;
     case smbl_memcmp:
         litargs(ptr, table_locale,  args, 3, "memcmp");
         ret = memcmp( (void *)conv('*',args[0],ptr).val.ptr,
                       (void *)conv('*',args[1],ptr).val.ptr,
                        conv('I',args[2],ptr).val.entier );
         break;
    case smbl_memcpy:
        litargs(ptr, table_locale,  args, 3, "memcpy");
        ret = _rep( (void *)memcpy( (void *)conv('*',args[0],ptr).val.ptr,
                                    (void *)conv('*',args[1],ptr).val.ptr,
                                    conv('I',args[2],ptr).val.entier ),
                    '*',std::string("V") );
        break;
    case smbl_memmove:
        litargs(ptr, table_locale,  args, 3, "memmove");
        ret = _rep( (void *)memmove( (void *)conv('*',args[0],ptr).val.ptr,
                                    (void *)conv('*',args[1],ptr).val.ptr,
                                    conv('I',args[2],ptr).val.entier ),
                    '*',std::string("V") );
        break;
    case smbl_memset:
        litargs(ptr, table_locale,  args, 3, "memset");
        ret = _rep( (void *)memset( (void *)conv('*',args[0],ptr).val.ptr,
                                    conv('I',args[1],ptr).val.entier,
                                    conv('I',args[2],ptr).val.entier ),
                    '*',std::string("V") );
        break;
    case smbl_memccpy:
        litargs(ptr, table_locale,  args, 4, "memccpy");
        ret = _rep( (void *)memccpy( (void *)conv('*',args[0],ptr).val.ptr,
                                    (void *)conv('*',args[1],ptr).val.ptr,
                                    conv('I',args[2],ptr).val.entier,
                                    conv('I',args[3],ptr).val.entier ),
                    '*',std::string("V") );
        break;
    case smbl_system:
        litargs(ptr, table_locale,  args, 1, "system");
        ret = system((char *)conv('*',args[0],ptr).val.ptr);
        break;
    case smbl_isdigit:
        litargs(ptr, table_locale,  args, 1, "isdigit");
        ret = isdigit(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_islower:
        litargs(ptr, table_locale,  args, 1, "islower");
        ret = islower(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isspace:
        litargs(ptr, table_locale,  args, 1, "isspace");
        ret = isspace(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_ispunct:
        litargs(ptr, table_locale,  args, 1, "ispunct");
        ret = ispunct(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isupper:
        litargs(ptr, table_locale,  args, 1, "isupper");
        ret = isupper(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isalpha:
        litargs(ptr, table_locale,  args, 1, "isalpha");
        ret = isalpha(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isxdigit:
        litargs(ptr, table_locale,  args, 1, "isxdigit");
        ret = isxdigit(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isalnum:
        litargs(ptr, table_locale,  args, 1, "isalnum");
        ret = isalnum(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isprint:
        litargs(ptr, table_locale,  args, 1, "isprint");
        ret = isprint(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isgraph:
        litargs(ptr, table_locale,  args, 1, "isgraph");
        ret = isgraph(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_iscntrl:
        litargs(ptr, table_locale,  args, 1, "iscntrl");
        ret = iscntrl(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_isascii:
        litargs(ptr, table_locale,  args, 1, "isascii");
        ret = isascii(conv('I',args[0],ptr).val.entier);
        break;
#ifdef AMIGA
    case smbl_isiso:
/* Cette fonction (ou macro) n'est pas définie sur tous les systèmes, même en
 * utilisant GCC (par exemple, elle n'existe pas sur le MicroVAX).
 */
        litargs(ptr, table_locale,  args, 1, "isiso");
        ret = isiso(conv('I',args[0],ptr).val.entier);
        break;
#endif
    case smbl_toupper:
        litargs(ptr, table_locale,  args, 1, "toupper");
        ret = toupper(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_tolower:
        litargs(ptr, table_locale,  args, 1, "tolower");
        ret = tolower(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_toiso:
        litargs(ptr, table_locale,  args, 1, "toiso");
        ret = tolower(conv('I',args[0],ptr).val.entier);
        break;
    case smbl_time: // Attention: il FAUT un pointeur sur un long int sur beaucoup de systèmes
        litargs(ptr, table_locale,  args, 1, "time");
        ret = time( (time_t *)conv('*',args[0],ptr).val.ptr);
        break;
    case smbl_ctime:
        litargs(ptr, table_locale,  args, 1, "ctime");
        ret = _rep( (void *)ctime( (time_t *)conv('*',args[0],ptr).val.ptr),
                    '*',std::string("C") );
        break;
    case smbl_clock:
        litargs(ptr, table_locale,  args, 0, "clock");
        ret=(long int)clock();
        break;
#ifndef __BCPLUSPLUS__
    case smbl_sleep:
/* sleep() n'est pas défini que sous un seul environnement: windows
 *  le problème est que même lorsqu'on compile un programme ms-dos, le
 *  borland ne défini pas cette fonction!
 */
        litargs(ptr, table_locale,  args, 1, "sleep");
        ret = (int)sleep(conv('I',args[0],ptr).val.entier);
        break;
#endif
    case smbl_realloc:
        litargs(ptr, table_locale,  args, 2, "realloc");
        ret=realloc((void *)conv('*',args[0],ptr).val.ptr,
                    conv('L',args[1],ptr).val.literal);
        break;
    case smbl_malloc:
        litargs(ptr, table_locale,  args, 1, "malloc");
        ret=malloc(conv('L',args[0],ptr).val.literal);
        break;
    case smbl_free:
        litargs(ptr, table_locale,  args, 1, "free");
        free(conv('*',args[0],ptr).val.ptr); // Pas d'affectation car c'est une fonction void
        break;
    default:
        std::cerr << "*F* Ligne " << calcligne(ptr) << ": Tentative d'exécuter une fonction interne non reconnue.\n"
            "*F* Peut-être n'existe-elle pas sur ce système.";
        exit(5);
    }

    if( *ptr != /*(*/')' ){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe pour " << ptr.obj() << ADEB( " (interne:1)" << ) " !\n";
        exit(5);
    }
    ptr++; // On saute le /*(*/ ')'
    return ret;
}

_rep lancefonc(_token &ptr, _var_fonc *fonc, _tablesmb *table_locale){
/* Lance l'exécution d'une fonction.
 *  <- ptr : debut des arguments (après le '(' #)# )
 *     fonc : Fonction à exécuter (DOIT être valide car aucun test n'est fait)
 *     table_locale : table locale au moment du passage des arguments
 *  -> ptr pointe sur ce qui suit les paramêtres (#(# après le ')' fermant )
 */
    LFDynaStack<_rep> args;

    if(*ptr != /*(*/ ')') FOREVER { // Lecture des arguments
        args.Push( eval(ptr,table_locale,true) );
        if(*ptr == ',')
            ptr++;
        else if(*ptr == /*(*/ ')') // C'était le dernier argument
            break;
        else {
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe pour " << ptr.obj() << ADEB( " (lancefonc:1)" << ) " !\n";
            exit(5);
        }
    }

    ptr++; // On saute le /*(*/ ')'
    return execfonc(fonc,args);
}
