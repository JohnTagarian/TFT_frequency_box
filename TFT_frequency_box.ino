#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include "Math.h"

#include <FreqCount.h>

unsigned long preTime;


const int pulsePin = 5;

// parameter
int frequency = 125;
int period;
int time_on;
int time_off;
int duty;

char ch;

// variable process
char state_scr = '0';
int state_mipl[2] = {ILI9341_WHITE, ILI9341_BLACK};
int state_cw[2] = {ILI9341_BLUE, ILI9341_BLACK};
int state_ccw[2] = {ILI9341_MAROON, ILI9341_BLACK};
int state_home_text[2] = {ILI9341_WHITE, ILI9341_BLACK};
int state_home_ed[2] = {ILI9341_BLACK, ILI9341_DARKGREEN};

#define IN1 4
#define IN2 2
#define PWM 6


int speed_motor = 250;

int state_home2_ed[2] = {ILI9341_BLACK, ILI9341_RED};
int state_home2_btn[2] = {ILI9341_RED, ILI9341_BLACK};


//variable time
unsigned long pre_time_des;
unsigned long pre_time_wave;

#include <Fonts/c__windows_fonts_BRITANIC12pt7b.h>
#include <Fonts/c__windows_fonts_BRITANIC18pt7b.h>
#include <Fonts/c__windows_fonts_BRITANIC24pt7b.h>
#include <Fonts/c__windows_fonts_BRITANIC36pt7b.h>

#include <Fonts/DSEG7_Classic_Mini_Bold_18.h>
#include <Fonts/DSEG7_Classic_Mini_Bold_22.h>
#include <Fonts/DSEG7_Classic_Mini_Bold_24.h>
#include <Fonts/DSEG7_Classic_Mini_Bold_28.h>
#include <Fonts/DSEG7_Classic_Mini_Bold_36.h>

// Fonts use
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMonoBoldOblique18pt7b.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>



int i;
String sep[3] = {"  ", " ", ""};
String sep_time[4] = {"  ", " ", ""};
String sep_motor[3] = {"00", "0", ""};
String text_wire[2] = {"BROKEN!", "CONNECT"};
String text_hz[2] = {"Hz", "kHz"};

#define CS_PIN 7
// MOSI=11, MISO=12, SCK=13
int x, y;
//XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN  2

bool onetime_1;

XPT2046_Touchscreen ts(CS_PIN);



// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);




// calibration values
float xCalM = -0.09, yCalM = 0.07; // gradients
float xCalC = 335.83, yCalC = -13.95; // y axis crossing points

int8_t blockWidth = 20; // block size
int8_t blockHeight = 20;
int16_t blockX = 0, blockY = 0; // block position (pixels)

class ScreenPoint {
  public:
    int16_t x;
    int16_t y;

    // default constructor
    ScreenPoint() {
    }

    ScreenPoint(int16_t xIn, int16_t yIn) {
      x = xIn;
      y = yIn;
    }
};

ScreenPoint getScreenCoords(int16_t x, int16_t y) {
  int16_t xCoord = round((x * xCalM) + xCalC);
  int16_t yCoord = round((y * yCalM) + yCalC);
  if (xCoord < 0) xCoord = 0;
  if (xCoord >= tft.width()) xCoord = tft.width() - 1;
  if (yCoord < 0) yCoord = 0;
  if (yCoord >= tft.height()) yCoord = tft.height() - 1;
  return (ScreenPoint(xCoord, yCoord));
}

