/*
 *  Tableau.x
 *
 *      Exemple d'utilisation de tableaux
 *
 *  Utiliser l'options -nprec pour �viter d'avoir plein de warning
 */

int main(int ac, char **av){
    char chaine[2][27];
    int i;

    for(i=0; i<26; i++)
        chaine[0][i]= i + 'A';
    chaine[0][i]=0; // Il faut terminer la cha�ne

    chaine[1][26]=0;
    for(i=25; i>=0; i--)
        chaine[1][i]= 'Z'-i;

    printf("Cha�ne 1: '%s'\n",chaine[0]);
    printf("Cha�ne 2: '%s'\n",chaine[1]);

    return 0;
}