#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <TimeLib.h> // Library untuk waktu

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

//pin sensor suhu
#define ONE_WIRE_BUS 2

//pin sensor tegangan
#define VPINV A0

//pin sensor arus
#define VPINC A1

#define SCREEN_WIDTH 128 // Lebar layar OLED dalam piksel
#define SCREEN_HEIGHT 32 // Tinggi layar OLED dalam piksel
#define OLED_RESET -1    // Pin reset (atau -1 jika tidak digunakan)
#define SCREEN_ADDRESS 0x3C // Alamat I2C untuk OLED

#define SEND_LEFT 3
#define SEND_RIGHT 4

int button[]={SEND_LEFT,SEND_RIGHT};
// Membuat objek display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

bool                showTime = true;  // Status untuk menampilkan waktu atau tanggal
unsigned long       previousMillis = 0; // Waktu sebelumnya dalam milidetik
const unsigned long interval = 10000; // Interval untuk bergantian (5 detik)

//variabel untuk sensor tegangan 
float tegangan,vout;
float vref = 5.0;
float res_bit = 1023.0;
float R1      = 30000.0;
float R2      = 7500.0;

//variabel untuk sensor arus
int    sensitivitas = 66;
int    nilaiADC=00;
int    offset = 2500;
double teganganArus = 00;
double nilaiArus = 00;

float filteredValue = 0;  // Nilai hasil filter

enum Mode{
  MODE_CLOCK,
  MODE_DATE,
  MODE_TEMP,
  MODE_VOLT,
  MODE_CURRENT,
  MODE_SEND
};
Mode mode = MODE_CURRENT;

