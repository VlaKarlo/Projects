#include <arduinoFFT.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define SAMPLES 64        
#define  xres 32          
#define  yres 8           
#define ledPIN 6          
#define NUM_LEDS (xres * yres)
#define BRIGHTNESS 32

byte yvalue;
byte displaycolumn, displayvalue;
int peaks[xres];
byte displaycolor = 0;

double vReal[SAMPLES];
double vImag[SAMPLES];
byte data_avgs[xres];
arduinoFFT FFT = arduinoFFT(); 

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(NUM_LEDS, ledPIN, NEO_GRB + NEO_KHZ800);

bool EQ_ON = true; 
byte eq[32] = {50, 55, 60, 70, 75, 80, 85, 95,
               100, 100, 100, 100, 100, 100, 100, 100,
               100, 100, 100, 100, 100, 100, 100, 100,
               115, 125, 140, 160, 185, 200, 225, 255
              };

byte colors[][8] = {
  {170, 160, 150, 140, 130, 120, 1, 1},
  {1, 5, 10, 15, 20, 25, 90, 90},
  {90, 85, 80, 75, 70, 65, 1, 1},
  {90, 90, 90, 30, 30, 30, 1, 1},
  {170, 160, 150, 140, 130, 120, 0, 0},
  {170, 160, 150, 140, 130, 120, 1, 1},
  {170, 160, 150, 140, 130, 120, 1, 1}
};

void setup() {
  
  pixel.begin();  
  pixel.setBrightness(BRIGHTNESS);

 
  ADCSRA = 0b11100101;      
  ADMUX = 0b00000000;       
}

void loop() {
    
  for (int i = 0; i < SAMPLES; i++) {
    while (!(ADCSRA & 0x10));       
    ADCSRA = 0b11110101 ;           
    int value = ADC - 512 ;         
    vReal[i] = value / 8;           
    vImag[i] = 0;
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  
  int step = (SAMPLES / 2) / xres;
  int c = 0;
  for (int i = 0; i < (SAMPLES / 2); i += step) {
    data_avgs[c] = 0;
    for (int k = 0 ; k < step ; k++) {
      data_avgs[c] = data_avgs[c] + vReal[i + k];
    }
    data_avgs[c] = data_avgs[c] / step;
    c++;
  }

  for (int i = 0; i <xres; i++) {
    if (EQ_ON)
      data_avgs[i] = data_avgs[i] * (float)(eq[i]) / 100; 
    data_avgs[i] = constrain(data_avgs[i], 0, 80);        
    data_avgs[i] = map(data_avgs[i], 0, 80, 0, yres);     
    yvalue = data_avgs[i];
    peaks[i] = peaks[i] - 1;                              
    if (yvalue > peaks[i]) peaks[i] = yvalue;             
    yvalue = peaks[i];
    displaycolumn = i;
    displayvalue = yvalue;
    setColumn(displaycolumn, displayvalue);               
  }
  pixel.show();                                                                         
}

void setColumn(byte x, byte y) {
  byte led, i;

  for (i = 0; i < yres; i++) {
    led = GetLedFromMatrix(x, i); 
    if (peaks[x] > i) {
      switch (displaycolor) {
        case 4:
          if (colors[displaycolor][i] > 0) {
            pixel.setPixelColor(led, Wheel(colors[displaycolor][i]));
          }
          else {
            pixel.setPixelColor(led, 255, 255, 255);
          }
          break;

        case 5:
          pixel.setPixelColor(led, Wheel(x * 16));
          break;

        case 6:       
          pixel.setPixelColor(led, Wheel(i * 36));
          break;

        default:
          pixel.setPixelColor(led, Wheel(colors[displaycolor][i]));
      }
    }
    else {
      pixel.setPixelColor(led, 0);
    }
  }
}

byte GetLedFromMatrix(byte x, byte y) {
  x = xres - x - 1;
  if (x & 0x01) {
    return ((x + 1) * yres - y - 1);
  }
  else {
    return ((x + 1) * yres - yres + y);
  }
}
