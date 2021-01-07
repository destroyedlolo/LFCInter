/*
 *  afferr.x
 *
 *      Affiche les messages d'erreurs de strerror().
 */

int main(int ac, char **av){
    if(ac != 2){
        puts("afferr num_erreur\n\tAffiche les messages d'erreurs strerror()");
        exit(5);
    }

    puts(strerror(atoi(av[1])));

    exit(0);
}