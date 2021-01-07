/***************************************************************************\
*                   classe pour créer une pile de valeurs...                *
*                           © LFSoft 1995-96                                *
 ***************************************************************************
*                                                                           *
*    Cette classe générique permet de créer une pile de valeurs.            *
*                                                                           *
*    Au lieu d'utiliser des listes chaînées qui semblent faire ramer les PC *
*   en 16 bits (pas testé en 32), mais surtout qui provoquent une           *
*   fragmentation de la mémoire, des blocs de données sont alloués. En plus *
*   pour les fonctions qui utilisent beaucoup ce genre de pile, l'exécution *
*   est accélérée car les allocations/libérations mémoires sont réduites.   *
*                                                                           *
*    Note: Ce fichier provoque beaucoup de warnings pour certains           *
*   compilateurs comme le Borland C++ qui ne développent pas en ligne les   *
*   methodes comportant des mots clefs du C (on se demande bien pourquoi !) *
*    J'ai été obligé de le faire car beaucoup d'autres compilateurs C++,    *
*   GCC en premier, se mélangent les pinceaux avec des 'templates' dont les *
*   méthodes ne sont pas 'inline'!                                          *
*                                                                           *
 ***************************************************************************
*         Historique                                                        *
*                                                                           *
*   Décembre 1995: Première version.                                        *
*   11/04/1996 : La méthode 'operator[]' renvoie une référence ce qui       *
*       permet de modifier la valeur stockée dans la pile...                *
*   11/04/1996 : GCC n'aime pas la définition d'un objet T dans la classe.  *
*       Une erreur est générée si l'opérande de la méthode 'operator[]' est *
*       mauvais... (en fait, le problème est qu'il s'agit d'un template sur *
*       une autre classe (_rep) qui doit obligatoirement être défini après  *
*       celle-ci).                                                          *
*   15/04/1996 SB: La méthode 'Pop()' mettait à 1 cidx s'il devenait plus   *
*       grand que maxidx mais sans revalider le segment courant...          *
*                                                                           *
\***************************************************************************/

#ifndef LFDYNASTACK_H
#define LFDYNASTACK_H

#ifdef LFCI_CAL_H
#error Inclure LFCI_Cal.h apres ce fichier
#endif

/*
#ifdef __GNUG__
    #include <builtin.h>
#endif
*/

template <class T> class LFDynaStack {
private:
    /*
     *  Segments de données
     */
    struct LFDSData {
        LFDSData *suivant, *precedent;
        T *data;
    } *premier, *dernier, *courant;

    /*
     * Index
     */
    int nbreparseg; // Nombre d'objets par segment
    int maxidx;     // Nombre d'objets dans la pile
    int cidx;       // Idx de l'objet courrant

public:
    LFDynaStack(int anbreparseg = 16)
        : premier(0),dernier(0),nbreparseg(anbreparseg),maxidx(-1),cidx(-1){};
    ~LFDynaStack(){ // Efface la pile et son contenu.
        while(premier){
            courant = premier;
            premier = premier->suivant;
            delete[] courant->data;
            delete courant;
        }
    };

    bool Push(T truc){
       /* Ajoute une donnée dans la pile
        * -> Succes ou non (manque de mémoire)
        */
        if(!((++maxidx)%nbreparseg)){ // On doit ajouter un nouveau segment
            LFDSData *nouveau = new LFDSData;
            if(!nouveau) return false;

            nouveau->data = new T[nbreparseg];
            if(!nouveau->data) return false;

                /* Lien inter segments */
            nouveau->suivant = 0;
            if((nouveau->precedent = dernier)) // Ce n'est pas le premier segment
                dernier->suivant = nouveau;
            else // C'est le premier segment
                premier = nouveau;
            dernier = nouveau;
        }

        dernier->data[maxidx%nbreparseg] = truc;
        return true;
    };

    T Pop(){
       /* Retourne la derniere valeur de la pile et l'efface
        * NOTE: Il faut absolument que l'objet mémorisé est un constructeur par défaut
        *  car c'est cette 'valeur' qui est renvoyée si la pile est vide.
        */
        T ret;

        if(maxidx != -1){
            ret = dernier->data[maxidx%nbreparseg];
            if(!(maxidx%nbreparseg)){ // C'était la derniere valeur du segment
                LFDSData *tmp = dernier;
                if(!(dernier = dernier->precedent)) // C'est le seul segment
                    premier = 0;
                else
                    dernier->suivant = 0;
                delete[] tmp->data;
                delete tmp;
            }
            maxidx--;
            if(cidx>maxidx) cidx = -1;
        }
        return ret;
    };

    T &operator[](int idx){
       /* Retourne la valeur de la pile dont l'index est passé en argument.
        * -> NULL en cas d'erreur (index incorrect)
        */
        if(idx>maxidx || idx<0) // Index hors limite (y compris s'il n'y a pas de données dans la pile).
        {
            std::cerr << "*F* Index incorrecte !\n";
            exit(20);
        }

        if(cidx == -1){ // Données courantes non initialisées
            courant = premier; cidx = 0;
        }

        int numcseg = cidx/nbreparseg,  // Numéro du segment courant
            numdseg = idx/nbreparseg;   // Numéro du segment de destination

        if(numdseg != numcseg){ // Ce n'est pas le segment courant
            int diff= numdseg - numcseg;

            if(abs(diff)>numdseg){ // Il est plus rapide de rechercher depuis le début
                diff = numdseg; courant = premier;
            }

            if(abs(diff) > maxidx/nbreparseg-numdseg){ // Plus rapide par la fin
                diff = numdseg - maxidx/nbreparseg; courant = dernier;
            }

            if(diff>0) // La destination est plus loin dans la liste
                for(;diff;diff--) courant = courant->suivant;
            else
                for(;diff;diff++) courant = courant->precedent;
        }
        cidx = idx;
        return courant->data[idx%nbreparseg];
    };

    int length(){ return maxidx; }; // Nombre d'objet dans la pile
    int current(){ return cidx; };  // Index de l'objet courant
};

#endif

