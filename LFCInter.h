/***************************************************************************\
*  LFCInter.h                                                               *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Définitions générales.                                                   *
*                                                                           *
\************* Voir LFCInter.cxx pour plus d'informations ******************/

#ifndef LFCINTER_H
#define LFCINTER_H

        /**** Compatibilités entre les compilateurs *****/
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

        /************* Memorisation des messages à afficher ************/
extern struct _amsg {
    _amsg() : wrn_1(false), conv_int_ptr(true), conv_ptr_int(true),
            perte_precis(true), rt_perte_prec(false), rt_fatal_ptr(false),
            conv_nstd(false) {};

    unsigned wrn_1          : 1; // Affiche les remarques qu'une seule fois
    unsigned conv_int_ptr   : 1; // Conversion int -> ptr
    unsigned conv_ptr_int   : 1; // Conversion ptr -> int
    unsigned perte_precis   : 1; // Perte de précision possible (int -> char)
    unsigned rt_perte_prec  : 1; // Test de perte de précision en temps réèl
    unsigned rt_fatal_ptr   : 1; // Test en temp réèl sur les pertes de précision lors de conversion ptr->int (pour PC)
    unsigned conv_nstd      : 1; // Opération non standard (utilisée pour pallier au manque de cast)
} amsg;

        /************ Quelques fonctions couramment utilisées ************/
extern int calcligne( const char * ); // Une surcharge utilisant _token est définie dans Token.h
extern int calchash( const char *);
#endif
