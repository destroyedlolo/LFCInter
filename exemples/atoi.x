/*  Atoi.x
 *
 *      Exemple d'utilisation de la fonction atoi(): Lecture d'un entier.
 *      Montre aussi comment palier au manque du scanf() dans la version 1.0
 *      de l'interpr�teur...
 */


int main(int ac, char **av){
    char x[256]; // D�finition du buffer

    printf("Entrez un chiffre :");
    flushstdout();
    gets(x);

    printf("-> %s : %d\n",x,atoi(x)); // R�sultat

    return 0;
}