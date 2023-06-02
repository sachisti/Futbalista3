#include <inttypes.h>

// vsetky prototypy funkcii

// pohyb

void setup_pohyb();

void motor_speed(uint8_t motor, uint8_t speed);
void motor_smer(uint8_t motor, uint8_t smer);
void test_motors();
void zastav_motory();


// vypinace

void setup_vypinace();
void test_vypinace();
uint8_t config_on();
uint8_t start_on();

// senzory (pouzi analogRead(0)-analogRead(7) okrem 4,5

void setup_senzory();
void test_senzorov();
int vidi_ciaru();

// kompas

void setup_kompas();
int kompas();
void test_kompas();

// komunikacia

void setup_komunikacia();
void komunikacia();        // treba volat v hlavnej slucke casto

// strategia

void setup_strategia();
void strategia();          // treba volat v hlavnej slucke casto
void kontrola_senzorov();
