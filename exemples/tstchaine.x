/*
 *  tstcha�ne.x
 *
 *       Programme pour tester le fonctionnement des fonctions de cha�nes
 *  et de la lecture des arguments.
 */

int main(int ac, char **av){
    char t[256];

    if(ac!=3){
        puts("Ce programme n�cessite 2 arguments...");
        exit(5);
    }

    printf("Argument 1: '%s' (longueur %d)\n",av[1],strlen(av[1]));
    printf("Argument 2: '%s' (longueur %d)\n",av[2],strlen(av[2]));

    if(strlen(av[1]) + strlen(av[1]) > 255){
        puts("Les 2 cha�nes fournies sont trop longues !\n"
             "(Le total ne doit pas d�passer 256 caract�res)");
        exit(10);
    }

    strcpy(t,av[1]);
    printf("strcpy() -> '%s'\n",t);

    strcat(t,av[2]);
    printf("strcat() -> '%s'\n",t);

    return 0;
}