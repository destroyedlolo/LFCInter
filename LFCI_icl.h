/***************************************************************************\
*  LFCI_icl.h                                                               *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Définition de fonctions internes aux calculs.                            *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#ifndef LFCI_ICL_H
#define LFCI_ICL_H

#ifndef LCI_TOKEN_H
#include "Token.h"
#endif

#ifndef LFDYNASTACK_H
#include "LFDStack.h"
#endif

#ifndef LFCI_CAL_H
#include "LFCI_Cal.h"
#endif

#ifndef isodigit
#define isodigit(x) ((x)>='0' && (x)<='7')
#endif

extern const struct _operateur {
    int op;             // Valeur de l'opérateur
    _rep (*fonc)( LFDynaStack<_rep> &, _token & );
    char priorite;      // Priorité de cet opérateur
    bool unaire;        // Est-ce un opérateur unaire
    bool devant;        // Si c'est un opérateur unaire, doit-il être devant la valeur
} operateur[];


extern _rep conditionnel(_token &, _tablesmb *);
extern _rep affectation(_token &, _tablesmb *);
extern _rep plus( LFDynaStack<_rep> &, _token &);
extern _rep deref( LFDynaStack<_rep> &, _token &);
#endif
