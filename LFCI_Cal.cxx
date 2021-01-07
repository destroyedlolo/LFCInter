/***************************************************************************\
*  LFCI_cal.cxx                                                             *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Evaluation d'une expression.                                             *
*                                                                           *
 ***************************************************************************
*                                                                           *
*   BNF (théorique) pour évaluer une expression C                           *
*                                                                           *
* (-11) expression ::= exp_affect {[, exp_affect ]}                         *
*   Le résultat de l'expression est la valeur de la derniere exp_affect.    *
*                                                                           *
* (-10) exp_affect ::= exp_un opp_aff exp_affect                            *
*                  ::= expr_cond                                            *
*                                                                           *
*   avec opp_aff '=','*=','/=','%=','+=','-=','&=','^=','|=','<<=','>>='    *
*        exp_un doit être une référence.                                    *
*                                                                           *
* (-9)  expr_cond ::= exp_OU_log ? exp_OU_log : expr_cond                   *
*                 ::= exp_OU_log                                            *
*                                                                           *
* (-8)  exp_OU_log ::= exp_OU_log || exp_ET_log                             *
*                  ::= exp_ET_log                                           *
*                                                                           *
* (-7)  exp_ET_log ::= exp_ET_log && exp_OU_incl                            *
*                  ::= exp_OU_incl                                          *
*                                                                           *
* (-6)  exp_OU_incl ::= exp_OU_incl | exp_OU_excl                           *
*                   ::= exp_OU_excl                                         *
*                                                                           *
* (-5)  exp_OU_excl ::= exp_OU_excl ^ exp_ET                                *
*                   ::= exp_ET                                              *
*                                                                           *
* (-4)  exp_ET ::= exp_ET & exp_egal                                        *
*              ::= exp_egal                                                 *
*                                                                           *
* (-3)  exp_egal ::= exp_egal == exp_rel                                    *
*                ::= exp_egal != exp_rel                                    *
*                ::= exp_rel                                                *
*                                                                           *
* (-2)  exp_rel ::= exp_rel opp_rel exp_dcl                                 *
*               ::= exp_dcl                                                 *
*                                                                           *
*   avec opp_rel '<','>','<=','>='                                          *
*                                                                           *
* (-1)  exp_dcl ::= exp_dcl >> exp_add                                      *
*               ::= exp_dcl << exp_add                                      *
*               ::= exp_add                                                 *
*                                                                           *
* (0)   exp_add ::= exp_add + exp_mul                                       *
*               ::= exp_add - exp_mul                                       *
*               ::= exp_mul                                                 *
*                                                                           *
* (1)   exp_mul ::= exp_mul * exp_un                                        *
*               ::= exp_mul / exp_un                                        *
*               ::= exp_mul % exp_un                                        *
*               ::= exp_un                                                  *
*                                                                           *
* (2)   exp_un ::= opp_un exp_un                                            *
*              ::=  exp_posf                                                *
*                                                                           *
*   avec opp_un '++','--','&','*','+','-','~','!'                           *
*                                                                           *
* (3)   exp_posf ::= exp_prim                                               *
*                ::= exp_posf++                                             *
*                ::= exp_posf--                                             *
*                ::= exp_posf [expr_cond]{[expr_cond]}                      *
*                                           Valeur d'un tableau             *
*                                                                           *
* (4)   exp_prim ::= valeur_littéral                                        *
*                ::= variable                                               *
*                ::= fonction (...)         Appel de fonction               *
*                ::= ( exp_affect )                                         *
*                                                                           *
*    Cette BNF est INSPIREE de celle fournie avec le BORLAND (ansi ?) avec  *
*   quelques modifications:                                                 *
*       - l'opérateur 'sizeof' est traité comme une fonction et DOIT avoir  *
*   son argument entre parenthèses -> Il a donc la même priorité que les    *
*   autres fonctions...                                                     *
*       - L'appel de fonction fait partie exp_prim (priorité la plus haute) *
*   au lieu de exp_posf: Le tableau des priorités du BORLAND indique une    *
*   priorité des appels sur les expressions post-fixé mais pas la BNF (!)   *
*   qui les place au même niveau => Erreur dans BNF !                       *
*       - La BNF place les [] et les opérateurs postfixés à la même         *
*   priorité. le tableau des associativités ne distingue pas les opérateurs *
*   préfixés des postfixés '++' et '--' et les places donc à une priorité   *
*   inférieure par rapport à []. A mon avis, ce doit être une erreur de ce  *
*   tableau: Tout les opérateurs post-fixés, y compris [], doivent avoir la *
*   même priorité et être lus de gauche à droite...                         *
*                                                                           *
*   J'aurai préféré utiliser le BNF de GCC mais je n'ai pas réussi à la     *
*   trouver (bon, c'est vrai que je n'ai pas trop cherché!). Je n'ai pas    *
*   non plus vérifié certaines priorités ... étranges genre que les         *
*   opérateurs <, <=, >=, ou > ont priorité sur == & != ou que && a         *
*   priorité sur ||.                                                        *
*                                                                           *
*    Dans le code, plutôt que d'utiliser un code adapté de cette BNF, j'ai  *
*   utilisé un remix entre la méthode des priorités des opérateurs et       *
*   l'utilisation d'une "BNF restreinte". C'est plus simple à programmer, à *
*   maintenir et, surtout, c'est beaucoup plus rapide !                     *
*                                                                           *
*    Voici donc la BNF restreinte utilisée dans ce programme:               *
*                                                                           *
*       reponse ::= exp_aff {, exp_aff}                                     *
*   codé dans la fonction eval().                                           *
*                                                                           *
*       exp_aff ::= reférence {oppaff exp_cond}                             *
*       exp_aff ::= exp_cond                                                *
*   codé dans la fonction affectation().                                    *
*                                                                           *
*       exp_cond ::= exp_binaire ? exp_cond : exp_cond                      *
*       exp_cond ::= exp_binaire                                            *
*   codé dans la fonction conditionnel().                                   *
*                                                                           *
*       exp_binaire ::= exp_un {[opérateur_binaire exp_un]}                 *
*   codé dans la fonction binaire(). Cette fonction gère elle-même la       *
*   priorité des opérateurs binaires.                                       *
*                                                                           *
*       exp_un ::= [{pré-opérateur}] exp_posf                               *
*   codé dans la fonction lectunaire().                                     *
*                                                                           *
*       exp_posf ::= exp_prim [{post-opérateur}]                            *
*   codé dans la fonction lectunaire().                                     *
*                                                                           *
*       exp_prim ::= valeur_littéral                                        *
*                ::= variable                                               *
*                ::= fonction (...)         Appel de fonction               *
*                ::= ( exp_affect )                                         *
*   codé dans la fonction CalcLexer().                                      *
*                                                                           *
*       ON N'EST PAS CHEZ MICRO-SUCKER, l'optimisation, ça existe !!        *
*                                                                           *
*    Le type de valeur 'R' pour 'référence', qui est physiquement un        *
*   pointeur, est utilisé pour représenter les 'lvalues':                   *
*       - Si l'opérateur suivant est une affectation, la valeur pointée     *
*       est modifiée (affectation),                                         *
*       - Sinon, la valeur pointée est placée dans la pile...               *
*                                                                           *
*    Le type de valeur 'X', basé sur le type 'I', contient la valeur d'un   *
*   token lu par la fonction CalcLexer().                                   *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#include "LFCInter.h"
#include "LFCI_icl.h"

	/*
	 *   Stockage des chaînes lues dans le source. Grâce à ceci, une chaîne
	 *  n'est décodée qu'une seule fois si le source repasse au même endroit...
	 */

