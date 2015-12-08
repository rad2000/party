#include<FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontRobotron.h>
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

float beatMultiplier = 1.0;

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

cLEDText ScrollingMsg;
const unsigned char msg[] = { "THIS IS A MESSAGE" };

#define GHOST_WIDTH 14
#define GHOST_HEIGHT 13
const uint8_t GhostData[] = {
  B8_2BIT(00000111),B8_2BIT(10000000),
  B8_2BIT(00011111),B8_2BIT(11100000),
  B8_2BIT(00111111),B8_2BIT(11110000),
  B8_2BIT(01112211),B8_2BIT(11221000),
  B8_2BIT(01122221),B8_2BIT(12222000),
  B8_2BIT(01122331),B8_2BIT(12233000),
  B8_2BIT(11122331),B8_2BIT(12233100),
  B8_2BIT(11112211),B8_2BIT(11221100),
  B8_2BIT(11111111),B8_2BIT(11111100),
  B8_2BIT(11111111),B8_2BIT(11111100),
  B8_2BIT(11111111),B8_2BIT(11111100),
  B8_2BIT(11011100),B8_2BIT(11101100),
  B8_2BIT(10001100),B8_2BIT(11000100)
};
const uint8_t GhostMask[] = {
  B8_1BIT(00000111),B8_1BIT(10000000),
  B8_1BIT(00011111),B8_1BIT(11100000),
  B8_1BIT(00111111),B8_1BIT(11110000),
  B8_1BIT(01111111),B8_1BIT(11111000),
  B8_1BIT(01111111),B8_1BIT(11111000),
  B8_1BIT(01111111),B8_1BIT(11111000),
  B8_1BIT(11111111),B8_1BIT(11111100),
  B8_1BIT(11111111),B8_1BIT(11111100),
  B8_1BIT(11111111),B8_1BIT(11111100),
  B8_1BIT(11111111),B8_1BIT(11111100),
  B8_1BIT(11111111),B8_1BIT(11111100),
  B8_1BIT(11011100),B8_1BIT(11101100),
  B8_1BIT(10001100),B8_1BIT(11000100)
};
struct CRGB GhostColTable[3] = { CRGB::Orange, CRGB::White, CRGB::Blue };
cLEDSprites Sprites(&leds);
cSprite Ghost(GHOST_WIDTH, GHOST_HEIGHT, GhostData, 1, _2BIT, GhostColTable, GhostMask);

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

  /*
  ScrollingMsg.SetFont(RobotronFontData);
  ScrollingMsg.Init(&leds, MATRIX_WIDTH, MATRIX_HEIGHT, 0, 0);
  ScrollingMsg.SetText((unsigned char*)msg, sizeof(msg)-1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg.SetScrollDirection(SCROLL_LEFT);
  ScrollingMsg.SetTextDirection(CHAR_UP);
  ScrollingMsg.SetFrameRate(40);
  */
  
  Ghost.SetPositionFrameMotionOptions(-3/*X*/, -3/*Y*/, 0/*Frame*/, 0/*FrameRate*/, 1/*XChange*/, 10/*XRate*/, 0/*YChange*/, 5/*YRate*/, SPRITE_X_KEEPIN | SPRITE_DETECT_EDGE);
  Sprites.AddSprite(&Ghost);
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
  beatMultiplier = 1.0 - (((float)(leftVal-minValue)/(float)(maxValue-minValue)));
  animation_speed = base_animation_speed*beatMultiplier;
  //Serial.println(animation_speed);
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
  
  waves(10, 20, LavaColors_p);
  fillnoise8();
  mapNoiseToLEDsUsingPalette();
  
  //FastLED.clear();
  
  /*
  Sprites.UpdateSprites();
  Sprites.RenderSprites();
  */

/*
  if (ScrollingMsg.UpdateText() == -1) {
    ScrollingMsg.SetText((unsigned char*)msg, sizeof(msg)-1);
    ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  } else
    FastLED.show();
  delay(40);
*/

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
  FastLED.show();
}

