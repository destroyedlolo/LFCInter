/***************************************************************************\
*  LFCI_exe.cxx                                                             *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Execution des instructions.                                              *
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

static _rep execbloc(_token &ptr, _tablesmb *tparent, int &inst);

static _rep execinst(_token &ptr, _tablesmb *tparent, int &inst){
/* Execute une instruction.
 * Voir execbloc() pour les paramêtres.
 *  -> ne retourne qu'une valeur 'void' sauf dans le cas de l'instruction return
 * où la valeur de retour est retournée.
 *  -> 'inst' contiendra le code de l'instruction exécutée.
 *
 * Note: Pour accélérer les choses, pour les instructions répétitives comme les
 *  boucles, la vérification de la syntaxe n'est faite qu'une seule fois. Les
 *  informations comme les conditions de sortie sont stockés à la premiere passe...
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
                    exp,  // Expression d'itération
                    dbloc; // Bloc à exécuter

            if(*++ptr != '(' /*)*/){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu après un 'for'\n" /*)*/;
                exit(5);
            }

            ptr++;
            eval(ptr,tparent); // Evaluation de l'expression initiale
            if(*ptr++ != ';'){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, ';' était attendu" << ADEB( " ('"<< ptr.obj() << "' lu)" << ) ".\n";
                exit(5);
            }
            cond=ptr;
            ptr.saute(); // Saute la condition de sortie
            exp=ptr;
            ptr.saute( /*(*/ ")" ); // Saute l'expréssion d'itération
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
     * car 'continue' doit passer le contrôle à l'expression d'incrémentation
     * ... qui se trouve juste ici dans ce source !
     */
                ptr=exp;
                if(*ptr != /*(*/ ')' ) // Y a t-il une expression
                    eval(ptr,tparent); // Evaluation de l'expression d'itération
            }
        }
        return _rep();
    case smbl_do:{
            _token dbloc=++ptr, // Début du bloc à exécuter
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
                    } else { // Il faut déterminer où sauter
                        ptr = dbloc;
                        ptr.saute();
                        if(*ptr != smbl_while){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un 'while' était attendu à la fin d'une boucle 'do...while'\n";
                            exit(5);
                        }
                        if(*++ptr != '(' /*)*/){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu après un 'while'\n" /*)*/;
                            exit(5);
                        }
                        ptr.saute();
                        break;
                    }
                } else if(svt == smbl_continue){ // Passe la main à la condition de sortie
                    ptr = dbloc;
                    ptr.saute();
                }

                if(*ptr != smbl_while){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un 'while' était attendu à la fin d'une boucle 'do...while'\n";
                    exit(5);
                }
                if(!*cond){
                    if(*++ptr != '(' /*)*/){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu après un 'while'\n" /*)*/;
                        exit(5);
                    }
                    cond = ptr;
                } else
                    ptr = cond;

                ret = eval(ptr,tparent);

                if(!*suite) // On n'a pas encore validé ce qui suit le 'while'
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
                    blk;    // Block à exécuter

            if(*++ptr != '(' /*)*/){
                std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu après un 'while'\n" /*)*/;
                exit(5);
            }
            exp = ++ptr;

            FOREVER {
                ptr = exp; // Evaluation de la condition
                ret = eval(ptr,tparent);

                if(!*blk){ // On a pas encore lu ce qui suit l'expression
                    if( /*(*/ *ptr != ')' ){
                        std::cerr << "*E* Ligne " << calcligne(ptr) << /*(*/ ":Erreur de syntaxe, un ')' était attendu après la condition d'un 'while'\n";
                        exit(5);
                    }
                    blk = ++ptr;
                } else
                    ptr = blk;

                if(!ret.nonnull(ptr))  // La condition est fausse
                    break;              // donc on sort

                int svt;
                ret = execbloc(ptr,tparent,svt);
                if(svt == smbl_break){ // break -> comme si la condition était fausse
                    ptr = blk;
                    break;
                } else if(svt == smbl_return){
                    inst = smbl_return;
                    return ret;
                }
            }
        }
        ptr.saute(); // On saute le bloc qui était à exécuter
        return _rep();
    case smbl_if:
        if(*++ptr != '(' /*)*/ ){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu après un 'if'\n" /*)*/;
            exit(5);
        }
        ptr++;
        ret = eval(ptr,tparent); // Evaluation de la condition

        if(*ptr != /*(*/ ')' ){
            std::cerr << "*E* Ligne " << calcligne(ptr) << /*(*/":Erreur de syntaxe, un ')' était attendu après la condition d'un 'if'\n";
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
        return ret; // ce n'est pas à 'if' de tester les 'break', 'continue',...
    case smbl_exit:
        if( *++ptr!='('/*)*/ ){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, un '(' était attendu après un 'exit'\n" /*)*/;
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
    default: // Le symbole n'est pas trouvé, c'est peut être une expression à exécuter
        eval(ptr,tparent);
    }

    if(*ptr == ';'){
        ptr++;
        return ret;
    }

    std::cerr << "*E* Ligne " << calcligne(ptr) << ":Erreur de syntaxe, ';' était attendu" << ADEB( " ('"<< ptr.obj() << "' lu)" << ) ".\n";
    exit(5);
}

