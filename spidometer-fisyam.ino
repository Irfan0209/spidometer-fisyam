#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "RTClib.h"

#include "OneButton.h"

OneButton INP_LEFT(3,true);
OneButton INP_RIGHT(4,true);

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Icon.h"

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

// Membuat objek display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

RTC_DS3231 rtc;

DateTime now;

bool                showTime = true;  // Status untuk menampilkan waktu atau tanggal
unsigned long       saveTmrAnimation = 0; // Waktu sebelumnya dalam milidetik
const unsigned long delayAnimation = 4000; // Interval untuk bergantian (5 detik)

//variabel untuk sensor tegangan 
float tegangan,vout;
float vref = 5.0;
float res_bit = 1023.0;
float R1      = 30000.0;
float R2      = 7500.0;

//variabel untuk sensor arus
float filteredValue = 0;  // Nilai hasil filter

//variabel untuk menu sein
bool currentButtonState;
bool isLedOn = false;
unsigned long startTime = 0;
bool stateDirrection=0;//0 = left 1 = right
bool stateModeSein=false;

bool stateRTC = 1;
bool stateOverheat = 0;
float maxTemp = 40.00;

enum Mode{
  MODE_CLOCK,
  MODE_DATE,
  MODE_TEMP,
  MODE_VOLT,
  MODE_CURRENT,
  MODE_SEIN,
  MODE_WARNING
};
Mode mode;



void sendRightOn(){
   if(!stateOverheat){mode = MODE_SEIN;}
   stateDirrection = 1;
   stateModeSein = true;
   isLedOn = true;
   startTime = millis();
 
}

void sendRightOff(){
  stateDirrection = 1;
  stateModeSein = false;
}

void sendLeftOn(){
  if(!stateOverheat){mode = MODE_SEIN;}
  stateDirrection = 0;
  stateModeSein = true;
  isLedOn = true;
  startTime = millis();
}

void sendLeftOff(){
  stateDirrection = 0;
  stateModeSein = false;
}


void setup() {
  Serial.begin(115200);
  filteredValue = analogRead(VPINC);
  // Start up the library DS13B20
  sensors.begin();
  if(! rtc.begin()){
    stateRTC = 0;
  }

  

  INP_LEFT.attachLongPressStart(sendLeftOn);
  INP_LEFT.attachLongPressStop(sendLeftOff);
  
  INP_RIGHT.attachLongPressStart(sendRightOn);
  INP_RIGHT.attachLongPressStop(sendRightOff);
  
  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Jangan lanjutkan, berhenti di sini
  }

  display.clearDisplay();
  
 if(rtc.lostPower() && stateRTC == 1) {
    display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 0); 
    display.print("!  RTC  !");
    display.setCursor(0, 17); 
    display.print("LOST POWER");
    display.display();
    delay(10000);
  }
  
  display.clearDisplay();
}

void loop() {

 INP_LEFT.tick();
 INP_RIGHT.tick();
 
 static int count=0;
 unsigned long tmr = millis();
  
  if (tmr - saveTmrAnimation >= delayAnimation && mode != MODE_SEIN  && stateOverheat == false) {
    saveTmrAnimation = tmr;
  
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
   count++; 
}

switch(mode){
  case MODE_CLOCK :
    display.clearDisplay();
    showClock();
   // showHighTemperature();
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
  case MODE_SEIN :
    display.clearDisplay();
    buttonSend();
  break;
  case MODE_WARNING :
    display.clearDisplay();
    showHighTemperature();
  break;
}
Serial.println(String("stateOverheat:")+stateOverheat);
}

void showClock(){
   // Tampilkan waktu tanpa detik
   if (stateOverheat) return;
    now = rtc.now();
    String waktu;
    int jam = now.hour();
    int menit = now.minute();
    char Jam[5],Menit[5];
    static bool flag;
    static uint32_t saveTmr=0;
    uint32_t tmr = millis();

    sprintf(Jam,"%02d",jam);
    sprintf(Menit,"%02d",menit);
  if(stateRTC){ 
    display.setTextSize(4); // Ukuran teks lebih besar untuk jam dan menit
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(3, 1); display.print(Jam);
    display.setCursor(77, 1); display.print(Menit);

    if(tmr-saveTmr >500){
      saveTmr=tmr;
      flag = !flag;
    }

    if(flag){
      display.fillCircle(62,7,4,SSD1306_INVERSE);
      display.fillCircle(62,21,4,SSD1306_INVERSE);
    }else{
      display.fillCircle(62,7,4,BLACK);
      display.fillCircle(62,21,4,BLACK);
    }
  }else{
    display.drawBitmap(45, 0, icon_clock_error, 31, 31, WHITE); 
  }
  display.display();
}

