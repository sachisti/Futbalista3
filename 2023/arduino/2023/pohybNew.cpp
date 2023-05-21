#include <Arduino.h>
#include <tuple>
#include "futbalista.h"
#include <opencv2/opencv.hpp>

#define MAX_SPEED 20

#define M1_IN1 4
#define M1_IN2 5

#define M2_IN1 6
#define M2_IN2 7

#define M3_IN1 2
#define M3_IN2 3

//Co? TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
// 1 - back
// 2 - right
// 3 - left
#define MR  1
#define ML  2
#define MB  3

#define MFW 0
#define MBW 1

#define CAS_OBRAT_SMER 500 //cas na ktory pojde robot do opacneho smeru

volatile uint8_t r1, r2, r3; //rychlosti motorov
volatile uint8_t t1_tick;
volatile uint8_t on1, off1, on2, off2, on3, off3;

void setup_pohyb()
{
	r1 = 0;
	r2 = 0;
	r3 = 0;
	t1_tick = 0;
	on1 = 1;
	off1 = 0;
	on2 = 1;
	off2 = 0;
	on3 = 1;
	off3 = 0;

	pinMode(M1_IN1, OUTPUT);
	pinMode(M1_IN2, OUTPUT);
	pinMode(M2_IN1, OUTPUT);
	pinMode(M2_IN2, OUTPUT);
	pinMode(M3_IN1, OUTPUT);
	pinMode(M3_IN2, OUTPUT);

	digitalWrite(M1_IN1, LOW);
	digitalWrite(M1_IN2, LOW);
	digitalWrite(M2_IN1, LOW);
	digitalWrite(M2_IN2, LOW);
	digitalWrite(M3_IN1, LOW);
	digitalWrite(M3_IN2, LOW);

	ICR1 = 1600;  // timer1 freq. = 10kHz
	TCNT1 = 0;
	TCCR1A = 0b00000010;
	TCCR1B = 0b00011001;
	TCCR1C = 0;
	TIMSK1 = 0b00000001;

	delay(300);
	motor_smer(1, 0);
	motor_smer(2, 0);
	motor_smer(3, 0);
}

void motor_speed(uint8_t motor, uint8_t speed)
{
	switch (motor)
	{
	case 1: r1 = speed; break;
	case 2: r2 = speed; break;
	case 3: r3 = speed; break;
	}
}

void motor_smer(uint8_t motor, uint8_t smer)
{
	switch (motor)
	{
	case 1: digitalWrite(M1_IN2, smer); off1 = smer; on1 = 1 - smer; break;
	case 2: digitalWrite(M2_IN2, smer); off2 = smer; on2 = 1 - smer; break;
	case 3: digitalWrite(M3_IN2, smer); off3 = smer; on3 = 1 - smer; break;
	}
}

void set_speeds(double sl, double sr, double sb, int speed) {
	sl /= max(max(sl, sr), sb); sl *= speed;
	sr /= max(max(sl, sr), sb);	sr *= speed;
	sb /= max(max(sl, sr), sb);	sb *= speed;

	uint8_t fw = 0;
	if (sl < 0) sl = -sl, fw = 1;
	motor_speed(ML, (uint8_t)sl);
	motor_smer(ML, fw);

	fw = 0;
	if (sr < 0) sr = -sr, fw = 1;
	motor_speed(MR, (uint8_t)sr);
	motor_smer(MR, fw);

	fw = 0;
	if (sb < 0) sb = -sb, fw = 1;
	motor_speed(MB, (uint8_t)sb);
	motor_smer(MB, fw);
}

// uhly kolies sú l = 120, r = 240, b = 0
double a = -0.5, b = -0.5,  c = 1; //cos
double d = 0.86, e = -0.86, f = 0; //sin
double g = 1,    h = 1,     i = 1; //theta

double det = a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;

double ai = (e * i - f * h) / det, bi = (h * c - i * b) / det, ci = (b * f - c * e) / det;
double di = (g * f - d * i) / det, ei = (a * i - g * c) / det, fi = (d * c - a * f) / det;
double gi = (d * h - g * e) / det, hi = (g * b - a * h) / det, ii = (a * e - d * b) / det;

void pohyb(double xs, double ys, double ws, int speed = MAX_SPEED) {
	double sl = ai * xs + bi * ys + ci * w;
	double sr = di * xs + ei * ys + fi * w;
	double sb = gi * xs + hi * ys + ii * w;
	set_speeds(sl, sr, sb, speed);
}