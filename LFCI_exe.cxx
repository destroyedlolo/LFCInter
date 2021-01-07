/***************************************************************************\
*  LFCI_exe.cxx                                                             *
*  Projet : LFCInter                                                        *
*      � LFSoft 1995-96                                                     *
*                                                                           *
*  Execution des instructions.                                              *
*                                                                           *
*   Note: Dans ce fichier il y a des carat�res '(',')','{','}'... mis en    *
*   commentaire. C'est uniquement pour permettre � mon �diteur de tester    *
*   les blocs...                                                            *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#include "LFCInter.h"
#include "Token.h"
#include "LFDStack.h"
#include "LFCI_Cal.h"

static _rep execbloc(_token &ptr, _tablesmb *tparent, int &inst);

static _rep execinst(_token &ptr, _tablesmb *tparent, int &inst){
/* Execute une instruction.
 * Voir execbloc() pour les param�tres.
 *  -> ne retourne qu'une valeur 'void' sauf dans le cas de l'instruction return
 * o� la valeur de retour est retourn�e.
 *  -> 'inst' contiendra le code de l'instruction ex�cut�e.
 *
 * Note: Pour acc�l�rer les choses, pour les instructions r�p�titives comme les
 *  boucles, la v�rification de la syntaxe n'est faite qu'une seule fois. Les
 *  informations comme les conditions de sortie sont stock�s � la premiere passe...
 */
    _rep ret;

    switch(inst=*ptr){
    case smbl_return:
        ptr++;
        ret = eval(ptr,tparent);
        if(ret.type() == 'V') return ret;
        break;
    case smbl_break:
    case smbl_continue:
        ptr++;
        break;
    case smbl_for:{
            _token  cond, // Condition de sortie de la boucle
                    exp,  // Expression d'it�ration
                    dbloc; // Bloc � ex�cuter

            if(*++ptr != '(' /*)*/){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' �tait attendu apr�s un 'for'\n" /*)*/;
                exit(5);
            }

            ptr++;
            eval(ptr,tparent); // Evaluation de l'expression initiale
            if(*ptr++ != ';'){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, ';' �tait attendu" << ADEB( " ('"<< ptr.obj() << "' lu)" << ) ".\n";
                exit(5);
            }
            cond=ptr;
            ptr.saute(); // Saute la condition de sortie
            exp=ptr;
            ptr.saute( /*(*/ ")" ); // Saute l'expr�ssion d'it�ration
            dbloc=ptr;

            FOREVER {
                ptr = cond;
                ret=eval(ptr,tparent); // Evaluation de la condition de sortie

                ptr = dbloc;
                if(ret.type() != 'V') if(!ret.nonnull(ptr)){ // La condition est fausse: Il faut sortir
                    ptr.saute();
                    break;
                }

                int svt;
                ret = execbloc(ptr,tparent,svt);
                if(svt == smbl_return){
                    inst = smbl_return;
                    return ret;
                } else if(svt == smbl_break){
                    ptr = dbloc;
                    ptr.saute();
                    break;
                }
    /* rien pour
     *   if(svt == smbl_continue)
     * car 'continue' doit passer le contr�le � l'expression d'incr�mentation
     * ... qui se trouve juste ici dans ce source !
     */
                ptr=exp;
                if(*ptr != /*(*/ ')' ) // Y a t-il une expression
                    eval(ptr,tparent); // Evaluation de l'expression d'it�ration
            }
        }
        return _rep();
    case smbl_do:{
            _token dbloc=++ptr, // D�but du bloc � ex�cuter
                    cond, // Condition de la boucle
                   suite; // Suite du programme

            FOREVER {
                int svt;
                ptr = dbloc;
                ret = execbloc(ptr,tparent,svt);
                if(svt == smbl_return){
                    inst = smbl_return;
                    return ret;
                } else if(svt == smbl_break){
                    if(*suite){
                        ptr = suite;
                        break;
                    } else { // Il faut d�terminer o� sauter
                        ptr = dbloc;
                        ptr.saute();
                        if(*ptr != smbl_while){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un 'while' �tait attendu � la fin d'une boucle 'do...while'\n";
                            exit(5);
                        }
                        if(*++ptr != '(' /*)*/){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' �tait attendu apr�s un 'while'\n" /*)*/;
                            exit(5);
                        }
                        ptr.saute();
                        break;
                    }
                } else if(svt == smbl_continue){ // Passe la main � la condition de sortie
                    ptr = dbloc;
                    ptr.saute();
                }

                if(*ptr != smbl_while){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un 'while' �tait attendu � la fin d'une boucle 'do...while'\n";
                    exit(5);
                }
                if(!*cond){
                    if(*++ptr != '(' /*)*/){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' �tait attendu apr�s un 'while'\n" /*)*/;
                        exit(5);
                    }
                    cond = ptr;
                } else
                    ptr = cond;

                ret = eval(ptr,tparent);

                if(!*suite) // On n'a pas encore valid� ce qui suit le 'while'
                    suite = ptr;

                if(!ret.nonnull(ptr)){ // Il faut sortir
                    ptr = suite;
                    break;
                }
            }
        }
        break;
    case smbl_while:{
            _token  exp,    // Condition du while
                    blk;    // Block � ex�cuter

            if(*++ptr != '(' /*)*/){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' �tait attendu apr�s un 'while'\n" /*)*/;
                exit(5);
            }
            exp = ++ptr;

            FOREVER {
                ptr = exp; // Evaluation de la condition
                ret = eval(ptr,tparent);

                if(!*blk){ // On a pas encore lu ce qui suit l'expression
                    if( /*(*/ *ptr != ')' ){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << /*(*/ ":Erreur de syntaxe, un ')' �tait attendu apr�s la condition d'un 'while'\n";
                        exit(5);
                    }
                    blk = ++ptr;
                } else
                    ptr = blk;

                if(!ret.nonnull(ptr))  // La condition est fausse
                    break;              // donc on sort

                int svt;
                ret = execbloc(ptr,tparent,svt);
                if(svt == smbl_break){ // break -> comme si la condition �tait fausse
                    ptr = blk;
                    break;
                } else if(svt == smbl_return){
                    inst = smbl_return;
                    return ret;
                }
            }
        }
        ptr.saute(); // On saute le bloc qui �tait � ex�cuter
        return _rep();
    case smbl_if:
        if(*++ptr != '(' /*)*/ ){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' �tait attendu apr�s un 'if'\n" /*)*/;
            exit(5);
        }
        ptr++;
        ret = eval(ptr,tparent); // Evaluation de la condition

        if(*ptr != /*(*/ ')' ){
            std::cerr << "*E* Ligne " << calcligne(ptr) << /*(*/":Erreur de syntaxe, un ')' �tait attendu apr�s la condition d'un 'if'\n";
            exit(5);
        }
        ptr++;
        if(ret.nonnull(ptr)){ // La condition est vrai
            ret = execbloc(ptr,tparent,inst);
            if(*ptr==smbl_else){
                ptr++;
                ptr.saute(); // Saute le bloc 'else'
            }
        } else {
            ptr.saute(); // Saute le proc 'if'
            if(*ptr==smbl_else){
                ptr++;
                ret = execbloc(ptr,tparent,inst);
            }
        }
        return ret; // ce n'est pas � 'if' de tester les 'break', 'continue',...
    case smbl_exit:
        if( *++ptr!='('/*)*/ ){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' �tait attendu apr�s un 'exit'\n" /*)*/;
            exit(5);
        }
        ret = eval(ptr,tparent); // Code de sortie
        ret = conv('I',ret,ptr);
        if(ret.val.entier)
            std::cerr << "*A* Sortie du programme: Code de sortie " << ret.val.entier << ".\n";
        exit(0);
    case '{':
        return execbloc(ptr,tparent,inst);
    case ';':
    case /* { */ '}': // C'est la fin d'un bloc
        ptr++;
        return _rep();
    default: // Le symbole n'est pas trouv�, c'est peut �tre une expression � ex�cuter
        eval(ptr,tparent);
    }

    if(*ptr == ';'){
        ptr++;
        return ret;
    }

    std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, ';' �tait attendu" << ADEB( " ('"<< ptr.obj() << "' lu)" << ) ".\n";
    exit(5);
}