void calibrateTouchScreen() {
  TS_Point p;
  int16_t x1, y1, x2, y2;

  tft.fillScreen(ILI9341_BLACK);
  // wait for no touch
  while (ts.touched());
  tft.drawFastHLine(10, 20, 20, ILI9341_RED);
  tft.drawFastVLine(20, 10, 20, ILI9341_RED);
  while (!ts.touched());
  delay(50);
  p = ts.getPoint();
  x1 = p.x;
  y1 = p.y;
  tft.drawFastHLine(10, 20, 20, ILI9341_BLACK);
  tft.drawFastVLine(20, 10, 20, ILI9341_BLACK);
  delay(500);
  while (ts.touched());
  tft.drawFastHLine(tft.width() - 30, tft.height() - 20, 20, ILI9341_RED);
  tft.drawFastVLine(tft.width() - 20, tft.height() - 30, 20, ILI9341_RED);
  while (!ts.touched());
  delay(50);
  p = ts.getPoint();
  x2 = p.x;
  y2 = p.y;
  tft.drawFastHLine(tft.width() - 30, tft.height() - 20, 20, ILI9341_BLACK);
  tft.drawFastVLine(tft.width() - 20, tft.height() - 30, 20, ILI9341_BLACK);

  int16_t xDist = tft.width() - 40;
  int16_t yDist = tft.height() - 40;

  // translate in form pos = m x val + c
  // x
  xCalM = (float)xDist / (float)(x2 - x1);
  xCalC = 20.0 - ((float)x1 * xCalM);
  // y
  yCalM = (float)yDist / (float)(y2 - y1);
  yCalC = 20.0 - ((float)y1 * yCalM);

  Serial.print("x1 = "); Serial.print(x1);
  Serial.print(", y1 = "); Serial.print(y1);
  Serial.print("x2 = "); Serial.print(x2);
  Serial.print(", y2 = "); Serial.println(y2);
  Serial.print("xCalM = "); Serial.print(xCalM);
  Serial.print(", xCalC = "); Serial.print(xCalC);
  Serial.print("yCalM = "); Serial.print(yCalM);
  Serial.print(", yCalC = "); Serial.println(yCalC);

}




void read_pram() {

  if (FreqCount.available()) {
    frequency = FreqCount.read();
    ("Frequency : " + String(frequency));
  }

  //  frequency = (frequency >= 1000) ? frequency / 1000 : frequency;
  if (frequency >= 2000)
    period = (1.0 / (float)(frequency)) * 1000000.0;
  else period = (1.0 / (float)(frequency)) * 1000.0;

  if (frequency < 20 ) {
    time_on = period / 2;
    time_off = period / 2;

  }
  else {
    int time_on_psu = pulseIn(pulsePin, HIGH );
    int time_off_psu = pulseIn(pulsePin, LOW );
    time_on = (time_on_psu != 0) ? time_on_psu / 1000 : time_on ;
    time_off = (time_off_psu != 0) ? time_off_psu / 1000 : time_off ;

  }
  duty = frequency < 500 ? (int)(((float)time_on / (float)period) * 100.0) : 50;



}







void setup() {
  Serial.begin(9600);
  FreqCount.begin(1000);
  pinMode(pulsePin, INPUT);

  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(PWM,OUTPUT);



  pinMode(TFT_CS, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(CS_PIN, HIGH);

  // tft seting
  tft.begin();
  tft.setRotation(3);
  ts.begin();
  ts.setRotation(3);
  //  calibrateTouchScreen();



  while (!Serial && (millis() <= 1000));


  home_scr_2();
  //  wave_mesure_scr();
  //  test_motor_screen();
  //  wire_check_scr();
}

void loop() {
  //  write_text_freq(sep[(String(i).length())-1] + String(i), 20, 148);
  ScreenPoint sp = ScreenPoint();
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    sp = getScreenCoords(p.x, p.y);
    x = sp.x;
    y = sp.y;

  }


  switch (state_scr) {
    case '0':
      onetime_1 = false;
      process_home_scr2();
      break;
    case '1':
      process_test_motor_scr();
      break;
    case '2':
      read_pram();
      process_wave_mes();

      break;
  }




}
void btn_home(String text, int x , int y , int w, int h , int r, int cx, int cy, bool act) {

  tft.fillRoundRect(x, y, w, h, r, ILI9341_RED);
  tft.drawRoundRect(x, y, w, h, r, state_home_ed[act]);
  tft.setCursor(cx, cy);
  tft.setTextColor(state_home_text[act]);
  tft.setTextSize(2);
  tft.setFont();
  tft.println(text);


}

void btn_home_2(String text, int x , int y , int w, int h , int r, int cx, int cy, bool act) {

  tft.fillRoundRect(x, y, w, h, r, state_home2_btn[act]);
  tft.drawRoundRect(x, y, w, h, r, state_home2_ed[act]);
  tft.setCursor(cx, cy);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setFont();
  tft.println(text);


}

