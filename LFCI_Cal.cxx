/***************************************************************************\
*  LFCI_cal.cxx                                                             *
*  Projet : LFCInter                                                        *
*      � LFSoft 1995-96                                                     *
*                                                                           *
*  Evaluation d'une expression.                                             *
*                                                                           *
 ***************************************************************************
*                                                                           *
*   BNF (th�orique) pour �valuer une expression C                           *
*                                                                           *
* (-11) expression ::= exp_affect {[, exp_affect ]}                         *
*   Le r�sultat de l'expression est la valeur de la derniere exp_affect.    *
*                                                                           *
* (-10) exp_affect ::= exp_un opp_aff exp_affect                            *
*                  ::= expr_cond                                            *
*                                                                           *
*   avec opp_aff '=','*=','/=','%=','+=','-=','&=','^=','|=','<<=','>>='    *
*        exp_un doit �tre une r�f�rence.                                    *
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
* (4)   exp_prim ::= valeur_litt�ral                                        *
*                ::= variable                                               *
*                ::= fonction (...)         Appel de fonction               *
*                ::= ( exp_affect )                                         *
*                                                                           *
*    Cette BNF est INSPIREE de celle fournie avec le BORLAND (ansi ?) avec  *
*   quelques modifications:                                                 *
*       - l'op�rateur 'sizeof' est trait� comme une fonction et DOIT avoir  *
*   son argument entre parenth�ses -> Il a donc la m�me priorit� que les    *
*   autres fonctions...                                                     *
*       - L'appel de fonction fait partie exp_prim (priorit� la plus haute) *
*   au lieu de exp_posf: Le tableau des priorit�s du BORLAND indique une    *
*   priorit� des appels sur les expressions post-fix� mais pas la BNF (!)   *
*   qui les place au m�me niveau => Erreur dans BNF !                       *
*       - La BNF place les [] et les op�rateurs postfix�s � la m�me         *
*   priorit�. le tableau des associativit�s ne distingue pas les op�rateurs *
*   pr�fix�s des postfix�s '++' et '--' et les places donc � une priorit�   *
*   inf�rieure par rapport � []. A mon avis, ce doit �tre une erreur de ce  *
*   tableau: Tout les op�rateurs post-fix�s, y compris [], doivent avoir la *
*   m�me priorit� et �tre lus de gauche � droite...                         *
*                                                                           *
*   J'aurai pr�f�r� utiliser le BNF de GCC mais je n'ai pas r�ussi � la     *
*   trouver (bon, c'est vrai que je n'ai pas trop cherch�!). Je n'ai pas    *
*   non plus v�rifi� certaines priorit�s ... �tranges genre que les         *
*   op�rateurs <, <=, >=, ou > ont priorit� sur == & != ou que && a         *
*   priorit� sur ||.                                                        *
*                                                                           *
*    Dans le code, plut�t que d'utiliser un code adapt� de cette BNF, j'ai  *
*   utilis� un remix entre la m�thode des priorit�s des op�rateurs et       *
*   l'utilisation d'une "BNF restreinte". C'est plus simple � programmer, � *
*   maintenir et, surtout, c'est beaucoup plus rapide !                     *
*                                                                           *
*    Voici donc la BNF restreinte utilis�e dans ce programme:               *
*                                                                           *
*       reponse ::= exp_aff {, exp_aff}                                     *
*   cod� dans la fonction eval().                                           *
*                                                                           *
*       exp_aff ::= ref�rence {oppaff exp_cond}                             *
*       exp_aff ::= exp_cond                                                *
*   cod� dans la fonction affectation().                                    *
*                                                                           *
*       exp_cond ::= exp_binaire ? exp_cond : exp_cond                      *
*       exp_cond ::= exp_binaire                                            *
*   cod� dans la fonction conditionnel().                                   *
*                                                                           *
*       exp_binaire ::= exp_un {[op�rateur_binaire exp_un]}                 *
*   cod� dans la fonction binaire(). Cette fonction g�re elle-m�me la       *
*   priorit� des op�rateurs binaires.                                       *
*                                                                           *
*       exp_un ::= [{pr�-op�rateur}] exp_posf                               *
*   cod� dans la fonction lectunaire().                                     *
*                                                                           *
*       exp_posf ::= exp_prim [{post-op�rateur}]                            *
*   cod� dans la fonction lectunaire().                                     *
*                                                                           *
*       exp_prim ::= valeur_litt�ral                                        *
*                ::= variable                                               *
*                ::= fonction (...)         Appel de fonction               *
*                ::= ( exp_affect )                                         *
*   cod� dans la fonction CalcLexer().                                      *
*                                                                           *
*       ON N'EST PAS CHEZ MICRO-SUCKER, l'optimisation, �a existe !!        *
*                                                                           *
*    Le type de valeur 'R' pour 'r�f�rence', qui est physiquement un        *
*   pointeur, est utilis� pour repr�senter les 'lvalues':                   *
*       - Si l'op�rateur suivant est une affectation, la valeur point�e     *
*       est modifi�e (affectation),                                         *
*       - Sinon, la valeur point�e est plac�e dans la pile...               *
*                                                                           *
*    Le type de valeur 'X', bas� sur le type 'I', contient la valeur d'un   *
*   token lu par la fonction CalcLexer().                                   *
*                                                                           *
\*************** Voir LFCInter pour plus d'informations ********************/

