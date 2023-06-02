#include "futbalista.h"

int SIRKA_KAMERY;

void navod() {
	//TODO
	// 11 - pohyb (x, y, w)
}

void spracuj_vstup(string vstup) {
	int a;
	sscanf(vstup.c_str(), "%d", &a);
	
	//TODO ostatne prikazy co bude robit rpi
	if(a == 9){
		calibrate_camera();
	}
	else{
		zapis_paket_do_arduina((const uint8_t*)vstup.c_str());
	}
}

int main() {
	// setup
	Lopta lopta(0, 95, 143, 15, 255, 255);
	//SIRKA_KAMERY = setup_kamera();
	//Branka my_branka();
	//Branka jeho_branka();

	// main loop
	while (1) {
		// ak nieco je v konzole, spracujeme to
		string in;
		while(cin >> in){
			spracuj_vstup(in);
		}
		
		//spracovavame kameru
		lopta.debug();
		//zistime o kolko sa treba otacat aby sme sa pozerali za loptou
		if(lopta.find()){
			double dist_x = (lopta.mid.x - SIRKA_KAMERY/2) / (SIRKA_KAMERY * 0.5);
			char packet[50] = "11 0 0 XxxXXXXX";
			sprintf(packet, "11 0 0 %d", (int)(0.5 + dist_x*100));
			zapis_paket_do_arduina((const uint8_t*)packet);
			
			//zapis_paket_do_arduina((const uint8_t*)("11 0 0 " + to_string(dist_x)));
			cout << dist_x << endl;
		}
		else{
			zapis_paket_do_arduina((const uint8_t*)"11 0 0 100");
		}
	}

	// setdown :)
}