void back(int ad) {
  if (x >= 10 && x <= 78 && y >= 10 && y <= 38 && ts.touched() ) {

    btn_back("<BACK", 10 , 10 , 68, 28 , 10, 12, 17 + ad, true);
    while (ts.touched());
    x = 0;
    y = 0;
    home_scr_2();
    state_scr = '0';
  }
}

void btn_back(String text, int x , int y , int w , int h  , int r , int cx , int cy, bool act) {
  tft.fillRoundRect(x, y, w, h, r, ILI9341_RED);
  tft.drawRoundRect(x, y, w, h, r, state_home_ed[act]);
  tft.setCursor(cx, cy);
  tft.setTextColor(state_home_text[act]);
  tft.setFont();
  tft.setTextSize(2);
  tft.println(text);


}
void btn_cw_mt(bool act, bool onetime) {
  static int y;
  if (onetime) {
    tft.fillRect(0, 150, 160, 90, state_cw[act]);
    tft.drawRect(0, 150, 160, 90, ILI9341_BLACK);
    tft.setCursor(45, 205 + y);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
    tft.println("CW");
    y = 5;
    onetime = false;
  }



}

void btn_ccw_mt(bool act, bool onetime) {
  if (onetime) {
    tft.fillRect(160, 150, 160, 90, state_ccw[act]);
    tft.drawRect(160, 150, 160, 90, ILI9341_BLACK);
    tft.setCursor(198, 210);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
    tft.println("CCW");
    onetime = false;
  }


}





// process home
void process_home_scr() {
  if (x >= 15 && x <= 155 && y >= 90 && y <= 130 && ts.touched()) {
    btn_home("MORTEST", 15, 90, 140, 40, 10, 46, 103, true);
    while (ts.touched());
    x = 0;
    y = 0;
    test_motor_scr();
    state_scr = '1';
  }
  if (x >= 175 && x <= 310 && y >= 90 && y <= 130 && ts.touched()) {
    btn_home("WAVEMESURE", 170, 90, 140, 40, 10, 183, 101, true);
    while (ts.touched());
    wave_mesure_scr();
    state_scr = '2';
  }
}

void process_home_scr2() {
  if (x >= 50 && x <= 265 && y >= 110 && y <= 155 && ts.touched()) {
    btn_home_2("MOTORTESTER", 50, 100, 215, 45, 10, 95, 115, true);
    while (ts.touched());
    x = 0;
    y = 0;
    test_motor_scr();
    state_scr = '1';
  }
  if (x >= 50 && x <= 265 && y >= 180 && y <= 230 && ts.touched()) {
    btn_home_2("WAVEMESUREMENT", 50, 170, 215, 45, 10, 78, 184, true);
    while (ts.touched());
    wave_mesure_scr();
    state_scr = '2';
  }
}



//process motor
void process_test_motor_scr() {
  if (!onetime_1) {
    btn_cw_mt(false, true);
    btn_ccw_mt(false, true);

    write_text_mt(sep_motor[String(speed_motor).length() - 1] + String(speed_motor), 135, 130);
    onetime_1 = true;
  }
  if (!ts.touched()) {
    draw_minus(40, 112, 35, 10, false);
    draw_plus(246, 112, 25, 10, false);

  }

  back(0);
  if (x >= 24 && x <= 80 && y >= 105 && y <= 150 && ts.touched()) {
    draw_minus(40, 112, 35, 10, true);
    change_speed(false);
    x = 0;
    y = 0;
  }
  else if (x >= 235 && x <= 290 && y >= 105 && y <= 150 && ts.touched()) {
    draw_plus(246, 112, 25, 10, true);
    change_speed(true);
    x = 0;
    y = 0;
  }
  else if (x >= 0 && x <= 160 && y >= 150 && y <= 240 && ts.touched()) {
    btn_cw_mt(true, true);
    CW(speed_motor);
    while (ts.touched());
    STOP();
    btn_cw_mt(false, true);
    x = 0;
    y = 0;

  }
  else if (x >= 160 && x <= 320 && y >= 150 && y <= 240 && ts.touched()) {
    btn_ccw_mt(true, true);
    CCW(speed_motor);
    while (ts.touched());
    STOP();
    btn_ccw_mt(false, true);
    x = 0;
    y = 0;
  }

}