static _rep execbloc(_token &ptr, _tablesmb *tparent, int &inst){
/* Execute un bloc de fonctions dont 'ptr' pointe sur le premier caract�re, � la fin,
 * il pointera sur le dernier.
 *  Si le bloc commence par un '{', l'ex�cution se terminera au '}' correspondant, sinon
 *  une seule instruction est ex�cut�e.
 *  <- 'tparent' est la table des symboles du bloc parent.
 *  -> non-void si la sortie du bloc se fait par un 'return' (sortie de la fonction).
 *  -> 'inst' contiendra le code de l'instruction qui a provoqu� la sortie du bloc
 *  (utile pour tester les 'break', 'return', 'continue',...).
 */
    _rep ret;

    if(*ptr == '{' /*}*/){
        _tablesmb table(tparent);   // Table pour ce bloc
        ptr++;

        FOREVER { // Lecture des variables de ce bloc
            std::string id, type;
            void *mem;

            if(lecdesc( ptr , id, type, 0, &mem,&table)){ // Lecture du premier id
                const char tbase = type[type.length()-1]; // Type de base de cette d�finition
                if(type[0]=='F'){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe." << ADEB("(exbloc:1)" << ) "\n";
                    exit(5);
                } else FOREVER {
                    _var *variable = ajoute_symbole(&table.var,id,type,ptr,mem); // Cr�ation de l'objet dans la table des symboles

                    if(*ptr == '='){ // Il y a une affectation imm�diate...
                        ptr++;
                        VarAffecte(variable, eval(ptr,&table,true),ptr);
                    }

                    if(*ptr == ','){ // Autre d�claration
                        ptr++;
                        if(!lecdesc(ptr, id, type, tbase , &mem,&table)){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, une d�claration �tait attendue.\n";
                            exit(5);
                        }

                        if(type[0]=='F'){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Une fonction ne peut �tre d�clar�e lors d'une d�finition multiple.\n";
                            exit(5);
                        }

                        // L'objet est ajout� � la table au d�but de la boucle
                    } else if(*ptr == ';'){ // C'est fini pour cette d�claration
                        ptr++;
                        break;
                    } else {
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe" << ADEB("(exbloc:2)" << ) ".\n";
                        exit(5);
                    }
                }
            } else if(*ptr == ';') // Il y a peut �tre une autre d�finition plus loin.
                ptr++;
            else
                break;
        }

#if DEBUG
        std::cerr << "*D* Liste des variables d'un bloc:\n";
        _var *smbl;
        for(smbl=table.var; smbl ; smbl = smbl->succ){
            std::cerr << "*D*\t" << smbl->nom << ":" << smbl->type << "(";
            switch(smbl->type[0]){
            case 'I':
                std::cerr << (int)((_var_int *)smbl)->val;
                break;
            case 'C':
                std::cerr << (int)((_var_char *)smbl)->val;
                break;
            case '*':
                std::cerr << (int)((_var_ptr *)smbl)->val;
                break;
            }
        std::cerr << ")\n";
        }
#endif

        do  // Execute les instructions de ce bloc
            ret = execinst(ptr, &table, inst);
        while(inst != /*{*/ '}' && !(inst >= smbl_break && inst <= smbl_return));

   } else // Une seule instruction � ex�cuter
        ret = execinst(ptr, tparent, inst);
    return ret;
}

