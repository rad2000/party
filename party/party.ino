#include<FastLED.h>
#include <LEDMatrix.h>
//#include <LEDText.h>
//#include <FontRobotron.h>
#include <LEDSprites.h>

#define LED_PIN     7
#define BRIGHTNESS  150
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB

#define STROBE 4
#define RESET 5
#define LEFT 0
#define RIGHT 1
#define MAXINPUT 1023.0
int left[7];
int right[7];
#define BEAT_MULTIPLIER 1.5

int minValue = 1023;
int maxValue = 0;

int beat = 0;

#define MATRIX_WIDTH 24
#define MATRIX_HEIGHT 8
#define NUM_LEDS MATRIX_WIDTH * MATRIX_HEIGHT
#define MATRIX_TYPE VERTICAL_ZIGZAG_MATRIX
#define MAX_DIMENSION ((MATRIX_WIDTH>MATRIX_HEIGHT) ? MATRIX_WIDTH : MATRIX_HEIGHT)

cLEDMatrix<-MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint16_t base_animation_speed = 20; // animation_speed is set dynamically once we've started up
uint16_t animation_speed = 20; // animation_speed is set dynamically once we've started up

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.
uint16_t animation_scale = 30; // scale is set dynamically once we've started up

// This is the array that we keep our computed noise values in
uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

CRGBPalette16 currentPalette( PartyColors_p );

uint8_t       colorLoop = 1;

static uint16_t x;
static uint16_t y;
static uint16_t z;

/*
cLEDText ScrollingMsg;
const unsigned char msg[] = { "PICKLE PARTY!" };
*/
const uint8_t pickleData[] = {
  B8_5BIT(00000000),B8_5BIT(00000000),B8_5BIT(00000000),
  B8_5BIT(00234540),B8_5BIT(00000004),B8_5BIT(54454300),
  B8_5BIT(06778849),B8_5BIT(000000AB),B8_5BIT(CBDEEFG0),
  B8_5BIT(067H4II8),B8_5BIT(J53DK5IL),B8_5BIT(MNDLOOP0),
  B8_5BIT(066HJM5Q),B8_5BIT(3DMMRINB),B8_5BIT(NDNRNNA0),
  B8_5BIT(00STUV2J),B8_5BIT(5J55Q55K),B8_5BIT(C5QERF00),
  B8_5BIT(0000H8JD),B8_5BIT(2QJQQM4L),B8_5BIT(4QI00000),
  B8_5BIT(00000000),B8_5BIT(00000000),B8_5BIT(00000000) };
const uint8_t pickleMask[] = {
  B8_1BIT(00000000),B8_1BIT(00000000),B8_1BIT(00000000),
  B8_1BIT(00111110),B8_1BIT(00000001),B8_1BIT(11111100),
  B8_1BIT(01111111),B8_1BIT(00000011),B8_1BIT(11111110),
  B8_1BIT(01111111),B8_1BIT(11111111),B8_1BIT(11111110),
  B8_1BIT(01111111),B8_1BIT(11111111),B8_1BIT(11111110),
  B8_1BIT(00111111),B8_1BIT(11111111),B8_1BIT(11111100),
  B8_1BIT(00001111),B8_1BIT(11111111),B8_1BIT(11100000),
  B8_1BIT(00000000),B8_1BIT(00000000),B8_1BIT(00000000) };
const struct CRGB pickleCols[31] = { CRGB(0,0,0), CRGB(71,95,34), CRGB(116,133,41), CRGB(87,115,49), CRGB(65,99,18), CRGB(35,48,12), CRGB(24,49,3), CRGB(45,72,13), CRGB(175,187,112), CRGB(151,177,48), CRGB(104,139,60), CRGB(148,167,92), CRGB(117,150,52), CRGB(104,149,24), CRGB(144,181,19), CRGB(142,196,77),
   CRGB(47,65,19), CRGB(86,117,37), CRGB(57,85,21), CRGB(80,118,10), CRGB(109,140,21), CRGB(98,131,41), CRGB(94,131,22), CRGB(82,131,4), CRGB(157,208,130), CRGB(74,107,28), CRGB(65,104,5), CRGB(117,128,75), CRGB(73,90,39), CRGB(94,107,36), CRGB(195,181,85) };
cSprite Pickle(24, 8, pickleData, 1, _5BIT, pickleCols, pickleMask);

cLEDSprites Sprites(&leds);

CRGBPalette16 whitePalette;
     

void setup() {
  Serial.begin(38400);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  
  FastLED.show();
  
  pinMode(RESET, OUTPUT);
  pinMode(STROBE, OUTPUT);
  digitalWrite(RESET, LOW);
  digitalWrite(STROBE, HIGH);

  fill_solid( whitePalette, 16, CRGB::Black);
  whitePalette[0] = CRGB::White;
  whitePalette[7] = CRGB::White;
  whitePalette[8] = CRGB(128,128,128);


/*
  ScrollingMsg.SetFont(RobotronFontData);
  ScrollingMsg.Init(&leds, MATRIX_WIDTH, MATRIX_HEIGHT, 0, 0);
  ScrollingMsg.SetText((unsigned char*)msg, sizeof(msg)-1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg.SetScrollDirection(SCROLL_LEFT);
  ScrollingMsg.SetTextDirection(CHAR_UP);
  ScrollingMsg.SetFrameRate(5);
  */
  Pickle.SetPositionFrameMotionOptions(0/*X*/, 0/*Y*/, 0/*Frame*/, 0/*FrameRate*/, 0/*XChange*/, 0/*XRate*/, 0/*YChange*/, 0/*YRate*/, SPRITE_X_KEEPIN | SPRITE_DETECT_EDGE);
  Sprites.AddSprite(&Pickle);
}