void change_speed(bool incre) {
  if (millis() - pre_time_des > 100) {
    pre_time_des = millis();
    speed_motor += incre ? 1 : -1;
    if (speed_motor < 0) speed_motor = 255;
    else if (speed_motor > 255) speed_motor = 0;
    write_text_mt(sep_motor[String(speed_motor).length() - 1] + String(speed_motor), 135, 125);
  }
}


//process wave
void process_wave_mes() {

  //    frequency += 10;
  if (millis() - pre_time_wave > 1000) {
    pre_time_wave = millis();


    if (frequency > 10000) {
      write_text_freq("ERR", 20, 148);
      if (frequency >= 1000)write_nor_prop(text_hz[1], 113, 145);
      else write_nor_prop(text_hz[0], 113, 145);

      write_text_period(" ERR", 170, 148);
      write_nor_prop("ms", 270, 145);



      write_text_time(" ERR", 45, 201);
      write_text_time(" ERR", 45, 223);


      write_text_duty("ERR%", 180, 223);
    }
    else {
      String frequencys = (frequency >= 1000) ? String(frequency / 1000) : String(frequency);
      write_text_freq(sep[(frequencys.length()) - 1] + frequencys, 20, 148);
      if (frequency >= 1000)write_nor_prop(text_hz[1], 113, 145);
      else write_nor_prop(text_hz[0], 113, 145);

      if (frequency >= 2000) {
        String periods = String(period);
        write_text_period(sep[periods.length() - 1] + periods, 170, 148);
        write_nor_prop("us", 270, 145);
      }
      else {
        String periods = String(period);
        write_text_period(sep[periods.length() - 1] + periods, 170, 148);
        write_nor_prop("ms", 270, 145);
      }


      String time_ons = String(time_on);
      String time_offs = String(time_off);
      write_text_time(sep[time_offs.length() - 1] + time_offs, 45, 201);
      write_text_time(sep[time_ons.length() - 1] + time_ons, 45, 223);

      String dutys = String(duty) + "%" ;
      write_text_duty(sep[dutys.length() - 2] + dutys, 180, 223);
    }



  }
  back(5);
}

// write text

void write_text(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_MAROON);
  tft.setFont(&c__windows_fonts_BRITANIC12pt7b);
  tft.setTextSize(1);
  tft.fillRect(x, y - 30, 80, 40, ILI9341_WHITE);
  tft.println(i);
}

void write_text_mt(String i, int x , int y) {
  tft.setCursor(x + 7, y - 14);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont();
  tft.setTextSize(2);
  tft.fillRect(x, 95, 80, 44, ILI9341_BLACK);
  tft.println(i);
}

void write_text_freq(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.fillRect(x, y - 30, 130, 37, ILI9341_BLUE);
  tft.println(i);
}

void write_text_period(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.fillRect(x, y - 30, 135, 37, ILI9341_BLACK);
  tft.println(i);
}

void write_text_time(String i, int x , int y) {
  tft.setCursor(x, y - 7);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont();
  tft.setTextSize(2);
  tft.fillRect(x, y - 16, 60, 25, ILI9341_MAROON);
  tft.println(i);
}

void write_text_duty(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.fillRect(x, y - 30, 125, 37, ILI9341_DARKGREEN);
  tft.println(i);
}

void write_text_wire(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.fillRect(0, y - 30, 320, 37, ILI9341_BLACK);
  tft.println(i);
}

void write_text_hz(String i) {
  tft.setCursor(113, 145);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont();
  tft.setTextSize(2);
  tft.fillRect(107, 136, 43, 22, ILI9341_BLUE);
  tft.println(i);
}

////


void load_tab(String i, int x , int y, float per) {
  tft.setCursor(x + 30, y);
  tft.setTextColor(ILI9341_DARKGREEN);
  tft.setFont(&DSEG7_Classic_Mini_Bold_24);
  tft.setTextSize(1);

  tft.fillRect(x - 25, y - 30, 150, 40, ILI9341_WHITE);
  tft.fillRect(x - 25, y - 30, (int)(per * 1.5), 40, ILI9341_BLUE);
  tft.drawRect(x - 25, y - 30, 150, 40, ILI9341_BLUE);
  tft.println(i);
}




void write_nor(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_DARKGREEN);
  tft.setFont(&c__windows_fonts_BRITANIC18pt7b);
  tft.setTextSize(1);
  tft.println(i);
}