static struct _stkchaine : public std::string {
	_stkchaine(const char *aptr) : ptr(aptr){};

	const char *ptr; // Valeur du pointeur de début de la chaîne
	const char *apres; // Valeur du pointeur après lecture de la chaîne
	_stkchaine *suivant;
} *premier = NULL;

_rep lectcar(const char **ptr){
/*  Ne lit qu'un seul caractère placé entre ''
 *  <- ptr pointe sur caractère lui-même
 *  -> ptr pointe sur le dernier caractère pris en compte
 */
	if(**ptr == '\n' || !*ptr){
		std::cerr << "*E* Ligne " << calcligne(*ptr) << ": Déclaration de caratère non terminée.\n";
		exit(5);
	}

	if(**ptr == '\\'){ // Caractère qualifié
		++*ptr; // Saute '\'
		switch(**ptr){
		case '\n':
		case 0:
			std::cerr << "*E* Ligne " << calcligne(*ptr) << ": Déclaration de caratère non terminée.\n";
			exit(5);
		case 'a':   // BEL
			return (char)'\a';
		case 'b':   // BS
			return (char)'\b';
		case 'f':   // FF
			return (char)'\f';
		case 'n':   // LF
			return (char)'\n';
		case 'r':   // CR
			return (char)'\r';
		case 't':   // HT
			return (char)'\t';
		case 'v':   // VT
			return (char)'\v';
		case 'x':
		case 'X':{  // Lecture d'un caractère mis sous forme héxa
				unsigned char i=0;
				while( isxdigit(*++*ptr) ){
					i *= 16;
					if(isdigit(**ptr))
						i += **ptr - '0';
					else
						i += tolower(**ptr) - 'a' + 10;
				}
				--*ptr; // Retour sur le dernier caractère prise en compte
				return (char) i;
			}
		default:
			if(isodigit(**ptr)){ // Peut être une constante octale
				unsigned char i=0;

				do
					i = i*8 + **ptr - '0';
				while( isodigit(*++*ptr) );

				--*ptr; // Retour sur le dernier caractère prise en compte
				return (char)i;
			} else // Juste un caractère qui ne doit pas être interprété
				return **ptr;
		}
	} else if(**ptr == '\''){ // Lecture d'un caractère nul ''
		--*ptr; // Pas de caractère pris en compte, mais on doit quant même revenir en arriere!
		return (char)0;
	} else // Simplement un caractère
		return **ptr;
}