#include "LFCInter.h"
#include "LFCI_icl.h"

	/*
	 *   Stockage des cha�nes lues dans le source. Gr�ce � ceci, une cha�ne
	 *  n'est d�cod�e qu'une seule fois si le source repasse au m�me endroit...
	 */

static struct _stkchaine : public std::string {
	_stkchaine(const char *aptr) : ptr(aptr){};

	const char *ptr; // Valeur du pointeur de d�but de la cha�ne
	const char *apres; // Valeur du pointeur apr�s lecture de la cha�ne
	_stkchaine *suivant;
} *premier = NULL;

_rep lectcar(const char **ptr){
/*  Ne lit qu'un seul caract�re plac� entre ''
 *  <- ptr pointe sur caract�re lui-m�me
 *  -> ptr pointe sur le dernier caract�re pris en compte
 */
	if(**ptr == '\n' || !*ptr){
		std::cerr << "*E* Ligne " << calcligne(*ptr) << ": D�claration de carat�re non termin�e.\n";
		exit(5);
	}

	if(**ptr == '\\'){ // Caract�re qualifi�
		++*ptr; // Saute '\'
		switch(**ptr){
		case '\n':
		case 0:
			std::cerr << "*E* Ligne " << calcligne(*ptr) << ": D�claration de carat�re non termin�e.\n";
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
		case 'X':{  // Lecture d'un caract�re mis sous forme h�xa
				unsigned char i=0;
				while( isxdigit(*++*ptr) ){
					i *= 16;
					if(isdigit(**ptr))
						i += **ptr - '0';
					else
						i += tolower(**ptr) - 'a' + 10;
				}
				--*ptr; // Retour sur le dernier caract�re prise en compte
				return (char) i;
			}
		default:
			if(isodigit(**ptr)){ // Peut �tre une constante octale
				unsigned char i=0;

				do
					i = i*8 + **ptr - '0';
				while( isodigit(*++*ptr) );

				--*ptr; // Retour sur le dernier caract�re prise en compte
				return (char)i;
			} else // Juste un caract�re qui ne doit pas �tre interpr�t�
				return **ptr;
		}
	} else if(**ptr == '\''){ // Lecture d'un caract�re nul ''
		--*ptr; // Pas de caract�re pris en compte, mais on doit quant m�me revenir en arriere!
		return (char)0;
	} else // Simplement un caract�re
		return **ptr;
}