void write_nor_prop(String i, int x , int y) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont();
  tft.setTextSize(2);
  tft.println(i);
}

void pulse() {
  tft.fillRect(10, 180, 20, 3, ILI9341_BLUE);
  tft.fillRect(30, 180, 3, 20, ILI9341_BLUE);

  tft.fillRect(30, 200, 20, 3, ILI9341_BLUE);
  tft.fillRect(50, 182, 3, 21, ILI9341_BLUE);



  tft.fillRect(50, 180, 20, 3, ILI9341_BLUE);
  tft.fillRect(70, 180, 3, 20, ILI9341_BLUE);

  tft.fillRect(70, 200, 20, 3, ILI9341_BLUE);
  tft.fillRect(90, 182, 3, 21, ILI9341_BLUE);

  tft.fillRect(90, 180, 20, 3, ILI9341_BLUE);

  tft.fillRect(110, 180, 3, 20, ILI9341_BLUE);

  tft.fillRect(110, 200, 20, 3, ILI9341_BLUE);
}


void tab_bar_mt(int y , int h) {
  tft.fillRoundRect(25, y, 75, h, 18, ILI9341_MAROON);
  tft.drawRoundRect(25, y, 75, h, 18, ILI9341_BLACK);


  tft.fillRoundRect(180, y, 115, h, 18, ILI9341_DARKGREEN);
  tft.drawRoundRect(180, y, 115, h, 18, ILI9341_BLACK);


  tft.fillRect(85, y, 150, h, ILI9341_BLACK);
  tft.drawRect(85, y, 150, h, ILI9341_BLACK);

  draw_minus(40, 112, 35, 10, false);
  draw_plus(246, 112, 25, 10, false);




}
void draw_minus(int x , int y , int w , int th, bool act) {

  tft.fillRect(x, y, w, th, state_mipl[act]);
}


void draw_plus(int x , int y , int w , int th, int act) {
  tft.fillRect(x, y, w + 10, th, state_mipl[act]);
  tft.fillRect((x + (w / 2)), y - ((w + 5) / 2) + 2, th, w + 10, state_mipl[act]);


}




// active






// SCREEN ----------------------------------

void home_scr() {

  tft.fillScreen(tft.color565(255, 255, 255));
  tft.fillRect(0, 0, 320, 80, ILI9341_NAVY);
  tft.drawFastHLine(0, 80, 320, ILI9341_BLACK);

  tft.fillRoundRect(25, 55, 290, 2, 25, ILI9341_ORANGE);
  tft.setCursor(35, 40);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.println("UNIVERSURE");

  tft.setCursor(80, 67);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setFont();
  tft.setTextSize(2);
  tft.println("Select function");

  btn_home("MORTEST", 15, 90, 140, 40, 10, 46, 103, false);
  btn_home("WIRETEST", 15, 140, 140, 40, 10, 40, 150, false);
  btn_home("RESISTANCE", 15, 190, 140, 40, 10, 24, 200, false);

  btn_home("WAVEMESURE", 170, 90, 140, 40, 10, 183, 101, false);
  btn_home("CURRENT", 170, 140, 140, 40, 10, 200, 150, false);
  btn_home("VOLTAGE", 170, 190, 140, 40, 10, 200, 200, false);
  //  btn_home();
}


void home_scr_2() {

  tft.fillScreen(tft.color565(255, 255, 255));
  tft.fillRect(0, 0, 320, 80, ILI9341_NAVY);
  tft.drawFastHLine(0, 80, 320, ILI9341_BLACK);

  tft.fillRoundRect(25, 55, 290, 2, 25, ILI9341_ORANGE);
  tft.setCursor(35, 40);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.println("UNIVERSURE");

  tft.setCursor(80, 67);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setFont();
  tft.setTextSize(2);
  tft.println("Select function");
  //
  btn_home_2("MOTORTESTER", 50, 100, 215, 45, 10, 95, 115, false);
  btn_home_2("WAVEMESUREMENT", 50, 170, 215, 45, 10, 78, 184, false);

}


