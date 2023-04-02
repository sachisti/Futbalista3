#include <inttypes.h>

// komunikacia 

int setup_komunikacia();
void zapis_paket_do_arduina(uint8_t *zapisovany_paket); 
void ukonci_komunikaciu();

// kamera

int setup_kamera();
void ukonci_kameru();
void najdi_loptu(int *sirka_lopty, int *vyska_lopty, int *velkost_lopty, int *riadok, int *stlpec);
void test_kamery();


// logovanie
void setup_log();
void zaloguj(char *sprava);
void zaloguj_n(char *sprava, int cislo);