static _rep trouvechaine(_token &ptr){
/* Lecture d'une cha�ne: Si ptr fait d�j� partie de la liste, la cha�ne stock�e
 * est renvoy�e et ptr prend la valeur 'apr�s'
 */
	_stkchaine *t;

	for(t=premier;t;t=t->suivant){
#ifdef __BCPLUSPLUS__
		if(std::string(t->ptr) == ptr){ // La cha�ne a d�j� �t� stock�e
#else
		if(t->ptr == ptr){ // La cha�ne a d�j� �t� stock�e
#endif
			ptr = t->apres;
			return _rep((void *)t->c_str(),'*',"C");
		}
	}

		// C'est une nouvelle cha�ne
	t = new _stkchaine(ptr);
	if(!t){
		std::cerr << "*F* Ligne " << calcligne(ptr) << ": Manque de m�moire pour stocker une cha�ne.\n";
		exit(10);
	}

tchn_autre:
	const char *x=ptr;

	for(++x; *x!='"'; ++x){
		if(*x=='\n' || !*x){
			std::cerr << "*E* Ligne " << calcligne(x) << ": Cha�ne non termin�e " << ADEB("(trouvechaine:1)" << ) ".\n";
			exit(5);
		}

		if(*x == '\\'){
			*t += lectcar(&x).val.caractere; // Stochage du caract�re
			continue;
		}
		*t += *x;
	}

		// Recherche si la cha�ne se poursuit "ch1" "ch2" -> "ch1ch2"
	ptr = x;
	ptr++; // Saute le '"' fermant

	if(*ptr == '"')
		goto tchn_autre;

	t->apres = ptr;
	t->suivant = premier;
	premier = t;

	return _rep((void *)t->c_str(),'*',"C");
}

	/*  Au lieu d'utiliser des valeurs par d�faut pour les arguments optionnels,
	 * rch_op() est surcharg� pour acc�l�rer sont fonctionnement !
	 */

static const _operateur *rch_op(int op){
/*  Recherche la PREMIERE structure associ�e avec l'op�rateur pass� en argument
 *  -> NULL en cas d'erreur
 */
	int i;

	for(i=0; operateur[i].op; i++)
		if(operateur[i].op == op)
			return &operateur[i];

	return NULL;
}

static const _operateur *rch_op(int op, bool unaire){
/*  Recherche la structure associ�e avec l'op�rateur pass� en argument
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
/*  Recherche la structure associ�e avec l'op�rateur pass� en argument
 * tient compte si l'operateur est unaire ou binaire, sa position (uniquement
 * pour les op�rateurs unaires).
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
/* Cette fonction est une interface entre les m�thodes de _token et eval(),
 * elle lit l'�l�ment suivant de l'expression et renvoie:
 *  <- un 'L' si c'est une valeur lit�rale,
 *  <- si c'est une variable, une r�f�rence sur celle-ci,
 *  <- la valeur de retour dans le cas d'une fonction (apr�s l'avoir appel�e)
 *  <- un 'X' s'il s'agit d'un token ou d'un autre caract�re
 *
 *  -> ptr est incr�ment� et len contiendra la longueur du token lu suivant les
 * m�me r�gles que dans LFCI_Lex/trouvetoken(); sauf que ptr est directement
 * incr�ment� s'il s'agit d'un symbole qui est lu.
 *  -> sansfonc == true si les �ventuelles fonctions trouv�es provoquent une
 * erreur au lieu d'�tre ex�cut�e.
 */

	if(*ptr == smbl_id){ // On lit un identifieur
		_var *symbole = trouve_symbole(table_locale, ptr.obj());

		if(!symbole){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Symbole inconnu '" << ptr.obj() << "'\n";
			exit(5);
		}

		ptr++;
		if(symbole->type[0] == 'F'){
				// Si le symbole suivant est un '(' /*)*/ il faut ex�cuter la fonction
			if(*ptr=='(' /*)*/ ){
				if(sansfonc){
					std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe pour '" << ptr.obj() << "' !\n";
					exit(5);
				} else { // C'est une fonction: Il faut l'ex�cuter
					ptr++;
					return lancefonc(ptr, (_var_fonc *)symbole, table_locale);
				}
			} else
				return symbole->repval(ptr); // Sinon on renvoie la repr�sentation de la fonction
		}
		return _rep(symbole->refval(), 'R', symbole->type); // C'est une variable, on renvoie sa r�f�rence
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
	} else if(isdigit(*ptr)){ // On lit une valeur num�rique
		long i=0;
		const char *p = ptr;

		do {
			i = i*10 + *p -'0';
		} while( isdigit(*++p) );
		ptr = p;
		return _rep(i);
	} else if(*ptr == '"') // On lit une cha�ne
		return trouvechaine(ptr);
	else if(*ptr == '\''){ // Lecture d'un caract�re
		const char *x = ptr; // On ne peut pas utiliser _token sinon, par exemple, les espaces seront saut�s
		x++; // Saute le "'"
		_rep t=lectcar(&x);
		if(*++x != '\''){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Impossible de trouver le \"'\" fermant !\n";
			exit(5);
		}
		ptr=++x;
		return t;
	} else if(*ptr == '(' /*)*/ ){ // Parenth�ses
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
	else { // C'est un autre caract�re
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
 *      reponse ::= [{pr�-op�rateur}] exp_prim [{post-op�rateur}]
 *
 *  notez que cette fonction s'attend � lire au moins exp_prim !
 *
 *  Comme tous les op�rateurs unaires ont la m�me priorit� et sont �valu�s de
 *  droite � gauche, il n'y a qu'� les pousser dans la pile et effectuer les
 *  calculs lors du d�pilement.
 */
	LFDynaStack<_rep> valeurs;  // Pile des valeurs
	LFDynaStack<const _operateur *> operateurs;   // Pile des op�rateurs

	_rep suivant=CalcLexer(ptr,table_locale);

	while(suivant.type() == 'X'){ // Lecture des pr�-op�rateurs unaires
		const _operateur *x = rch_op(suivant.val.entier,true,true);

		if(!x){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Le symbole avant '" << ptr.obj() << "' provoque une erreur de syntaxe.\n";
			exit(5);
		}

		operateurs.Push(x); // Tous les op�rateurs unaires de devant ont les m�mes priorit�s

		suivant=CalcLexer(ptr,table_locale);
	}

	valeurs.Push(suivant);
	_token t;

	FOREVER{    // Lecture des post-op�rateurs unaires
		t=ptr;
		suivant=CalcLexer(ptr,table_locale,true);

		if(suivant.type()!='X'){
			std::cerr << "*E* Ligne " << calcligne(ptr) << ": Erreur de syntaxe lors d'un post-operateur unaire ("<< ptr.obj() << ").\n";
			exit(5);
		}

		const _operateur *x = rch_op(suivant.val.entier,true,false);

		if(!x){ // Ce n'est plus un post-op�rateur
			ptr = t; // Il faut donc restorer le pointeur
			break;
		}

		if(x->fonc) // C'est un op�rateur
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

	while(operateurs.length()>=0) // Ex�cution des calculs
		valeurs.Push(operateurs.Pop()->fonc(valeurs,ptr));

	DEB(
		if(valeurs.length()){ // Il ne doit en rester qu'un (highlander ?)
			std::cerr << "*F* Ligne " << calcligne(ptr) << ": Il reste plus d'une valeur dans la pile � la sortie de LFCI_cal/lectunaire().\n";
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
	LFDynaStack<const _operateur *> operateurs;   // Pile des op�rateurs

	FOREVER{
		valeurs.Push(lectunaire(ptr,table_locale));

		const _operateur *x = rch_op(*ptr,false);
		if(!x) break;
		ptr++; // L'op�rateur est accept�, on passe au symbole suivant...

	autres:
		if(operateurs.length()>=0){ // Il y a d�j� quelque chose dans la pile des op�rateurs
			if(operateurs[operateurs.length()]->priorite >= x->priorite){ // Il a priorit�: On l'ex�cute
				valeurs.Push(operateurs.Pop()->fonc(valeurs,ptr));
				goto autres; // Peut-�tre que l'op�rateur pr�c�dent est aussi � ex�cuter.
			}
		}
		operateurs.Push(x);
	}

	while(operateurs.length()>=0) // Ex�cution des calculs
		valeurs.Push(operateurs.Pop()->fonc(valeurs,ptr));

	DEB(
		if(valeurs.length()){ // Il ne doit en rester qu'un
			std::cerr << "*F* Ligne " << calcligne(ptr) << ": Il reste plus d'une valeur dans la pile � la sortie de LFCI_cal/binaire().\n";
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
 * <- 'ptr' doit pointer sur le d�but de l'expression
 *    table_locale est la table actuelle des symboles (NULL= uniquement la table globale)
 *    'un' est vrai si une seule expression doit �tre lue, la pr�sence d'un ','
 *    ne provoque pas la lecture d'autres valeurs (par exemple dans le cas de
 *    d�finitions de variables...). Note: Si 'un = 'false', la pr�sence d'un ';'
 *    comme premier caract�re lu provoque le retour d'un void.
 *
 *  -> Renvoie la valeur repr�sentant le resultat de l'expression
 *     'ptr' pointe sur le premier caract�re qui provoque une erreur de l'expression
 *     (caract�re inconnu, s'il s'agit d'une erreur de synstaxe de calcul genre
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
