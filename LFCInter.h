/***************************************************************************\
*  LFCInter.h                                                               *
*  Projet : LFCInter                                                        *
*      � LFSoft 1995-96                                                     *
*                                                                           *
*  D�finitions g�n�rales.                                                   *
*                                                                           *
\************* Voir LFCInter.cxx pour plus d'informations ******************/

#ifndef LFCINTER_H
#define LFCINTER_H

        /**** Compatibilit�s entre les compilateurs *****/
#ifdef __BCPLUSPLUS__
    #include <cstring.h>
        /* La define suivante indique que la classe 'string' contient une valeur
         * d'offset dans son constructeur...
         */
    #define STRCONSTOFF
    typedef int bool;
    #define true -1
    #define false 0
    #define strcasecmp stricmp
    #define strncasecmp strnicmp
#else
    #include <string>
    #include <stdlib.h>
#endif

#include <iostream>

        /**** Quelques #defines utiles *****/

#ifdef DEBUG
    #define DEB(x) {x;}
    #define ADEB(x) x
#else
    #define DEB(x)
    #define ADEB(x)
#endif

#ifndef NULL
    #define NULL 0
#endif

#ifndef FOREVER
#define FOREVER for(;;)
#endif

        /************* Memorisation des messages � afficher ************/
extern struct _amsg {
    _amsg() : wrn_1(false), conv_int_ptr(true), conv_ptr_int(true),
            perte_precis(true), rt_perte_prec(false), rt_fatal_ptr(false),
            conv_nstd(false) {};

    unsigned wrn_1          : 1; // Affiche les remarques qu'une seule fois
    unsigned conv_int_ptr   : 1; // Conversion int -> ptr
    unsigned conv_ptr_int   : 1; // Conversion ptr -> int
    unsigned perte_precis   : 1; // Perte de pr�cision possible (int -> char)
    unsigned rt_perte_prec  : 1; // Test de perte de pr�cision en temps r��l
    unsigned rt_fatal_ptr   : 1; // Test en temp r��l sur les pertes de pr�cision lors de conversion ptr->int (pour PC)
    unsigned conv_nstd      : 1; // Op�ration non standard (utilis�e pour pallier au manque de cast)
} amsg;

        /************ Quelques fonctions couramment utilis�es ************/
extern int calcligne( const char * ); // Une surcharge utilisant _token est d�finie dans Token.h
extern int calchash( const char *);
#endif
