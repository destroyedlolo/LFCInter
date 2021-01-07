/***************************************************************************\
*  LFCI_Lex.cxx                                                             *
*  Projet : LFCInter                                                        *
*      � LFSoft 1995-96                                                     *
*                                                                           *
*  Analyse lexicale.                                                        *
*                                                                           *
*   Note: Dans ce fichier il y a des carat�res '(',')','{','}'... mis en    *
*   commentaire. C'est uniquement pour permettre � mon �diteur de tester    *
*   les blocs...                                                            *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#include "LFCInter.h"
#include "LFDStack.h"
#include "Token.h"
#include "LFCI_Cal.h"

static void *alloue(std::string &type,LFDynaStack<unsigned int> &taille,int idx,_resallouee **mem,_token &ptr){
/*  Allocation d'une dimension d'un tableau.
 *  Cette fonction est appell�e r�cursivement pour allouer chaque dimension du tableau
 *
 *  type: type d'objet � allouer
 *  taille: pile des dimension du tableau
 *  idx: index de la dimension � allouer dans la pile
 *  mem: pointeur sur un pointeur sur le premier �l�ment de la liste des ressources allou�es pour ce tableau
 */

    if( idx<taille.length() ){ // Ce n'est pas la derniere dimension que l'on alloue
        unsigned int i;
        void **x=(void **)calloc(sizeof(void*),taille[idx]);

        if(!x){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de m�moire.\n";
            exit(5);
        }

        for(i=0; i<taille[idx]; i++)
            x[i]=alloue(type,taille,idx+1,mem,ptr);

        *mem= new _resallouee(*mem,(void *)x);
        if(!*mem){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de m�moire.\n";
            exit(5);
        }

        return (void *)x;
    } else {
        size_t t;
        switch(type[0]){
        case 'C':
            t=sizeof(char); break;
        case 'I':
            t=sizeof(int); break;
        case '*':
            t=sizeof(void *); break;
        default:
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de d�terminer la taille d'un objet de type '" << type[0] << "'.\n";
            exit(5);
        };

        void *x = calloc(t,taille[idx]);
        if(!x){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de m�moire.\n";
            exit(5);
        }

        *mem= new _resallouee(*mem,x);
        if(!*mem){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Manque de m�moire.\n";
            exit(5);
        }

        return x;
    }
}

bool lecdesc( _token &ptr, std::string &id, std::string &type, char itype, void **mem,_tablesmb *table_locale){
/* Lecture d'une d�finition.
 *  <-  ptr: D�but de la cha�ne � �valuer.
 *      itype: Type initial.
 *      mem : NULL si on ne veut que lire les descripteurs (pas d'allocation de m�moire)
 *
 *  -> false si le premier symbole n'est pas un type.
 *      ptr : pointe sur le premier objet qui suit la d�finition (pas besoin de ++)
 *      id : Identificateur.
 *      mem :
 *          .Si le type 'im�diat' est un tableau (T), cette variable pointe
 *      vers les resources qui lui sont allou�es (class _resallouee).
 *          .En cas de d�finition de fonction, cette variable pointe sur sur le
 *      '(' de d�finition des arguments. // )
 */
    _token porg = ptr;
    type = "";
    id = "";
    if(mem) *mem= NULL;

    if(itype){ // Le type initial est donn�
        type = itype;
        if(!ptr.definition()) // Il n'y a que le nom de la variable
            goto lex_id;
    } else if( !ptr.definition() || *ptr=='*' ){ // Ce n'est pas une d�finition
/* Si le premier caract�re est un '*' ce n'est pas une d�finition mais un
 * d�r�f�rencement...
 */
        ptr = porg;
        return false;
    }

    do {
        switch(*ptr){
        case -2: // Qualificateur ignor�
            break;
        case smbl_void:
            if(type.length()){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe lors d'une d�claration." << ADEB("(lex:1)" << ) "\n";
                exit(5);
            }
            type = 'V';
            break;
        case smbl_int:
            if(type.length()){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe lors d'une d�claration." << ADEB("(lex:2)" << ) "\n";
                exit(5);
            }
            type = 'I';
            break;
        case smbl_char:
            if(type.length()){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe lors d'une d�claration." << ADEB("(lex:3)" << ) "\n";
                exit(5);
            }
            type = 'C';
            break;
        case '*':
                // Si aucun type n'a �t� d�fini, c'est un pointeur sur un entier
            if(!type.length()) type='I';
            type= '*' + type;
            break;
        default:
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe lors d'une d�claration." << ADEB("(lex:4)" << ) "\n";
            exit(5);
        }
        ptr++;
    } while( ptr.definition() ); // On reste dans la boucle tant que ce qu'on lit est un type

lex_id:
    id = ptr.obj();

    if(*ptr != smbl_id){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Un identificateur �tait attendu.\n";
        exit(5);
    }

    ptr++;
    if(*ptr == '[' /* ] */ ){   // D�finition d'un tableau
        LFDynaStack<unsigned int> taille(5); // Stockage des diff�rentes dimensions du tableau

        if(!mem){
            std::cerr << "*F* Ligne " << calcligne(ptr) << ": Tentative pour allouer un tableau o� une allocation �tait interdite.\n";
            exit(5);
        }

        do {
            ptr++;
            taille.Push( (unsigned int)conv('I',eval(ptr,table_locale),ptr).val.entier );
            if(*ptr != /*[*/ ']'){
                std::cerr << "*E* Ligne " << calcligne(ptr) << /*[*/ ": Erreur de syntaxe, il manque le ']' de fermeture d'une d�claration de tableau.\n";
                exit(5);
            }
        } while(*++ptr == '[' /*]*/ );

        alloue(type,taille,0,(_resallouee **)mem,ptr);

            // Seul le type de base est un tableau, les autres sont de 'simples' pointeurs
        for(int i=0; i<taille.length(); i++)
            type = '*' + type;

        type = 'T' + type;

    } else if(*ptr == '(' /* ) */){ // D�finition d'une fonction
        if(mem) *mem = (void *) ((const char *)ptr + 1);
        ptr.saute();
        type = 'F' + type;
    }

    return true;
}
