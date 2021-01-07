/*  Atoi.x
 *
 *      Exemple d'utilisation de la fonction atoi(): Lecture d'un entier.
 *      Montre aussi comment palier au manque du scanf() dans la version 1.0
 *      de l'interpréteur...
 */


int main(int ac, char **av){
    char x[256]; // Définition du buffer

    printf("Entrez un chiffre :");
    flushstdout();
    gets(x);

    printf("-> %s : %d\n",x,atoi(x)); // Résultat

    return 0;
}