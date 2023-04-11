#include <stdio.h>
#include <string.h>

#include "futbalista.h"

uint8_t headless = 0;

void navod()
{
	printf("0 - navod\n");
	printf("1 - test senzorov\n");
	printf("2 - test vypinacov\n");
	printf("3 - test pohybov\n");
	printf("4 - test dokola\n");
	printf("5 - test dolava\n");
	printf("6 - test dopredu\n");
	printf("7 - test doprava\n");
	printf("8 - test zastav\n");
	printf("9 - test kamery\n");
	printf("11 - start hry\n");
	printf("100 - koniec\n");
	printf("9999 - test komunikacie\n");
	printf("31 - Arduino dolava_vzad()\n");
	printf("32 - Arduino doprava_vzad()\n");
	printf("33 - Arduino dozadu()\n");
}

//spravy RPI -> Arduino:
//  21 - lopta vpravo
//  22 - lopta vlavo
//  23 - lopta v strede
//  24 - nevidi loptu


void posli_lopta_vpravo()
{
    char *s = "21";    
    zapis_paket_do_arduina(s);

}

void posli_lopta_vlavo()
{
    char *s = "22";    
    zapis_paket_do_arduina(s);

}

void posli_lopta_vstrede()
{
    char *s = "23";    
    zapis_paket_do_arduina(s);
}

void posli_nevidi_loptu()
{
    char *s = "24";    
    zapis_paket_do_arduina(s);
}


int hra()
{
    int iter = 0;
    zaloguj("futbalista bezi v headless rezime");
    
    do {
        int sirka, vyska, velkost, riadok, stlpec;
        najdi_loptu(&sirka, &vyska, &velkost, &riadok, &stlpec);
	
	if (!stlpec) posli_nevidi_loptu();
	if (velkost < 20) continue;
	if (stlpec < 320 / 3) posli_lopta_vpravo();
	else if (stlpec < 2 * 320 / 3) posli_lopta_vstrede();
	else posli_lopta_vlavo();
	
	iter++;
	if (iter % 100 == 0)
	  zaloguj_n("hra() iter", iter);
    } while (1);
    
    return 0;
}

int hlavny_program()
{
    if (headless) return hra();
    
    zaloguj("futbalista bezi v interaktivnom rezime");
    char sprava[100];
    int a = 0;
    printf("0 = navod\n");

    do {
        printf("presov> ");
        fgets(sprava, 5, stdin);
        sscanf(sprava, "%d", &a);
        if (a == 0) navod();
	else if (a == 9) test_kamery();
        else
        {
            sprintf(sprava, "%d", a);
            zapis_paket_do_arduina(sprava);
        }
    } while (a != 100);
}

int main(int argc, char **argv)
{
    if ((argc > 1) && (strcmp(argv[1], "headless") == 0)) headless = 1;
    
    setup_log();
    setup_komunikacia();
    setup_kamera();

    hlavny_program();

    ukonci_kameru();
    ukonci_komunikaciu();
    return 0;
}

