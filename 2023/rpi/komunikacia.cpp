#include "third-party/include/serial/serial.h"
#include <iostream>
#include "thread"

using namespace std;

serial::Serial my_serial("/dev/ttyUSB0", 115200, serial::Timeout::simpleTimeout(3000));

void setup_komunikacia() {
    if (my_serial.isOpen())
        cout << "Port opened succesfully" << endl;
    else
        cout << "Port failed to open" << endl;
    my_serial.flushOutput();
}

void precitaj_paket() {
    char ch;
    string packet;
    // cakame na zaciatok inputu
    do {
        ch = my_serial.read(1)[0];
    } while (ch != '$');
    
    do {
        ch = my_serial.read(1)[0];
        packet.push_back(ch);
    } while (ch != '\n');
    
    spracuj_packet(packet);
}

void spracuj_packet(string packet) {
    cout << "ard->rpi: " << packet << endl;
}

void posli_packet(string packet) {
    while(my_serial.write(packet) < packet.size()*sizeof(char)); // netusim ci to robi to co chcem
}
