/*
 *  is.x
 *
 *  Test des fonctions is????()
 */

int main(int ac, char **av){
    char *x;

    if(ac != 2){
        puts("is chaîne.\nAffiche les groupes auxquels appartiennent les caractères de la chaîne.");
        exit(5);
    }

    for(x=av[1];*x;x++){
        printf("%d: '%c'",*x,*x);

        if(isdigit(*x))
            printf(" digit");

        if(isalpha(*x))
            printf(" alpha");

        if(islower(*x))
            printf(" lower");

        if(isupper(*x))
            printf(" upper");

        if(isspace(*x))
            printf(" space");

        if(ispunct(*x))
            printf(" punct");

        if(isxdigit(*x))
            printf(" xdigit");

        if(isalnum(*x))
            printf(" alnum");

        if(isprint(*x))
            printf(" print");

        if(isgraph(*x))
            printf(" graph");

        if(iscntrl(*x))
            printf(" cntrl");

        if(isascii(*x))
            printf(" ascii");

        putchar('\n');
    }

    return 0;
}