void test_motor_scr() {

  tft.fillScreen(tft.color565(255, 255, 255));
  tft.fillRect(0, 0, 320, 82, tft.color565(30, 15, 124));
  tft.drawFastHLine(0, 82, 320, ILI9341_BLACK);


  tft.setCursor(95, 38);
  tft.fillRoundRect(64, 53, 255, 2, 25, ILI9341_ORANGE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.println("MORTEST");

  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setCursor(70, 62);
  tft.println("Set speed motor");

  btn_back("<BACK", 10 , 10 , 68, 28 , 10, 12, 17, false);
  tab_bar_mt(95, 44);

  write_text_mt(String(speed_motor), 135, 125);

  //  btn_cw_mt(false, true);
  //  btn_ccw_mt(false, true);

}


void frame_1() {
  tft.fillRect(0, 82, 160, 79, ILI9341_BLUE);
  tft.drawRect(0, 82, 160, 79, ILI9341_BLACK);


  write_nor_prop("Frequency", 15, 95);


}

void frame_2() {
  tft.fillRect(160, 82, 160, 79, ILI9341_BLACK);
  tft.drawRect(160, 82, 160, 79, ILI9341_BLACK);
  write_nor_prop("Period", 180, 95);
}

void frame_3() {
  tft.fillRect(0, 161, 160, 79, ILI9341_MAROON);
  tft.drawRect(0, 161, 160, 79, ILI9341_BLACK);
  write_nor_prop("Time (ms)", 10, 168);
  write_nor_prop("OFF", 110, 193);
  write_nor_prop("ON", 110, 219);
}

void frame_4() {
  tft.fillRect(160, 161, 160, 79, ILI9341_DARKGREEN);
  tft.drawRect(160, 161, 160, 79, ILI9341_BLACK);
  write_nor_prop("Duty", 180, 168);

}


void wave_mesure_scr() {
  tft.fillScreen(tft.color565(255, 255, 255));
  tft.fillRect(0, 0, 320, 82, tft.color565(30, 15, 124));
  tft.drawFastHLine(0, 82, 320, ILI9341_BLACK);


  tft.setCursor(95, 42);
  tft.fillRoundRect(64, 53, 255, 2, 25, ILI9341_ORANGE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.println("WAVEMES");


  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setCursor(70, 60);
  tft.println("Squre wave parameter");

  btn_back("<BACK", 10 , 10 , 68, 28 , 10, 12, 17, false);
  frame_1();
  frame_2();
  frame_3();
  frame_4();

  //  // frame1
  //  write_text_freq(sep[(String("212").length()) - 1] + "212", 20, 148);
  //  write_nor_prop("kHz", 113, 145);
  //
  //  // frame2
  //  write_text_period(sep[(String("132").length()) - 1] + "132", 170, 148);
  //  write_nor_prop("ms", 270, 145);
  //
  //
  //  // frame 3
  //    write_text_time(sep_time[(String("123.2").length()) - 3] + "123.2", 20, 206);
  //    write_text_time(sep_time[(String("1.2").length()) - 3] + "1.2", 20, 228);
  //
  //  //frame4
  //  write_text_duty(sep[(String("50%").length()) - 2] + "50%", 180, 223);


}

void wire_check_scr() {

  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 82, tft.color565(30, 15, 124));
  tft.drawFastHLine(0, 82, 320, ILI9341_WHITE);


  tft.setCursor(95, 42);
  tft.fillRoundRect(50, 53, 269, 2, 25, ILI9341_ORANGE);
  tft.setTextColor(ILI9341_WHITE);
  tft.setFont(&c__windows_fonts_BRITANIC24pt7b);
  tft.setTextSize(1);
  tft.println("WIRETEST");


  tft.setFont(&c__windows_fonts_BRITANIC12pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setCursor(50, 74);
  tft.println("Check for breakage wire");


  btn_back("<BACK", 10 , 10 , 68, 28 , 10, 12, 23, false);
  write_nor_prop("CIRCUIT CONDITION", 75, 120);
  update_wire_scr();


}
void STOP() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(PWM, 0);
}
void CW(int sp) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(PWM, sp);
}
void CCW(int sp) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(PWM, sp);
}

void update_wire_scr() {

  write_text_wire(text_wire[0], 65, 200);
  tft.fillCircle(40 , 120, 25, ILI9341_RED);
  tft.drawCircle(40 , 120, 25, ILI9341_BLACK);

}