static _rep trouvechaine(_token &ptr){
/* Lecture d'une chaîne: Si ptr fait déjà partie de la liste, la chaîne stockée
 * est renvoyée et ptr prend la valeur 'après'
 */
	_stkchaine *t;

	for(t=premier;t;t=t->suivant){
#ifdef __BCPLUSPLUS__
		if(std::string(t->ptr) == ptr){ // La chaîne a déjà été stockée
#else
		if(t->ptr == ptr){ // La chaîne a déjà été stockée
#endif
			ptr = t->apres;
			return _rep((void *)t->c_str(),'*',"C");
		}
	}

		// C'est une nouvelle chaîne
	t = new _stkchaine(ptr);
	if(!t){
		std::cerr << "*F* Ligne " << calcligne(ptr) << ": Manque de mémoire pour stocker une chaîne.\n";
		exit(10);
	}

tchn_autre:
	const char *x=ptr;

	for(++x; *x!='"'; ++x){
		if(*x=='\n' || !*x){
			std::cerr << "*E* Ligne " << calcligne(x) << ": Chaîne non terminée " << ADEB("(trouvechaine:1)" << ) ".\n";
			exit(5);
		}

		if(*x == '\\'){
			*t += lectcar(&x).val.caractere; // Stochage du caractère
			continue;
		}
		*t += *x;
	}

		// Recherche si la chaîne se poursuit "ch1" "ch2" -> "ch1ch2"
	ptr = x;
	ptr++; // Saute le '"' fermant

	if(*ptr == '"')
		goto tchn_autre;

	t->apres = ptr;
	t->suivant = premier;
	premier = t;

	return _rep((void *)t->c_str(),'*',"C");
}

	/*  Au lieu d'utiliser des valeurs par défaut pour les arguments optionnels,
	 * rch_op() est surchargé pour accélérer sont fonctionnement !
	 */

static const _operateur *rch_op(int op){
/*  Recherche la PREMIERE structure associée avec l'opérateur passé en argument
 *  -> NULL en cas d'erreur
 */
	int i;

	for(i=0; operateur[i].op; i++)
		if(operateur[i].op == op)
			return &operateur[i];

	return NULL;
}

static const _operateur *rch_op(int op, bool unaire){
/*  Recherche la structure associée avec l'opérateur passé en argument
 * tient compte si l'operateur est unaire ou binaire.
 *  -> NULL en cas d'erreur
 */
	int i;

	for(i=0; operateur[i].op; i++)
		if(operateur[i].op == op && unaire == operateur[i].unaire)
			return &operateur[i];

	return NULL;
}

static const _operateur *rch_op(int op, bool unaire, bool devant){
/*  Recherche la structure associée avec l'opérateur passé en argument
 * tient compte si l'operateur est unaire ou binaire, sa position (uniquement
 * pour les opérateurs unaires).
 *  -> NULL en cas d'erreur
 */
	int i;

	for(i=0; operateur[i].op; i++)
		if(operateur[i].op == op && unaire == operateur[i].unaire){
			if( unaire && operateur[i].devant == devant )
				return &operateur[i];
		}

	return NULL;
}

static _rep CalcLexer(_token &ptr, _tablesmb *table_locale, bool sansfonc=false){
/* Cette fonction est une interface entre les méthodes de _token et eval(),
 * elle lit l'élément suivant de l'expression et renvoie:
 *  <- un 'L' si c'est une valeur litérale,
 *  <- si c'est une variable, une référence sur celle-ci,
 *  <- la valeur de retour dans le cas d'une fonction (après l'avoir appelée)
 *  <- un 'X' s'il s'agit d'un token ou d'un autre caractère
 *
 *  -> ptr est incrémenté et len contiendra la longueur du token lu suivant les
 * même règles que dans LFCI_Lex/trouvetoken(); sauf que ptr est directement
 * incrémenté s'il s'agit d'un symbole qui est lu.
 *  -> sansfonc == true si les éventuelles fonctions trouvées provoquent une
 * erreur au lieu d'être exécutée.
 */

	if(*ptr == smbl_id){ // On lit un identifieur
		_var *symbole = trouve_symbole(table_locale, ptr.obj());

		if(!symbole){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Symbole inconnu '" << ptr.obj() << "'\n";
			exit(5);
		}

		ptr++;
		if(symbole->type[0] == 'F'){
				// Si le symbole suivant est un '(' /*)*/ il faut exécuter la fonction
			if(*ptr=='(' /*)*/ ){
				if(sansfonc){
					std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe pour '" << ptr.obj() << "' !\n";
					exit(5);
				} else { // C'est une fonction: Il faut l'exécuter
					ptr++;
					return lancefonc(ptr, (_var_fonc *)symbole, table_locale);
				}
			} else
				return symbole->repval(ptr); // Sinon on renvoie la représentation de la fonction
		}
		return _rep(symbole->refval(), 'R', symbole->type); // C'est une variable, on renvoie sa référence
	} else if(*ptr == '0'){
		long i=0;
		const char *p=ptr;
		p++; // Ne pas mettre le ++ dans le tolower() car cette macro peut utiliser plusieurs fois son argument
		if(tolower(*p) == 'x'){ // Lecture d'un nombre hexa
			while(isxdigit(*++p)){
				i *= 16;
				if(isdigit(*p))
					i += *p - '0';
				else
					i += tolower(*p) - 'a' + 10;
			}
		} else { // Lecture d'un octal
			while( isodigit(*p) ){
				i = i*8 + *p - '0';
				p++;
			}
		}
		ptr = p;
		return _rep(i);
	} else if(isdigit(*ptr)){ // On lit une valeur numérique
		long i=0;
		const char *p = ptr;

		do {
			i = i*10 + *p -'0';
		} while( isdigit(*++p) );
		ptr = p;
		return _rep(i);
	} else if(*ptr == '"') // On lit une chaîne
		return trouvechaine(ptr);
	else if(*ptr == '\''){ // Lecture d'un caractère
		const char *x = ptr; // On ne peut pas utiliser _token sinon, par exemple, les espaces seront sautés
		x++; // Saute le "'"
		_rep t=lectcar(&x);
		if(*++x != '\''){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de trouver le \"'\" fermant !\n";
			exit(5);
		}
		ptr=++x;
		return t;
	} else if(*ptr == '(' /*)*/ ){ // Parenthèses
		ptr++;
		_rep t=eval(ptr, table_locale);
		ptr++; // Saute le #(# ')' de fermeture
		return t;
	} else if( ptr.interne() )
		if(sansfonc){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe pour '" << ptr.obj() << "' !\n";
			exit(5);
		} else
			return interne(ptr,table_locale);
	else { // C'est un autre caractère
		_rep t = _rep(*ptr,'X');
		ptr++;
		return t;
	}
}

static _rep lectunaire(_token &ptr, _tablesmb *table_locale){
/* Lecture d'une valeur "unaire".
 * A la sortie, ptr pointe sur la premiere valeur non reconnue
 * Voir eval() pour plus d'informations sur les arguments.
 *
 *  BNF restreinte de l'expression attendu par cette fonction:
 *
 *      reponse ::= [{pré-opérateur}] exp_prim [{post-opérateur}]
 *
 *  notez que cette fonction s'attend à lire au moins exp_prim !
 *
 *  Comme tous les opérateurs unaires ont la même priorité et sont évalués de
 *  droite à gauche, il n'y a qu'à les pousser dans la pile et effectuer les
 *  calculs lors du dépilement.
 */
	LFDynaStack<_rep> valeurs;  // Pile des valeurs
	LFDynaStack<const _operateur *> operateurs;   // Pile des opérateurs

	_rep suivant=CalcLexer(ptr,table_locale);

	while(suivant.type() == 'X'){ // Lecture des pré-opérateurs unaires
		const _operateur *x = rch_op(suivant.val.entier,true,true);

		if(!x){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Le symbole avant '" << ptr.obj() << "' provoque une erreur de syntaxe.\n";
			exit(5);
		}

		operateurs.Push(x); // Tous les opérateurs unaires de devant ont les mêmes priorités

		suivant=CalcLexer(ptr,table_locale);
	}

	valeurs.Push(suivant);
	_token t;

	FOREVER{    // Lecture des post-opérateurs unaires
		t=ptr;
		suivant=CalcLexer(ptr,table_locale,true);

		if(suivant.type()!='X'){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe lors d'un post-operateur unaire ("<< ptr.obj() << ").\n";
			exit(5);
		}

		const _operateur *x = rch_op(suivant.val.entier,true,false);

		if(!x){ // Ce n'est plus un post-opérateur
			ptr = t; // Il faut donc restorer le pointeur
			break;
		}

		if(x->fonc) // C'est un opérateur
			valeurs.Push(x->fonc(valeurs,ptr));
		else {  // c'est un [] => resultat = *(adr + index)
			valeurs.Push(conv('I',eval(ptr,table_locale),ptr));
			if(*ptr != /*[*/ ']'){
				std::cerr << "*E* Ligne " << calcligne(ptr) << /*[*/ ": Impossible de trouver le ']' fermant.\n";
				exit(5);
			}
			ptr++;
			valeurs.Push(plus(valeurs,ptr));
			valeurs.Push(deref(valeurs,ptr));
		}
	}

	while(operateurs.length()>=0) // Exécution des calculs
		valeurs.Push(operateurs.Pop()->fonc(valeurs,ptr));

	DEB(
		if(valeurs.length()){ // Il ne doit en rester qu'un (highlander ?)
			std::cerr << "*F* Ligne " << calcligne(ptr) << ": Il reste plus d'une valeur dans la pile à la sortie de LFCI_cal/lectunaire().\n";
			exit(20);
		}
	);
	return valeurs.Pop();
}

static _rep binaire(_token &ptr, _tablesmb *table_locale){
/* Lecture d'une valeur "binaire".
 * Voir eval() pour plus d'informations sur les arguments.
 *
 *  BNF restreinte de l'expression attendue par cette fonction:
 *
 *      reponse ::= exp_un {[operateur exp_un]}
 *
 */
	LFDynaStack<_rep> valeurs;  // Pile des valeurs
	LFDynaStack<const _operateur *> operateurs;   // Pile des opérateurs

	FOREVER{
		valeurs.Push(lectunaire(ptr,table_locale));

		const _operateur *x = rch_op(*ptr,false);
		if(!x) break;
		ptr++; // L'opérateur est accepté, on passe au symbole suivant...

	autres:
		if(operateurs.length()>=0){ // Il y a déjà quelque chose dans la pile des opérateurs
			if(operateurs[operateurs.length()]->priorite >= x->priorite){ // Il a priorité: On l'exécute
				valeurs.Push(operateurs.Pop()->fonc(valeurs,ptr));
				goto autres; // Peut-être que l'opérateur précédent est aussi à exécuter.
			}
		}
		operateurs.Push(x);
	}

	while(operateurs.length()>=0) // Exécution des calculs
		valeurs.Push(operateurs.Pop()->fonc(valeurs,ptr));

	DEB(
		if(valeurs.length()){ // Il ne doit en rester qu'un
			std::cerr << "*F* Ligne " << calcligne(ptr) << ": Il reste plus d'une valeur dans la pile à la sortie de LFCI_cal/binaire().\n";
			exit(20);
		}
	);

	return valeurs.Pop();
}

_rep conditionnel(_token &ptr, _tablesmb *table_locale){
/* Lecture d'une valeur "conditionnelle".
 * Voir eval() pour plus d'informations sur les arguments.
 *
 *  BNF restreinte de l'expression attendu par cette fonction:
 *       reponse ::= exp_binaire ? exp_cond : exp_cond
 *       reponse ::= exp_binaire
 */
	_rep t = binaire(ptr,table_locale);

	return t;
}

_rep eval(_token &ptr, _tablesmb *table_locale, bool un){
/* Evaluation d'une expression.
 * <- 'ptr' doit pointer sur le début de l'expression
 *    table_locale est la table actuelle des symboles (NULL= uniquement la table globale)
 *    'un' est vrai si une seule expression doit être lue, la présence d'un ','
 *    ne provoque pas la lecture d'autres valeurs (par exemple dans le cas de
 *    définitions de variables...). Note: Si 'un = 'false', la présence d'un ';'
 *    comme premier caractère lu provoque le retour d'un void.
 *
 *  -> Renvoie la valeur représentant le resultat de l'expression
 *     'ptr' pointe sur le premier caractère qui provoque une erreur de l'expression
 *     (caractère inconnu, s'il s'agit d'une erreur de synstaxe de calcul genre
 *      5 + %3, il y a directement une erreur).
 *
 *  BNF restreinte de l'expression attendu par cette fonction:
 *
 *      reponse ::= exp_aff {, exp_aff}
 *
 */
	_rep t;

	if(!un && *ptr == ';') // C'est possible: Lecture de l'argument de return;
		return _rep();

	t = affectation(ptr,table_locale);

	while(*ptr == ',' && !un){ // C'est un 'calcul multiple'
		ptr++; // Saute le ','
		t = affectation(ptr,table_locale);
	}

	return t;
}