void readMSGEQ7() {
  // 0 = 63Hz
  // 1 = 160Hz
  // 2 = 400Hz
  // 3 = 1kHz
  // 4 = 2.5kHz
  // 5 = 6.25kHz
  // 6 = 16kHz

  digitalWrite(RESET, HIGH);
  digitalWrite(RESET, LOW);
  int band;
  for(band=0; band<7; band++) {
    digitalWrite(STROBE, LOW);
    delayMicroseconds(30);
    left[band] = analogRead(LEFT);
    right[band] = analogRead(RIGHT);
    
    digitalWrite(STROBE, HIGH);
  }
  
  int leftVal = left[0];
  int rightVal = right[0];
  
  if(leftVal < minValue) minValue = leftVal;
  if(leftVal > maxValue) maxValue = leftVal;
  if(rightVal < minValue) minValue = rightVal;
  if(rightVal > maxValue) maxValue = rightVal;
  
  //int leftValue = curveSize*(((float)(leftVal-minValue)/(float)(maxValue-minValue)));
  //int rightValue = 5*(((float)(rightVal-minValue)/(float)(maxValue-minValue)));
  beat = 10*(((float)(leftVal-minValue)/(float)(maxValue-minValue)));
  //animation_speed = base_animation_speed*beatMultiplier;

  for(int i=0; i<16; i++) {
    if(whitePalette[i].r > 20)
      whitePalette[i] = CRGB(whitePalette[i].r-20, whitePalette[i].g-20, whitePalette[i].b-20);
  }
  whitePalette[0] = CRGB::White;
  whitePalette[7] = CRGB::White;
  //whitePalette[8] = CRGB::Grey;

  if(beat>2) {
    whitePalette[1] = CRGB::White;
    whitePalette[2] = CRGB::White;
  }
  if(beat>4) {
    whitePalette[3] = CRGB::White;
    whitePalette[4] = CRGB::White;
  }
  if(beat>6) {
    whitePalette[11] = CRGB::White;
    whitePalette[12] = CRGB::White;
  }
  if(beat>8)   {
    whitePalette[13] = CRGB::White;
    whitePalette[14] = CRGB::White;
  }
  Serial.println(beat);
}


// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  
  uint8_t dataSmoothing = 0;
  if( animation_speed < 50) {
    dataSmoothing = 200 - (animation_speed * 4);
  }
  
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = animation_scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = animation_scale * j;
      
      uint8_t data = inoise8(x + ioffset,y + joffset,z);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
      
      noise[i][j] = data;
    }
  }
  
  z += animation_speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += animation_speed / 8;
  y -= animation_speed / 16;
}

void mapNoiseToLEDsUsingPalette()
{
  static uint8_t ihue=0;
  
  for(int i = 0; i < MATRIX_WIDTH; i++) {
    for(int j = 0; j < MATRIX_HEIGHT; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];

      // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds(i,j) = color;
    }
  }
  
  ihue+=1;
}

void waves(int speed, int scale, CRGBPalette16 palette) {
  currentPalette = palette;
  animation_speed = speed;
  base_animation_speed = speed;
  animation_scale = scale;
  colorLoop = 1;
}

void scrollText() {
  
}

// Palettes
//  PartyColors_p
//  RainbowColors_p
//  ForestColors_p
//  OceanColors_p
//  LavaColors_p

int8_t currentY = 0;

int8_t count = 0;
int8_t tailX[5];
int8_t tailY[5];
CRGB tailC[5];

void loop() {
  readMSGEQ7();

  // Changing the first value changes speed 1 = fast, second value is 'thickness' of wave
  
  waves(1, 20, whitePalette);
  fillnoise8();
  mapNoiseToLEDsUsingPalette();
//  FastLED.show();
  //FastLED.clear();
  
  Sprites.UpdateSprites();
  Sprites.RenderSprites();
  FastLED.show();

/*  
    if (ScrollingMsg.UpdateText() == -1) {
    ScrollingMsg.SetText((unsigned char*)msg, sizeof(msg)-1);
    ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  } else
    FastLED.show();
    */
  //delay(40);

/*
  // oval
  int x = (float)((float)MATRIX_WIDTH/2.0) + (float)((float)MATRIX_WIDTH/2.0)*cos16((float)count/256.0);
  int y = (float)((float)MATRIX_HEIGHT/2.0) + (float)((float)MATRIX_HEIGHT/2.0)*sin16((float)count/256.0);
  count++;
  if(count == 256) {
    count = 0;
  }
  
  tailC[0].fadeToBlackBy(64);
  for(int i=4; i>=0; i--) {
    tailC[i] = tailC[i-1];
    tailX[i] = tailX[i-1];
    tailY[i] = tailY[i-1];
    leds(tailX[i], tailY[i]) = tailC[i];
  }
  tailX[0] = x;
  tailY[0] = y;
  tailC[0] = CRGB::White;
  leds(x,y) = CRGB::White;
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" y = ");
  Serial.println(y);
  */
  //FastLED.show();
}