// 'pngwing', 40x40px
const unsigned char send_left [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 
  0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 
  0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xf0, 
  0x01, 0xff, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xff, 0xf0, 0x0f, 
  0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xff, 0xf0, 0x03, 0xff, 
  0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0xff, 0xf0, 0x00, 0x3f, 0xf0, 
  0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 
  0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 'pngwing', 40x40px
const unsigned char send_right [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 
  0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0xe0, 
  0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x0f, 0xff, 0xff, 0xfe, 0x00, 
  0x0f, 0xff, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xe0, 0x0f, 
  0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xe0, 0x0f, 0xff, 
  0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x0f, 
  0xfc, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xc0, 
  0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// 'suhu', 32x32px
const unsigned char logo_suhu [] PROGMEM = {
  0x00, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x80, 0x00, 0x00, 0x08, 0x40, 0x00, 0x00, 0x10, 0x40, 0x00, 
  0x00, 0x10, 0x40, 0x00, 0x00, 0x10, 0x40, 0x00, 0x00, 0x10, 0x40, 0x00, 0x00, 0x10, 0x5c, 0x00, 
  0x00, 0x10, 0x40, 0x00, 0x00, 0x13, 0x58, 0x00, 0x00, 0x13, 0x40, 0x00, 0x00, 0x13, 0x5c, 0x00, 
  0x00, 0x13, 0x40, 0x00, 0x00, 0x13, 0x40, 0x00, 0x00, 0x13, 0x58, 0x00, 0x00, 0x13, 0x40, 0x00, 
  0x00, 0x23, 0x20, 0x00, 0x00, 0x23, 0x10, 0x00, 0x00, 0x47, 0x90, 0x00, 0x00, 0x4f, 0x90, 0x00, 
  0x00, 0x4f, 0x90, 0x00, 0x00, 0x67, 0x90, 0x00, 0x00, 0x20, 0x10, 0x00, 0x00, 0x10, 0x20, 0x00, 
  0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x3c, 0x78, 0xf0, 
  0x21, 0xc3, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3c, 0x78, 0xf8, 0x00, 0xc1, 0x86, 0x00
};

// '—Pngtree—car battery icon_4393362', 40x42px
const unsigned char petir [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xf1, 0xff, 0x8f, 0x00, 0x00, 0xf1, 0xff, 0x8f, 0x00, 0x00, 0xf1, 0xff, 
  0x8f, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xff, 
  0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 
  0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xf7, 0xff, 0xff, 0xc0, 0x03, 
  0xf7, 0xff, 0xff, 0xc0, 0x03, 0xf7, 0xff, 0xff, 0xc0, 0x03, 0xc0, 0xff, 0x03, 0xc0, 0x03, 0xf7, 
  0xff, 0xff, 0xc0, 0x03, 0xf7, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 
  0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 
  0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xf0, 
  0x07, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00
};


// // 'petir2', 40x40px
// const unsigned char petir1 [] PROGMEM = {
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x03, 
//   0xfa, 0x80, 0x00, 0x00, 0x0f, 0xfe, 0xe0, 0x00, 0x00, 0x1f, 0x87, 0xf0, 0x00, 0x00, 0x3e, 0x0c, 
//   0xf8, 0x00, 0x00, 0x78, 0x0c, 0x3c, 0x00, 0x00, 0xf0, 0x1c, 0x1e, 0x00, 0x00, 0xe0, 0x18, 0x0e, 
//   0x00, 0x00, 0xe0, 0x38, 0x0f, 0x00, 0x01, 0xc0, 0x78, 0x07, 0x00, 0x01, 0xc0, 0x78, 0x07, 0x00, 
//   0x01, 0xc0, 0xff, 0x07, 0x00, 0x01, 0xc0, 0xff, 0x07, 0x00, 0x01, 0xc1, 0xff, 0x07, 0x00, 0x01, 
//   0xc0, 0x3e, 0x07, 0x00, 0x01, 0xc0, 0x1c, 0x07, 0x00, 0x01, 0xc0, 0x1c, 0x07, 0x00, 0x00, 0xe0, 
//   0x38, 0x0f, 0x00, 0x00, 0xe0, 0x38, 0x0e, 0x00, 0x00, 0xf0, 0x30, 0x1e, 0x00, 0x00, 0x78, 0x70, 
//   0x3c, 0x00, 0x00, 0x3e, 0x60, 0x78, 0x00, 0x00, 0x1f, 0x63, 0xf0, 0x00, 0x00, 0x0e, 0xdf, 0xe0, 
//   0x00, 0x00, 0x02, 0xbf, 0x80, 0x00, 0x00, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
//   0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// };


// 'ameter', 40x40px
const unsigned char amper [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 
  0x07, 0xc1, 0xe0, 0x00, 0x00, 0x0e, 0x00, 0x38, 0x00, 0x00, 0x18, 0x00, 0x1c, 0x00, 0x00, 0x30, 
  0x00, 0x0e, 0x00, 0x00, 0x60, 0x00, 0x07, 0x00, 0x00, 0xe0, 0x18, 0x03, 0x00, 0x00, 0xc0, 0x18, 
  0x01, 0x80, 0x00, 0x80, 0x3c, 0x01, 0x80, 0x01, 0x80, 0x34, 0x01, 0x80, 0x01, 0x80, 0x26, 0x00, 
  0xc0, 0x01, 0x80, 0x66, 0x00, 0xc0, 0xff, 0x80, 0x7e, 0x00, 0xff, 0xff, 0x80, 0x7f, 0x00, 0xff, 
  0x01, 0x80, 0xc1, 0x00, 0xc0, 0x01, 0x80, 0x81, 0x80, 0xc0, 0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 
  0xc0, 0x00, 0x01, 0x80, 0x00, 0xc0, 0x00, 0x03, 0x00, 0x00, 0xe0, 0x00, 0x03, 0x00, 0x00, 0x60, 
  0x00, 0x06, 0x00, 0x00, 0x30, 0x00, 0x0e, 0x00, 0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0x0f, 0x00, 
  0x78, 0x00, 0x00, 0x07, 0xe7, 0xe0, 0x00, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// // 'ameter', 32x32px
// const unsigned char amper1 [] PROGMEM = {
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x70, 0x0e, 0x00, 
//   0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x80, 0x01, 0x80, 0x00, 0x80, 0x03, 0x01, 0x80, 0xc0, 
//   0x02, 0x01, 0x80, 0x40, 0x02, 0x02, 0x40, 0x60, 0x02, 0x02, 0x40, 0x60, 0xfe, 0x03, 0xe0, 0x7f, 
//   0xfe, 0x06, 0x20, 0x7f, 0x02, 0x04, 0x20, 0x60, 0x02, 0x00, 0x10, 0x60, 0x03, 0x00, 0x00, 0x40, 
//   0x03, 0x00, 0x00, 0xc0, 0x01, 0x80, 0x00, 0x80, 0x00, 0xc0, 0x01, 0x80, 0x00, 0x60, 0x03, 0x00, 
//   0x00, 0x38, 0x0e, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
// };

// 'lamp_on4', 32x32px
const unsigned char icon_lamp [] PROGMEM = {
	0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 
	0x06, 0x01, 0x80, 0x40, 0x03, 0x01, 0x80, 0xc0, 0x01, 0x80, 0x01, 0x80, 0x00, 0xc3, 0xc3, 0x00, 
	0x00, 0x4f, 0xf2, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x30, 0x0c, 0x00, 0x00, 0x60, 0x06, 0x00, 
	0x00, 0x40, 0x02, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x7e, 0xc0, 0x03, 0x7e, 0x3e, 0xc0, 0x03, 0x7c, 
	0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0c, 0x00, 
	0x00, 0x10, 0x08, 0x00, 0x00, 0xd8, 0x1b, 0x00, 0x01, 0x98, 0x11, 0x80, 0x03, 0x0f, 0xf0, 0xc0, 
	0x06, 0x0f, 0x80, 0x60, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x0f, 0xf0, 0x00, 
	0x00, 0x07, 0x80, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x01, 0x80, 0x00
};



// 'lamp_off', 32x32px
const unsigned char icon_off [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0xff, 0xf8, 
	0x00, 0x01, 0xff, 0xf8, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x03, 0xff, 0xfe, 
	0x00, 0x03, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x03, 0xff, 0xfe, 
	0x00, 0x03, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf8, 
	0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xe0, 
	0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 
	0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00
};


// 'sepeda', 54x48px
const unsigned char sepeda [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x1c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x1f, 
  0x00, 0x00, 0x7f, 0xf0, 0x01, 0xf0, 0x27, 0x80, 0x00, 0x7f, 0xf0, 0x07, 0xfe, 0x7b, 0xc0, 0x00, 
  0x1f, 0xfc, 0x07, 0xff, 0xfb, 0xc0, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x38, 0x7f, 
  0xff, 0xff, 0xff, 0x80, 0x00, 0x10, 0x0f, 0xff, 0xff, 0xff, 0xc8, 0x00, 0x60, 0x07, 0xff, 0xff, 
  0xff, 0xf0, 0x00, 0x40, 0x1f, 0xdf, 0xff, 0x3f, 0x00, 0x00, 0x03, 0xc8, 0x5f, 0xf8, 0x7f, 0xe0, 
  0x00, 0x0f, 0xf2, 0x3f, 0xf1, 0xff, 0xf0, 0x00, 0x1f, 0xfb, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x38, 
  0x9f, 0xfd, 0xf7, 0xff, 0xdc, 0x00, 0x70, 0xff, 0xff, 0xff, 0xf9, 0x8e, 0x00, 0x7b, 0xfe, 0x7f, 
  0xff, 0xff, 0xe6, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x67, 0xf6, 0x7f, 0xff, 0xf7, 
  0xf7, 0x00, 0x63, 0xe6, 0x7f, 0xff, 0xb7, 0xf7, 0x00, 0x73, 0xde, 0x7f, 0xff, 0xb3, 0xe6, 0x00, 
  0x74, 0x2e, 0xff, 0xff, 0xbb, 0xe6, 0x00, 0x3c, 0x3c, 0x00, 0x00, 0x1e, 0x3e, 0x00, 0x3f, 0xfc, 
  0x00, 0x00, 0x1f, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x07, 0xe0, 0x00, 0x00, 
  0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void setup() {
  Serial.begin(9600);
  filteredValue = analogRead(VPINC);
  // Start up the library DS13B20
  sensors.begin();
  for(int i=0;i<2;i++){pinMode(button[i],OUTPUT);}
  
  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Jangan lanjutkan, berhenti di sini
  }

  display.clearDisplay();
  
  // for(int i=0;i<= 200;i++){
  //  display.drawBitmap(-54+i, -9, sepeda, 54, 48, WHITE); 
  //  display.display();
  //  delay(2);
  //  display.clearDisplay();
  // }
  // Set waktu awal (contoh: 10 Desember 2024, pukul 21:30:00)
  //setTime(23, 00, 0, 10, 12, 2024);
   //delay (1000);
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();
static int count=1;
  // Jika sudah melewati 5 detik, ganti status antara waktu dan tanggal
  if (currentMillis - previousMillis >= interval && mode != MODE_SEND) {
    previousMillis = currentMillis;
  
  switch(count){
    case 1 : 
      mode = MODE_CLOCK;
    break;
    case 2 : 
      mode = MODE_DATE;
    break;
    case 3 : 
      mode = MODE_TEMP;
    break;
    case 4 : 
      mode = MODE_VOLT;
    break;
    case 5 : 
      mode = MODE_CURRENT;
      count = 0;
    break;
  };
   count++; // Toggle antara waktu dan tanggal
}

switch(mode){
  case MODE_CLOCK :
    display.clearDisplay();
    showClock();
  break;
  case MODE_DATE :
    display.clearDisplay();
    showDate();
  break;
  case MODE_TEMP :
    display.clearDisplay();
    showTemperatur();
  break;
  case MODE_VOLT :
    display.clearDisplay();
    showVoltage();
  break;
  case MODE_CURRENT :
    display.clearDisplay();
    showCurrent();
  break;
  case MODE_SEND :
    display.clearDisplay();
    showSend(buttonPin,buttonPin);
  break;
}
//Serial.println(String()+"count:"+count);
  
}

void showClock(){
   // Tampilkan waktu tanpa detik
    String waktu;
    int jam = hour();
    int menit = minute();
    char Jam[5],Menit[5];
    static bool flag;
    static uint32_t saveTmr=0;
    uint32_t tmr = millis();

    sprintf(Jam,"%02d",jam);
    sprintf(Menit,"%02d",menit);
    display.setTextSize(4); // Ukuran teks lebih besar untuk jam dan menit
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 1); display.print(Jam);
    display.setCursor(80, 1); display.print(Menit);

    if(tmr-saveTmr >500){
      saveTmr=tmr;
      flag = !flag;
    }

    if(flag){
      display.fillCircle(62,7,3,SSD1306_INVERSE);
      display.fillCircle(62,21,3,SSD1306_INVERSE);
    }else{
      display.fillCircle(62,7,3,BLACK);
      display.fillCircle(62,21,3,BLACK);
    }
    display.display();
}

void showDate(){
  // Tampilkan tanggal
    int tanggal = day();
    int bulan = month();
    int tahun = year();
    String tanggalStr = (tanggal < 10 ? "0" : "") + String(tanggal) + "/" +
                        (bulan < 10 ? "0" : "") + String(bulan) + "/" +
                        String(tahun);
                        
    display.setTextSize(2); // Ukuran teks lebih kecil untuk tanggal
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(tanggalStr);
    display.display();
}

void showTemperatur(){
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

   // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
  static float state=100.00;

  display.drawBitmap(0, 1, logo_suhu, 32, 32, WHITE); 
  display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(34, 12);
  display.print((String)tempC);
  display.drawCircle((tempC<100)?98:109,14,2,WHITE); //FULL 109 END 98
  display.setCursor((tempC<100)?102:113,12);          //FULL 113 END 102
  display.print("C");
  display.display();
}

void showSend(uint8_t Direction,uint8_t Mode){
//  static uint32_t saveTmr=0;
//  static bool state=false;

//  if(millis() - saveTmr > 500){
//    saveTmr = millis();
//    state = !state;
//  }
  if(Mode){
    (Direction)?display.drawBitmap(90, 0, send_right, 40, 40, WHITE):display.drawBitmap(0, 0, send_left, 40, 40, WHITE); 
    display.drawBitmap(50, 0, icon_lamp, 32, 32, WHITE);
  }else{
    (Direction)?display.drawBitmap(90, 0, send_right, 40, 40, BLACK):display.drawBitmap(0, 0, send_left, 40, 40, BLACK);
    display.drawBitmap(50, 0, icon_lamp, 32, 32, BLACK)
  }

  display.display(); //tampilkan data

 }

void showVoltage(){
  display.drawBitmap(0, 0, petir, 40, 42, WHITE); 
  display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(45, 12);
  display.print(voltage());
  display.println(" V");
  display.display();
}

void showCurrent(){
  static uint32_t saveTmr=0;
  //static bool state=false;
  static char *nama[]={ "mA","A"};
  
  if(millis() - saveTmr > 100){
    saveTmr = millis();
    //state = !state;
  
  
  display.drawBitmap(-2, 1, amper, 40, 40, WHITE);
  display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(50, 12);
  //display.print("A:");
  display.print(current());
  display.print((current()<1000)?nama[0]:nama[1]);
  display.display();
  }
}
int voltage(){
//  vout     = (analogRead(vpin) * vref) / res_bit;
//  tegangan = 2.207 * vout + 0.2129;

//  vout     = analogRead(vpin);
//  tegangan = ((vout*0.00489) * 5);

  int vin = analogRead(VPINV);
  vout = (vin * vref) / res_bit;
  tegangan = vout / (R2/(R1+R2));
  return tegangan;
}

float current(){
  static float alpha = 0.1;
  //nilaiADC = analogRead(VPINC);
  // Baca nilai baru dari sensor
  float rawValue = analogRead(VPINC);

  // Terapkan low-pass filter (EMA)
  filteredValue = alpha * rawValue + (1 - alpha) * filteredValue;

  // Cetak nilai hasil filter
  Serial.print("Nilai hasil filter: ");
  Serial.println(filteredValue);

  float tegangan = filteredValue * 5 / 1023.0;

  float arus = (tegangan - 2.5) / 0.66;
  if(arus < 0.16){ arus = 0; }
  return arus;
}


void buttonSend(int buttonPin){
  static unsigned long startTime = 0; // Waktu ketika tombol ditekan
  static bool isLedOn = false;        // Status LED
  static int counter = 0;
  static bool buttonPressed = false; // Status tombol sebelumnya
 
  bool currentButtonState;//= digitalRead(button[buttonPin]) == LOW; // Tombol aktif saat LOW
  
  if(digitalRead(button[0]) == LOW){ currentButtonState = digitalRead(button[0]) == LOW; }
  else if(digitalRead(button[1]) == LOW){ currentButtonState = digitalRead(button[1]) == LOW }
  else{ currentButtonState = HIGH; }
  

  // Deteksi perubahan tombol dari tidak ditekan ke ditekan
  if (currentButtonState && !buttonPressed) {
    buttonPressed = true; // Catat bahwa tombol sedang ditekan
    isLedOn = true;       // Nyalakan LED
    startTime = millis(); // Mulai hitung waktu
   
    //counter++;
  } else if (!currentButtonState) {
    buttonPressed = false; // Reset status tombol
  }
  //(isLedOn)?mode = MODE_SEND:mode = MODE_CLOCK;
  if(isLedOn){
    mode = MODE_SEND;
   // showSend(buttonPin,buttonPin);
  }else{
    mode = MODE_CLOCK;
  }
  //if(isLedOn){stateSend = !stateSend;}
  
  // Matikan LED jika waktu telah melebihi 5 detik
  if (isLedOn && millis() - startTime >= 5000) {
    isLedOn = false;
//    digitalWrite(ledPin, LOW); // Matikan LED
  }
  Serial.println(String("counter=")+counter);
}

}
