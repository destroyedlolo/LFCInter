/*
 *  Message.x
 *
 *  Message secret : D�monstration des carat�res sp�ciaux.
 */

int main(int ac, char **av){
    int i;

    for(i=0;i<35;i++){
        printf("%*s\r",i,"bonjours");
        flushstdout();
    }

    puts("\a\a\f");

    puts(" u  r v ir\rx\a\ba\n     \ve\n       \vo");
    return 0;
}