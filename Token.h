/***************************************************************************\
*  Token.h                                                                  *
*  Projet : LFCInter                                                        *
*      © LFSoft 1995-96                                                     *
*                                                                           *
*  Définition de la classe et de ses méthodes qui permettent de parser le   *
*  texte source.                                                            *
*                                                                           *
\************* Voir LFCInter.cxx pour plus d'informations ******************/

#ifndef LCI_TOKEN_H
#define LCI_TOKEN_H

#ifndef LFCINTER_H
#include "LFCInter.h"
#endif

    /*
     * Valeurs symboliques des token lus.
     * Note: l'ordre des symboles est important. Ils sont regroupés par type
     * avec comme bornes :
     *      'void' et 'char' : pour  les descripteurs de types
     *      'for' et 'enum' : pour les mots clefs
     *      'printf' et 'free' : pour les fonctions émulées ou les opérateurs complexes
     *  de même:
     *      'smbl_sra' et 'smbl_ora' encadre les opérateurs d'affectations.
     */
enum {
    smbl_id = -1, // Identificateur
    smbl_icn = 0, // objet non initialisé ou fin du fichier source
    smbl_void=128,smbl_int,smbl_char,smbl_enum,
    smbl_for,smbl_do,smbl_while,smbl_switch,smbl_default,smbl_case,smbl_if,
    smbl_else, smbl_break,smbl_continue,smbl_return,smbl_goto, smbl_exit,
    smbl_sra,smbl_sla,smbl_aa,smbl_sa,smbl_ma,smbl_da,smbl_mda,
    smbl_anda,smbl_xora,smbl_ora,
    smbl_sr,smbl_sl,smbl_inc,smbl_dec,smbl_land,
    smbl_lor,smbl_lle,smbl_lge,smbl_lequ,smbl_lne,
    smbl_printf,smbl_sprintf,smbl_scanf,smbl_putchar,smbl_puts, smbl_sizeof,
    smbl_gets,smbl_getchar,smbl_flushstdout,smbl_atoi,
    smbl_strcat,smbl_strchr,smbl_strcmp,smbl_strcpy,smbl_strerror,smbl_strlen,
    smbl_strncat,smbl_strncmp,smbl_strncpy,smbl_strrchr,smbl_strdup,
    smbl_stricmp,smbl_strnicmp, smbl_strcasecmp,smbl_strncasecmp,
    smbl_strpbrk,smbl_strstr,smbl_strcoll,
    smbl_strcspn,smbl_strspn,smbl_strtok,smbl_strtol,
#if 0
	smbl_swab,
#endif
    smbl_memchr,smbl_memcmp,smbl_memcpy,smbl_memmove,smbl_memset,smbl_memccpy,
    smbl_system,smbl_isdigit,smbl_islower,smbl_isspace,smbl_ispunct,smbl_isupper,
    smbl_isalpha,smbl_isxdigit,smbl_isalnum,smbl_isprint,smbl_isgraph,smbl_iscntrl,
    smbl_isascii,smbl_isiso,smbl_toupper,smbl_tolower,smbl_toiso,
    smbl_time,smbl_ctime,smbl_clock,smbl_sleep,
    smbl_realloc,smbl_malloc, smbl_free,
    smbl_debc,smbl_finc,smbl_debcppc
};

    /*
     * Classe de représentation de l'objet actuel.
     */
class _token {
public:
        // Constructeurs
    _token(const char *x){ ptr=x-1; len=1; operator++(); }; // Grâce au ++ si le symbole pointé est un commentaire, il est directement sauté
    _token() { ptr = NULL; construit(); };

    int operator * () { return val; }; // Return la valeur symbolique du token
    operator const char * () { return ptr; };
    _token operator ++ (); // Vas au token suivant (préfixe)...
    _token operator ++ (int); // Vas au token suivant (suffixe)...
    void saute(const char *sep=";"); // Saute l'objet courant
    std::string obj();   // Retourne l'objet pointé

        // Est-ce un mot clef utilisé pour une définition ?
    bool definition() { return ((val>=smbl_void && val<=smbl_char) || val=='*' || val==-2); };
        // Est-ce une 'fonction' interne ?
    bool interne() { return (val>=smbl_printf && val<=smbl_free); };

    int hash;

protected:
    const char *ptr;    // pointeur dans le buffer du début de l'objet actuellement pointé
    short int val;      // Valeur du symbolique du token lu.
    size_t len;         // Longueur de l'objet pointé

private:
    void construit( bool ignore= false );   // Construit les valeurs internes.

friend int calcligne( _token &);
};

        /************ Quelques fonctions couramment utilisées ************/

    // LFCInter.cxx
inline int calcligne( _token &t ){ return calcligne(t.ptr); };

#endif