void showDate(){
  // Tampilkan tanggal
    if(stateOverheat) return;
    now = rtc.now();
    int tanggal = now.day();
    int bulan = now.month();
    int tahun = now.year();
    String tanggalStr = (tanggal < 10 ? "0" : "") + String(tanggal) + "/" +
                        (bulan < 10 ? "0" : "") + String(bulan) + "/" +
                        String(tahun);
                        
  if(stateRTC){
    display.setTextSize(2); // Ukuran teks lebih kecil untuk tanggal
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println(tanggalStr);
  }else{
    display.drawBitmap(45, -4, icon_kalender_error, 31, 40, WHITE); 
   // display.display();
  }
  display.display();
}

float requestTemp(){
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  return tempC;
}

void cekTemp(){
  float tempC = requestTemp();
  if(tempC > 45.00){
    stateOverheat = 1;
    mode = MODE_WARNING;
  }else{
    stateOverheat = 0;
    //mode = MODE_CLOCK;
  }
}

void showTemperatur(){
  if(stateOverheat) return;
  
  float tempC = requestTemp();
   // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    display.drawBitmap(0, 1, icon_temp, 32, 32, WHITE); 
    display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(34, 12);
    display.print((String)tempC);
    display.drawCircle((tempC<100)?98:109,14,2,WHITE); //FULL 109 END 98
    display.setCursor((tempC<100)?102:113,12);          //FULL 113 END 102
    display.print("C");
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
    if(tempC >= maxTemp){
    stateOverheat = 1;
    mode = MODE_WARNING;
    }
  } 
  else
  {
    display.drawBitmap(45, -3, icon_error_temp, 40, 38, WHITE); 
    Serial.println("Error: Could not read temperature data");
  }
  display.display();
}

void showSein(bool Direction,bool Mode){
  if(Mode){
    if(Direction){
      display.drawBitmap(90, 0, icon_sein_right, 40, 40, WHITE);//kanan hidup
      display.drawBitmap(0, 0, icon_sein_left, 40, 40, WHITE);//kiri hidup
     }
    else{
      display.drawBitmap(0, 0, icon_sein_left, 40, 40, WHITE);//kiri hidup
      display.drawBitmap(90, 0, icon_sein_right, 40, 40, WHITE);//kanan hidup
    }
    display.drawBitmap(50, 0, icon_lamp_on, 32, 32, WHITE);
  }else{
    if(Direction){
      display.drawBitmap(90, 0, icon_sein_right, 40, 40, BLACK);//kanan mati
      display.drawBitmap(0, 0, icon_sein_left, 40, 40, WHITE);//KIRI HIDUP
     }
    else{
      display.drawBitmap(0, 0, icon_sein_left, 40, 40, BLACK);
      display.drawBitmap(90, 0, icon_sein_right, 40, 40, WHITE);
    }
    display.drawBitmap(44, 2, icon_lamp_off, 32, 32, WHITE);
  }
  display.display(); //tampilkan data
 }

void showVoltage(){
  if(stateOverheat) return;
  display.drawBitmap(0, 0, icon_petir, 40, 42, WHITE); 
  display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(45, 12);
  display.print(voltage());
  display.println(" V");
  display.display();
}

void showCurrent(){
  if(stateOverheat) return;
  static uint32_t saveTmr=0;
  static char *nama[]={ "mA","A"};
  
  if(millis() - saveTmr > 100){
    saveTmr = millis();
    
  display.drawBitmap(-2, 1, icon_ampere, 40, 40, WHITE);
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

void showHighTemperature(){
  static uint32_t saveTmr = 0;
  static bool flag = false;
  uint32_t tmr = millis();
  float tempC = requestTemp();
  if(tempC == DEVICE_DISCONNECTED_C){tempC = 0; }
   
  if(tmr - saveTmr > 100){
    saveTmr = tmr;
    flag = !flag;
    display.drawBitmap(0, 7, icon_warning, 26, 26, WHITE);//:display.drawBitmap(0, 7, icon_warning, 26, 26, BLACK); 
    display.setTextSize(2); // Ukuran teks lebih besar untuk jam dan menit
    (flag)?display.setTextColor(SSD1306_WHITE):display.setTextColor(SSD1306_BLACK);
    display.setCursor(31, 12);
    display.print("OVERHEAT");
    display.display();
  }
  Serial.println(String("temp:")+tempC);
  if(tempC < maxTemp){stateOverheat = 0; mode = MODE_CLOCK; }
}
float current(){
  static float alpha = 0.1;
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


void buttonSend(){
  if(isLedOn){
    
    showSein(stateDirrection,stateModeSein);
    Serial.println(String("stateDirrection=")+stateDirrection);
    Serial.println(String("stateModeSein  =")+stateModeSein);
  }
  
  // Matikan LED jika waktu telah melebihi 5 detik
  if (isLedOn && millis() - startTime >= 5000) {
    isLedOn = false;
    mode = MODE_CLOCK;
  }
  
}