static _rep execbloc(_token &ptr, _tablesmb *tparent, int &inst){
/* Execute un bloc de fonctions dont 'ptr' pointe sur le premier caractère, à la fin,
 * il pointera sur le dernier.
 *  Si le bloc commence par un '{', l'exécution se terminera au '}' correspondant, sinon
 *  une seule instruction est exécutée.
 *  <- 'tparent' est la table des symboles du bloc parent.
 *  -> non-void si la sortie du bloc se fait par un 'return' (sortie de la fonction).
 *  -> 'inst' contiendra le code de l'instruction qui a provoqué la sortie du bloc
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
                const char tbase = type[type.length()-1]; // Type de base de cette définition
                if(type[0]=='F'){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe." << ADEB("(exbloc:1)" << ) "\n";
                    exit(5);
                } else FOREVER {
                    _var *variable = ajoute_symbole(&table.var,id,type,ptr,mem); // Création de l'objet dans la table des symboles

                    if(*ptr == '='){ // Il y a une affectation immédiate...
                        ptr++;
                        VarAffecte(variable, eval(ptr,&table,true),ptr);
                    }

                    if(*ptr == ','){ // Autre déclaration
                        ptr++;
                        if(!lecdesc(ptr, id, type, tbase , &mem,&table)){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, une déclaration était attendue.\n";
                            exit(5);
                        }

                        if(type[0]=='F'){
                            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Une fonction ne peut être déclarée lors d'une définition multiple.\n";
                            exit(5);
                        }

                        // L'objet est ajouté à la table au début de la boucle
                    } else if(*ptr == ';'){ // C'est fini pour cette déclaration
                        ptr++;
                        break;
                    } else {
                        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe" << ADEB("(exbloc:2)" << ) ".\n";
                        exit(5);
                    }
                }
            } else if(*ptr == ';') // Il y a peut être une autre définition plus loin.
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

   } else // Une seule instruction à exécuter
        ret = execinst(ptr, tparent, inst);
    return ret;
}

_rep execfonc( _var_fonc *fonc, LFDynaStack<_rep> &args ){
/* Execute une fonction dont les arguments se trouvent dans 'args' */
    if(!fonc){
        std::cerr << "*F* Tentative d'exécuter une fonction NULLE.\n";
        exit(5);
    }

    _tablesmb table;    // Table des symboles locaux.
    std::string id,type;
    _token ptr((const char *)fonc->val);
    int numarg=0;

    if(lecdesc(ptr, id, type, 0, NULL,&table)){ // Lecture des paramêtres
        if(type[0] == 'F'){
            std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe (déclaration d'un argument de type 'fonction')." << ADEB("(fnc:1)" << ) "\n";
            exit(5);
        } else FOREVER {
            _var *var = ajoute_symbole(&table.var,id,type,ptr); // Création de la variable
            _rep param = args[numarg++]; // Récupération de l'argument correspondant

            if(param.type() == 'V'){
                std::cerr << "*E* Fonction "<< fonc->nom << "(): Pas assez d'arguments ont été fournis.\n";
                exit(5);
            }
            VarAffecte(var,param,ptr);

            if(*ptr == ','){ // encore un/des paramêtre(s)
                ptr++;

                if(!lecdesc(ptr, id, type, 0, NULL,&table)){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, une déclaration était attendue.\n";
                    exit(5);
                }

                if(type[0]=='F'){
                    std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe(déclaration d'un argument de type 'fonction')." << ADEB("(fnc:2)" << ) "\n";
                    exit(5);
                }

                // Le reste est fait au début de la boucle
            } else if(*ptr == /* ( */ ')') // Fin de lecture des arguments
                break;
            else {
                std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe." << ADEB("(fnc:3)" << ) "\n";
                exit(5);
            }
        }
    } else if(*ptr != /*(*/ ')'){
        std::cerr << "*E* Ligne " << calcligne(ptr) << /* ( */": Erreur de syntaxe, ')' était attendue. '"
            << ptr.obj() << "' lu.\n";
        exit(5);
    }
    ptr++; // On passe la parenthèse

    if( args.length() != numarg-1 )
        std::cerr << "*A* Fonction "<< fonc->nom << "(): Trop d'arguments ont été fournis.\n";

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
        std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe, un '{' était attendu, '" << ptr.obj() << "' lu.\n"; /* } */
        exit(5);
    }

    int token_sortie; // Token qui a provoqué la sortie
    _rep ret= execbloc(ptr,&table,token_sortie); // Exécution du bloc
    switch(token_sortie){
    case smbl_break:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ":<break> inattendu.\n";
        exit(5);
    case smbl_continue:
        std::cerr << "*E* Ligne " << calcligne(ptr) << ":<continue> inattendu.\n";
        exit(5);
    }

    if(fonc->type[1] != 'V' && ret.type() == 'V'){ // On retourne quant même car c'est peut être une fonction 'void' qui s'ignore...
        std::cerr << "*A* Ligne " << calcligne(ptr) << ", fonction " << fonc->nom << "() : Pas de valeur de retour dans une fonction non-'void'.\n";
        return ret;
    }

    if(fonc->type[1] == 'V' && ret.type() == 'V') return ret;

    if(fonc->type[1]==ret.type())
        return ret;
    else
        return conv(fonc->type[1], ret, ptr);  // Retourne après avoir fait une conversion vers le type de la fonction
}

