/***************************************************************************\
*                   classe pour cr�er une pile de valeurs...                *
*                           � LFSoft 1995-96                                *
 ***************************************************************************
*                                                                           *
*    Cette classe g�n�rique permet de cr�er une pile de valeurs.            *
*                                                                           *
*    Au lieu d'utiliser des listes cha�n�es qui semblent faire ramer les PC *
*   en 16 bits (pas test� en 32), mais surtout qui provoquent une           *
*   fragmentation de la m�moire, des blocs de donn�es sont allou�s. En plus *
*   pour les fonctions qui utilisent beaucoup ce genre de pile, l'ex�cution *
*   est acc�l�r�e car les allocations/lib�rations m�moires sont r�duites.   *
*                                                                           *
*    Note: Ce fichier provoque beaucoup de warnings pour certains           *
*   compilateurs comme le Borland C++ qui ne d�veloppent pas en ligne les   *
*   methodes comportant des mots clefs du C (on se demande bien pourquoi !) *
*    J'ai �t� oblig� de le faire car beaucoup d'autres compilateurs C++,    *
*   GCC en premier, se m�langent les pinceaux avec des 'templates' dont les *
*   m�thodes ne sont pas 'inline'!                                          *
*                                                                           *
 ***************************************************************************
*         Historique                                                        *
*                                                                           *
*   D�cembre 1995: Premi�re version.                                        *
*   11/04/1996 : La m�thode 'operator[]' renvoie une r�f�rence ce qui       *
*       permet de modifier la valeur stock�e dans la pile...                *
*   11/04/1996 : GCC n'aime pas la d�finition d'un objet T dans la classe.  *
*       Une erreur est g�n�r�e si l'op�rande de la m�thode 'operator[]' est *
*       mauvais... (en fait, le probl�me est qu'il s'agit d'un template sur *
*       une autre classe (_rep) qui doit obligatoirement �tre d�fini apr�s  *
*       celle-ci).                                                          *
*   15/04/1996 SB: La m�thode 'Pop()' mettait � 1 cidx s'il devenait plus   *
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
     *  Segments de donn�es
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
       /* Ajoute une donn�e dans la pile
        * -> Succes ou non (manque de m�moire)
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
        * NOTE: Il faut absolument que l'objet m�moris� est un constructeur par d�faut
        *  car c'est cette 'valeur' qui est renvoy�e si la pile est vide.
        */
        T ret;

        if(maxidx != -1){
            ret = dernier->data[maxidx%nbreparseg];
            if(!(maxidx%nbreparseg)){ // C'�tait la derniere valeur du segment
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
       /* Retourne la valeur de la pile dont l'index est pass� en argument.
        * -> NULL en cas d'erreur (index incorrect)
        */
        if(idx>maxidx || idx<0) // Index hors limite (y compris s'il n'y a pas de donn�es dans la pile).
        {
            std::cerr << "*F* Index incorrecte !\n";
            exit(20);
        }

        if(cidx == -1){ // Donn�es courantes non initialis�es
            courant = premier; cidx = 0;
        }

        int numcseg = cidx/nbreparseg,  // Num�ro du segment courant
            numdseg = idx/nbreparseg;   // Num�ro du segment de destination

        if(numdseg != numcseg){ // Ce n'est pas le segment courant
            int diff= numdseg - numcseg;

            if(abs(diff)>numdseg){ // Il est plus rapide de rechercher depuis le d�but
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

