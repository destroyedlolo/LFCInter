/*  tstvar.x
 *
 *  Test sur la port� des variables;
 */

char *x = "globale";

void fonc1(){ // variable locale
    char *x="fonction 1";

    printf("Dans fonc1 (variable locale):%s\n",x);
}

void fonc2(){ // Pas de variable locale
    printf("Dans fonc2 (pas de variable locale):%s\n",x);
}

void fonc3(char *x){ // Pass� en argument
    printf("Dans fonc3 (passage d'argument):%s\n",x);
}

void fonc4(char *x){ // Pass� en argument par valeur avec modif
    x="modifier par fonc4";
    printf("Dans fonc4 (passage d'argument par valeur avec modification):%s\n",x);
}

void fonc5(char **x){ // Pass� en argument par adresse avec modif
    *x="modifier par fonc5";
    printf("Dans fonc5 (passage d'argument par valeur avec modification):%s\n",*x);
}

void main(){
    char *x="fonction main()";

    printf("Au d�but de main :%s\n",x);

    fonc1();
    fonc2();
    fonc3(x);

    printf("Dans main() apr�s fonc3 :%s\n",x);

    fonc4(x);
    printf("Dans main() apr�s fonc4 :%s\n",x);

    fonc5(&x);
    printf("Dans main() apr�s fonc5 :%s\n",x);
}