_rep execfonc( _var_fonc *fonc, LFDynaStack<_rep> &args ){
/* Execute une fonction dont les arguments se trouvent dans 'args' */
    if(!fonc){
        std::cerr << "*F* Tentative d'ex�cuter une fonction NULLE.\n";
        exit(5);
    }

    _tablesmb table;    // Table des symboles locaux.
    std::string id,type;
    _token ptr((const char *)fonc->val);
    int numarg=0;

    if(lecdesc(ptr, id, type, 0, NULL,&table)){ // Lecture des param�tres
        if(type[0] == 'F'){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe (d�claration d'un argument de type 'fonction')." << ADEB("(fnc:1)" << ) "\n";
            exit(5);
        } else FOREVER {
            _var *var = ajoute_symbole(&table.var,id,type,ptr); // Cr�ation de la variable
            _rep param = args[numarg++]; // R�cup�ration de l'argument correspondant

            if(param.type() == 'V'){
                std::cerr << "*E* Fonction "<< fonc->nom << "(): Pas assez d'arguments ont �t� fournis.\n";
                exit(5);
            }
            VarAffecte(var,param,ptr);

            if(*ptr == ','){ // encore un/des param�tre(s)
                ptr++;

                if(!lecdesc(ptr, id, type, 0, NULL,&table)){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, une d�claration �tait attendue.\n";
                    exit(5);
                }

                if(type[0]=='F'){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe(d�claration d'un argument de type 'fonction')." << ADEB("(fnc:2)" << ) "\n";
                    exit(5);
                }

                // Le reste est fait au d�but de la boucle
            } else if(*ptr == /* ( */ ')') // Fin de lecture des arguments
                break;
            else {
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe." << ADEB("(fnc:3)" << ) "\n";
                exit(5);
            }
        }
    } else if(*ptr != /*(*/ ')'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << /* ( */": Erreur de syntaxe, ')' �tait attendue. '"
            << ptr.obj() << "' lu.\n";
        exit(5);
    }
    ptr++; // On passe la parenth�se

    if( args.length() != numarg-1 )
        std::cerr << "*A* Fonction "<< fonc->nom << "(): Trop d'arguments ont �t� fournis.\n";

    DEB(
        std::cerr << "*D* Liste des arguments pour la fonction "<< fonc->nom << "():\n";
        _var *smbl;
        for(smbl=table.var; smbl ; smbl = smbl->succ){
            std::cerr << "*D*\t" << smbl->nom << ":" << smbl->type << "(";
            switch(smbl->type[0]){
            case 'I':
                std::cerr << (int)((_var_int *)smbl)->val;
                break;
            case 'C':
                std::cerr << (int)((_var_char *)smbl)->val;
                break;
            case '*':
                std::cerr << (int)((_var_ptr *)smbl)->val;
                break;
            }
            std::cerr << ")\n";
        }
    );

    if(*ptr != '{' /* } */ ){
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, un '{' �tait attendu, '" << ptr.obj() << "' lu.\n"; /* } */
        exit(5);
    }

    int token_sortie; // Token qui a provoqu� la sortie
    _rep ret= execbloc(ptr,&table,token_sortie); // Ex�cution du bloc
    switch(token_sortie){
    case smbl_break:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ":<break> inattendu.\n";
        exit(5);
    case smbl_continue:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ":<continue> inattendu.\n";
        exit(5);
    }

    if(fonc->type[1] != 'V' && ret.type() == 'V'){ // On retourne quant m�me car c'est peut �tre une fonction 'void' qui s'ignore...
        std::cerr << "*A* Ligne " << calcligne(ptr) << ", fonction " << fonc->nom << "() : Pas de valeur de retour dans une fonction non-'void'.\n";
        return ret;
    }

    if(fonc->type[1] == 'V' && ret.type() == 'V') return ret;

    if(fonc->type[1]==ret.type())
        return ret;
    else
        return conv(fonc->type[1], ret, ptr);  // Retourne apr�s avoir fait une conversion vers le type de la fonction
}

