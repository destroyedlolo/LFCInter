/*
 *  Tstheure.x
 *
 *  Test les fonctions time() & ctime()
 */

int main(int ac, char **av){
    int x[2]; // Il faut au moins 4 octets pour stocker l'heure, comme les
              // entiers sur PC ne font que 16 bits, utiliser un tableau permet
              // une compatibilit� sur tous les syst�mes...

    time(x);
    printf("Il est %s\n",ctime(x));

    return 0;
}