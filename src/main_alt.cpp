/*   
*     Yo Wallah Geiler Hut
*/

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "index-html.h"
#include "buchstabenyolo.h"

// Global Variables ****************************************

WiFiUDP g_Udp;
ESP8266WebServer server(80);

int g_PacketCount = 0;              // Number of UDP packets received
unsigned long g_PreviousTime = 0;   // Time since the last UDP packet arrived (to detect disconnects)

static enum{
  E_BOOT = 0,
  E_BOB_POLY,
  E_BOB_MONO,
  E_PRESET
} g_State=E_BOOT;                    // Current state, not to be confused with mode

#define NUM_LEDS 270                // Physical amount of LEDs
CRGB g_aLeds[NUM_LEDS];             // Array of CRGBs to be handled by FastLED
//CRGB g_aColor[5];                   
//CRGB g_UdpColor = CRGB::Blue;
#define BOBWAVEBUFFER 64            // how many previous bob colors to store
CRGB g_aBobWave[BOBWAVEBUFFER];     // previous bob colors for waves

#define DEFAULT_COLOR_1 CRGB::Green
#define DEFAULT_COLOR_2 CRGB::Purple

// lookup tables
const int g_aRes[8] = {                    // Number of LEDs in each row
  35, 35, 34, 34, 34, 33, 33, 32
};
const int g_aOff[8] = {
  0, 35, 70, 104, 138, 172, 205, 238      // indices of first LED in row
};

unsigned long g_Tick=NUM_LEDS;              // program counter,
                                          // starts at NUM_LEDS to avoid integer underflow

static int g_OverallBrightness = 8;       // x/255, keep between 8 and 64 for power usage

void(* resetFunc) (void) = 0;             // reboots the entire processor (have you tried turning...)

// LED Kram ******************************************************

CRGB g_ErrorLed;                          // dummy to point to instead of NULL (avoids segfault)
static CRGB* coords(int x, int y)         // returns pointer to the CRGB at coordinates
{
  if(0<=x&&x<g_aRes[y])
    return &g_aLeds[g_aOff[y]+x];
  else
    return &g_ErrorLed;              // point to dummy CRGB which can get overwritten safely
}

class mode                  // base class so pCurrentMode->render works
{
  public:
  static char           s_aWebpage[];
  static CRGB           s_aColor[];
  
  virtual void          init() = 0;
  virtual void          activate() = 0;
  virtual void          render() = 0;
  virtual int           webHandler(int) = 0;
  virtual void          genWebpage() = 0;
};

char  mode::s_aWebpage[3072];
CRGB  mode::s_aColor[5];

static mode* g_pCurrentMode;           // global pointer to currently active mode


static class : public mode // RAINBOW ****************
{
  int     m_Speed;
  int     m_Slant;
  int     m_InvertOffset;
  int     m_SpeedOffset;
  public:
  void init()
  {
    m_Speed = 16;
    m_Slant = 0;
    m_InvertOffset=0;
    m_SpeedOffset=0;
    Serial.print("Rainbow, ");
  }
  void activate()
  {
    genWebpage();
    // reset offsets (why not)
    m_InvertOffset=0;
    m_SpeedOffset=0;
  }
  void setSpeed(int s)
  {
    m_SpeedOffset = ((m_Speed-s)*g_Tick - m_SpeedOffset)%255; // doesnt work, check math
    if(m_Speed<0)
      m_Speed = -s;
    else
      m_Speed = s;
  }
  void invertDirection()
  {
    m_InvertOffset = (2*(g_Tick*m_Speed)/16 + m_InvertOffset)%255; // so the current offset plus this one = 0
    m_Speed = -m_Speed;
  }
  void setSlant(int s)
  {
    m_Slant = s;
  }
  void render()
  {
    for(int y = 0; y<8; y++){
      for(int x = 0; x<g_aRes[y]; x++){
        coords(x, y)->setHue(( 
            (15*x)/2              // TODO: manual scale
          + (g_Tick*m_Speed)/16 
          + m_InvertOffset
          + m_SpeedOffset
          - y*m_Slant
          )%255);      
      }
    }
  }
  int webHandler(int i)
  {
    if(!strcmp(server.argName(i).c_str(),"rainbow_speed")){
      setSpeed(atoi(server.arg(i).c_str()));
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"rainbow_slant")){
      setSlant(atoi(server.arg(i).c_str()));
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"rainbow_invertDirection")){
      Serial.printf("Changing Rainbow Direction");
      invertDirection();
      return 1;
    }else
      return 0;
  }
  void genWebpage()
  {
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hRainbow, sizeof(g_hRainbow));
    index += sizeof(g_hRainbow)-1;  
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Rainbow;

static class : public mode // CIRCLE ****************
{
  int     m_Speed[5];
  int     m_Width[5];
  public:
  void init()
  {
    m_Speed[0] = 18;
    m_Speed[1] = -8;
    m_Speed[2] = 23;
    m_Speed[3] = -15;
    m_Speed[4] = 10;    
    m_Width[0] = 4;
    m_Width[1] = 5;
    m_Width[2] = 2;
    m_Width[3] = 2;
    m_Width[4] = 1;
    Serial.print("Circle, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    for(int n=0;n<5;n++){
      for(int y=0;y<8;y++){
        for(int w=0;w<m_Width[n];w++){
          CRGB tmp = s_aColor[n];
          tmp%=((m_Width[n]-w)*255/m_Width[n]);             //linear fade (kein unterschied, basst scho)
          //tmp%=(255-((255*w/m_Width[n])*(255*w/m_Width[n])/255)); // quadratic fade
          *coords((((g_Tick*m_Speed[n])/128)+w)%35 ,y) += tmp;    //TODO: slant (see rainbow)
          *coords((((g_Tick*m_Speed[n])/128)-w)%35 ,y) += tmp;
        }
      }
    }
    // TODO: maybe multiple phases, instead of only one stripe circling
  }
  void setSpeed(int n, int s)
  {
    m_Speed[n] = s;
  }
  void setWidth(int n, int w)
  {
    m_Width[n] = w;
  }
  int webHandler(int i)
  {
    static int num=0;
    if(!strcmp(server.argName(i).c_str(),"circle_number")){
      num=atoi(server.arg(i).c_str())-1;                // parse num, -1 to index at 0
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"circle_color")){
      s_aColor[num] = strtol(server.arg(i).c_str()+1, NULL,16);      //pointer to string incremented to avoid '#'
      Serial.printf("new color #%d = %d\n", num, strtol(server.arg(i).c_str(),NULL,16));
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"circle_speed")){
      setSpeed(num, atoi(server.arg(i).c_str()));
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"circle_width")){
      setWidth(num, atoi(server.arg(i).c_str()));
      return 1;
    }else
      return 0;
  }
  void genWebpage()
  {
    //set Webpage
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hCircle, sizeof(g_hCircle));
    index += sizeof(g_hCircle)-1;
    memcpy(s_aWebpage+index, g_hColorPickerMulti, sizeof(g_hColorPickerMulti));
    for(int i=0;i<5;i++){
      sprintf(s_aWebpage+index+g_aColorPickerOffset[i], "%02x%02x%02x", s_aColor[i].r, s_aColor[i].g, s_aColor[i].b);
      s_aWebpage[index+g_aColorPickerOffset[i]+6] = '\"'; //replace character overwritten by trailing 0
    }
    index += sizeof(g_hColorPickerMulti)-1;    
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Circle;

#if 0
static class : public mode // BREATHE ****************
{
  int     m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Breathe, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    for(int i=0;i<NUM_LEDS;i++)
      g_aLeds[i] = (g_Tick%4) ? CRGB::Black : CRGB::White; 
    
  }
  int webHandler(int i)
  {
    return 0;
  }
  void genWebpage()
  {
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hRainbow, sizeof(g_hRainbow));
    index += sizeof(g_hRainbow)-1;  
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Breathe;
#endif

#if 0
static class : public mode // STROBE ****************
{
  int     m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Strobe, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    for(int i=0;i<NUM_LEDS;i++)
      g_aLeds[i] = (g_Tick%4) ? CRGB::Black : CRGB::White; 
    
  }
  int webHandler(int i)
  {
    return 0;
  }
  void genWebpage()
  {
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hRainbow, sizeof(g_hRainbow));
    index += sizeof(g_hRainbow)-1;  
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Strobe;
#endif

#if 0
static class : public mode // FIRE ****************
{
  int     m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Fire, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    //do stuff
  }
  int webHandler(int i)
  {
      return 0;
  }
  void genWebpage()
  {
  }
} g_Fire;
#endif

#if 0
static class : public mode // WAVE ****************
{
  int     m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Wave, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    //do stuff
  }
  int webHandler(int i)
  {
    return 0;
  }
  void genWebpage()
  {
  }
} g_Wave;
#endif

static class : public mode // ZEPELLI ****************
{
  int             m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Zepelli, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    for(int i=0;i<NUM_LEDS;i++)
      g_aLeds[i] = CRGB::Black;

    for(int x=0;x<35;x++){
      int currentByteIndex = (x+(m_Speed*g_Tick/64)) % sizeof(g_aZepelli);
      for(int y=0;y<8;y++){
        if(g_aZepelli[currentByteIndex] & 1<<y)
          *coords(x,y) = CRGB::White;
      }
    }
  }
  int webHandler(int i)
  {
    if(!strcmp(server.argName(i).c_str(),"zepelli_speed")){
      m_Speed = atoi(server.arg(i).c_str());
      return 1;
    }else
      return 0;
  }
  void genWebpage()
  {
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hZepelli, sizeof(g_hZepelli));
    index += sizeof(g_hZepelli)-1;  
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Zepelli;

static class : public mode // NOISE ****************
{
  int     m_Speed;
  int     m_Scale;
  int     m_Threshold;
  enum {
    E_NOISE_HUE,
    E_NOISE_SINGLE,
    E_NOISE_MULTI
  }       m_Type;
  public:
  void init()
  {
    // do stuff
    m_Speed = 4;
    m_Scale = 8;
    m_Threshold = 85;       // 255/3 = 85
    m_Type = E_NOISE_HUE;
    Serial.print("Noise, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    int colors=0;
    while(colors<5 && s_aColor[colors])
      colors++;                                             // count colors
    if(m_Type == E_NOISE_MULTI && colors<2){                // if you fuck up the colors, get default ones
      s_aColor[0] = DEFAULT_COLOR_1;
      s_aColor[1] = DEFAULT_COLOR_2;
      Serial.println("Too little colors, falling back");
    }
                                                            // TODO: add "convection" (just a linear factor to y-Speed)
    for(int y=0;y<8;y++){
      for(int x=0;x<35; x++){
        int noise = inoise8(x*m_Scale*4, y*m_Scale*4, (g_Tick*m_Speed)/2);
        switch(m_Type)
        {
          case E_NOISE_HUE:
            coords(x,y)->setHue(noise);
            break;
          case E_NOISE_SINGLE:
            noise = (noise<m_Threshold)? 0 : 255/(255-m_Threshold) * (noise-m_Threshold);
            *coords(x,y) = s_aColor[0];
            coords(x,y)->nscale8(noise);
            break;
          case E_NOISE_MULTI:
            switch(colors)
            {
              case 2:
                #define NOISE_M2_T1 128
                if      (noise<NOISE_M2_T1){              //first half
                  noise = -2*(noise-NOISE_M2_T1);
                  *coords(x,y) = s_aColor[0];
                  coords(x,y)->nscale8(noise);
                }else if(NOISE_M2_T1<noise){              //second half
                  noise = 2*(noise-NOISE_M2_T1);
                  *coords(x,y) = s_aColor[1];
                  coords(x,y)->nscale8(noise);
                }else
                  *coords(x,y) = CRGB::Black;
                break;
              case 3:
                #define NOISE_M3_T1 100
                #define NOISE_M3_T2 155
                if      (noise<NOISE_M3_T1){             //first third
                  noise = -3*(noise-NOISE_M3_T1);
                  *coords(x,y) = s_aColor[0];
                  coords(x,y)->nscale8(noise);
                }else if(NOISE_M3_T1<noise&&noise<NOISE_M3_T2){  //second third
                  noise = (noise<(NOISE_M3_T1+NOISE_M3_T2)/2)?   //divide in two parts and linearize
                          6*(noise-NOISE_M3_T1):
                          -6*(noise-NOISE_M3_T2);
                  *coords(x,y) = s_aColor[1];
                  coords(x,y)->nscale8(noise);
                }else if(NOISE_M3_T2<noise){            //last third
                  noise = 3*(noise-NOISE_M3_T2);
                  *coords(x,y) = s_aColor[2];
                  coords(x,y)->nscale8(noise);
                }else
                  *coords(x,y) = CRGB::Black;
                break;
              case 4:
                #define NOISE_M4_T1 84
                #define NOISE_M4_T2 128
                #define NOISE_M4_T3 172
                if      (noise<NOISE_M4_T1){             //first quarter
                  noise = -4*(noise-NOISE_M4_T1);
                  *coords(x,y) = s_aColor[0];
                  coords(x,y)->nscale8(noise);
                }else if(NOISE_M4_T1<noise&&noise<NOISE_M4_T2){  //second quarter
                  noise = (noise<(NOISE_M4_T1+NOISE_M4_T2)/2)? 
                          8*(noise-NOISE_M4_T1):
                          -8*(noise-NOISE_M4_T2);
                  *coords(x,y) = s_aColor[1];
                  coords(x,y)->nscale8(noise);
                }else if(NOISE_M4_T2<noise&&noise<NOISE_M4_T3){  //third quarter
                  noise = (noise<(NOISE_M4_T2+NOISE_M4_T3)/2)?
                          8*(noise-NOISE_M4_T2):
                          -8*(noise-NOISE_M4_T3);
                  *coords(x,y) = s_aColor[2];
                  coords(x,y)->nscale8(noise);
                }else if(NOISE_M4_T3<noise){            //last quarter
                  noise = 4*(noise-NOISE_M4_T3);
                  *coords(x,y) = s_aColor[3];
                  coords(x,y)->nscale8(noise);
                }else
                  *coords(x,y) = CRGB::Black;
                break;
              case 5:
                // no.
                break;
            }
            break;
        }
      }
    }
  }
  int webHandler(int i)
  {
    static int num=0;    
    if(!strcmp(server.argName(i).c_str(),"noise_number")){
      num=atoi(server.arg(i).c_str())-1;                // parse num, -1 to index at 0
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"noise_color_multi")){
      s_aColor[num] = strtol(server.arg(i).c_str()+1, NULL,16);      //pointer to string incremented to avoid '#'
      Serial.printf("new color #%d = %d\n", num, strtol(server.arg(i).c_str(),NULL,16));
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"noise_color_single")){
      s_aColor[0] = strtol(server.arg(i).c_str()+1, NULL,16);      //pointer to string incremented to avoid '#'
      Serial.printf("new color #%d = %d\n", num, strtol(server.arg(i).c_str(),NULL,16));
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"noise_threshold")){   // single noise parameters
      m_Threshold = atoi(server.arg(i).c_str());
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"noise_speed")){         // general noise parameters
      m_Speed = atoi(server.arg(i).c_str());
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"noise_scale")){
      m_Scale=atoi(server.arg(i).c_str());
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"noise_switchType")){
      // redirect pointer
      if     (!strcmp(server.arg(i).c_str(),"hue"))     m_Type=E_NOISE_HUE;
      else if(!strcmp(server.arg(i).c_str(),"single"))  m_Type=E_NOISE_SINGLE;
      else if(!strcmp(server.arg(i).c_str(),"multi"))   m_Type=E_NOISE_MULTI;
      
      genWebpage();
      // redirect browser to homepage
      server.sendHeader("Location","/");
      server.send(303);
      return 1;
    }
    return 0;
  }
  void genWebpage()
  {
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hNoiseHead, sizeof(g_hNoiseHead));
    index += sizeof(g_hNoiseHead)-1;  
    switch(m_Type)
    {
      case E_NOISE_HUE:
        // Hue Noise has no own Parameters (yet!)
        // Idea: not just hue(0-255) but custom color scales
        break;
      case E_NOISE_SINGLE:
        memcpy(s_aWebpage+index, g_hColorPickerSingle, sizeof(g_hColorPickerSingle));
        sprintf(s_aWebpage+index+g_aColorPickerOffset[0], "%02x%02x%02x", s_aColor[0].r, s_aColor[0].g, s_aColor[0].b);
        s_aWebpage[index+g_aColorPickerOffset[0]+6] = '\"'; //replace character overwritten by trailing 0
        index += sizeof(g_hColorPickerSingle)-1;
        memcpy(s_aWebpage+index, g_hNoiseSingle, sizeof(g_hNoiseSingle));
        index += sizeof(g_hNoiseSingle)-1;  
        break;
      case E_NOISE_MULTI:
        memcpy(s_aWebpage+index, g_hColorPickerMulti, sizeof(g_hColorPickerMulti));
        for(int i=0;i<5;i++){
          sprintf(s_aWebpage+index+g_aColorPickerOffset[i], "%02x%02x%02x", s_aColor[i].r, s_aColor[i].g, s_aColor[i].b);
          s_aWebpage[index+g_aColorPickerOffset[i]+6] = '\"'; //replace character overwritten by trailing 0
        }
        index += sizeof(g_hColorPickerMulti)-1;  
        break;
    }
    memcpy(s_aWebpage+index, g_hNoiseTail, sizeof(g_hNoiseTail));
    index += sizeof(g_hNoiseTail)-1;  
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Noise;

//include buchstabenyolo.h!

#define TEXT_MAX_LENGTH 128

static class : public mode // TEXT ****************
{
  int     m_Speed;
  unsigned char m_aBytes[TEXT_MAX_LENGTH * FONT_WIDTH];
  int     m_TextLength;
  int     m_ByteLength;
  int     m_WrapOffset;
  int     m_NumColors;
  int strToBytes(const char *pText)
  {
    m_TextLength = strlen(pText);
    for(int c=0;c<m_TextLength;c++){ // basically strcpy but sanitized to our stuff
      int chr = pText[c];
      if     (chr == 0xE4) chr = FONT_ae; // We're Using ISO-8859-15 in the webpage
      else if(chr == 0xF6) chr = FONT_oe;
      else if(chr == 0xFC) chr = FONT_ue;
      else if(chr == 0xC4) chr = FONT_AE;
      else if(chr == 0xD6) chr = FONT_OE;
      else if(chr == 0xDC) chr = FONT_UE;
      else if(chr == 0xDF) chr = FONT_ssharp;
      else if(chr == 0xA4) chr = FONT_euro;
      else if(chr<FONT_FIRST_CHAR||chr>FONT_LAST_CHAR)
        return chr;
      chr -= FONT_FIRST_CHAR;

      for(int i=0;i<FONT_WIDTH;i++){
        m_aBytes[c*FONT_WIDTH +i] = g_aFont[chr*FONT_WIDTH +i];
      }
    }
    m_ByteLength = m_TextLength*FONT_WIDTH + m_WrapOffset;
    for(int i=m_TextLength*FONT_WIDTH; i<m_ByteLength; i++)
      m_aBytes[i] = 0;
    return 0;
  }
public:
  void init()
  {
    // do stuff
    m_Speed=16;
    m_WrapOffset=10;
    m_NumColors = 2;
    strToBytes("Hello World!");
    Serial.print("Text, ");
  }
  void activate()
  {
    genWebpage();
  }
  void render()
  {
    for(int i=0;i<NUM_LEDS;i++)
      g_aLeds[i] = CRGB::Black;

    for(int x=0;x<35;x++){
      int currentByteIndex = (x+(m_Speed*g_Tick/64)) % m_ByteLength;
      for(int y=0;y<8;y++){
        if(m_aBytes[currentByteIndex] & 1<<(FONT_HEIGHT-y-1))
          *coords(x,y) = s_aColor[(currentByteIndex/FONT_WIDTH)%m_NumColors];
      }
    }
    
  }
  int webHandler(int i)
  {
    if(!strcmp(server.argName(i).c_str(),"text_speed")){
      m_Speed = atoi(server.arg(i).c_str());
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"text_text")){
      char c = strToBytes(server.arg(i).c_str());
      if(c)
        Serial.printf("Invalid Character: %c (0x%X)", c, c);
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"text_wrapOffset")){
      m_WrapOffset = atoi(server.arg(i).c_str());
      return 1;
    }else if(!strcmp(server.argName(i).c_str(),"text_color")){
      s_aColor[0] = strtol(server.arg(i).c_str()+1, NULL,16);  
      return 1;
    }else
      return 0;
  }
  void genWebpage()
  {
    int index = sizeof(g_hHead)-1;
    memcpy(s_aWebpage+index, g_hText, sizeof(g_hText)); //todo add colorpickers
    index += sizeof(g_hText)-1;  
    memcpy(s_aWebpage+index, g_hTail, sizeof(g_hTail));
  }
} g_Text;

#if 0
class : public mode // BOB POLY ****************
{
  int     m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Bob Poly, ");
  }
  void activate()
  {
    strcpy(s_aWebpage+sizeof(g_hHead)-1, g_hRainbow);
    strcat(s_aWebpage+sizeof(g_hHead), g_hTail);
  }
  void render()
  {
    //do stuff
  }
  int webHandler(int i)
  {
    return 0;
  }
} g_BobPoly;

static class : public mode // BOB MONO ****************
{
  int     m_Speed;
  public:
  void init()
  {
    // do stuff
    Serial.print("Bob Mono, ");
  }
  void activate()
  {
    strcpy(s_aWebpage+sizeof(g_hHead)-1, g_hRainbow);
    strcat(s_aWebpage+sizeof(g_hHead), g_hTail);
  }
  void render()
  {
    //do stuff
  }
  int webHandler(int i)
  {
    return 0;
  }
} g_BobMono;
#endif

// Internet/UDP Kram *******************************************
      //TODO UDP und Wlan/Hotspot trennen?

#include "wificredentials.h"
//#define STASSID "wifiname"
//#define STAPSK  "wifipassword"

#define NETWORK_IP 173
#define DATA_PIN 4

static unsigned int localPort = 26091;                                 // local port to listen on
IPAddress staticIP(10,96,0,NETWORK_IP), gateway (10,96,0,254);  // awabo
IPAddress subnet(255,255,255,0);
IPAddress dns(gateway);

// buffers for receiving and sending data
char g_aPacketBuffer[UDP_TX_PACKET_MAX_SIZE + 1];     // buffer to hold incoming packet,
char  g_aReplyBuffer[UDP_TX_PACKET_MAX_SIZE + 1];     // a string to send back


static void sendPacket(char*string)
{
  // send a reply, to the IP address and port that sent us the packet we received
  g_Udp.beginPacket(g_Udp.remoteIP(), g_Udp.remotePort());
  g_Udp.write(string);
  g_Udp.endPacket();
}

static int getPacket()
{  
  // if there's data available, read a packet
  int packetSize = g_Udp.parsePacket();
  if (packetSize) {
    #ifdef PRINTPACKETS2
      Serial.printf("Received packet #%d of size %d from %s:%d\n  Arrived after %d seconds, first one since %d milliseconds\n  (to %s:%d, free heap = %d B)\n",
                  g_PacketCount, packetSize,
                  millis()/60, millis()-g_PreviousTime,
                  g_Udp.remoteIP().toString().c_str(), g_Udp.remotePort(),
                  g_Udp.destinationIP().toString().c_str(), g_Udp.localPort(),
                  ESP.getFreeHeap());
    #endif

    // read the packet into packetBuffer
    int n = g_Udp.read(g_aPacketBuffer, UDP_TX_PACKET_MAX_SIZE);
    g_aPacketBuffer[n] = 0;
    
    #ifdef PRINTPACKETS
      Serial.printf("Contents: %d, %d, %d, %d\n", g_aPacketBuffer[0], g_aPacketBuffer[1], g_aPacketBuffer[2], g_aPacketBuffer[3]);
    #endif
    
    g_PacketCount++;
    g_PreviousTime=millis(); //update time of last packet
  }
  return packetSize;
}

static void parsePacket()
{
  switch(g_aPacketBuffer[0]){       // OP-Code
    case 0x00:                      //reboot
      resetFunc();
    break;

    case 0x12:                      //Change Mode
      switch(g_aPacketBuffer[1]){
        //case 0:   g_State = E_RAINBOW;   break;
        //case 1:   g_State = E_CIRCLE;    break;
        //case 2:   g_State = E_LAVA;      break;
        default:  g_State = E_BOOT;   break; // default
      }      
    break;

    case 4:                         // bob color deprecated
    case 0x10:                      // bob color
      g_State = E_BOB_POLY;
      //g_UdpColor.setRGB(g_aPacketBuffer[1], g_aPacketBuffer[2], g_aPacketBuffer[3]); 
    break;
    case 0x11:                      // bob mono
      g_State = E_BOB_MONO;
      // todo brightness variable?
    break;
  }
}

// Webserver Kram ******************************************


// callback for /, has to deliver the html of the homepage
static void handleRoot()
{ 
  Serial.println("url: /");
  server.send(200, "text/html", g_pCurrentMode->s_aWebpage);
}


// callback for /cmd, has to parse arguments
static void handleCmd()
{
  Serial.printf("cmd, n_args %d\n",server.args());

  for(int i=0;i<server.args();i++)
  {
    Serial.printf("%s=%s\n",server.argName(i).c_str(),server.arg(i).c_str());

    if(g_pCurrentMode->webHandler(i));                // individual mode webhandler
    else if(!strcmp(server.argName(i).c_str(),"overallBrightness")){
      g_OverallBrightness=atoi(server.arg(i).c_str());
      FastLED.setBrightness(g_OverallBrightness);
    }else if(!strncmp(server.argName(i).c_str(),"color_", 6)){
      mode::s_aColor[*(server.argName(i).c_str()+6)-'1'] = strtol(server.arg(i).c_str()+1, NULL,16);
      g_pCurrentMode->genWebpage();
    }else if(!strcmp(server.argName(i).c_str(),"switchMode")){
      // redirect pointer
      if     (!strcmp(server.arg(i).c_str(),"rainbow"))   g_pCurrentMode=&g_Rainbow;
      else if(!strcmp(server.arg(i).c_str(),"circle"))    g_pCurrentMode=&g_Circle;
      //else if(!strcmp(server.arg(i).c_str(),"breathe"))   g_pCurrentMode=&g_Breathe;
      //else if(!strcmp(server.arg(i).c_str(),"fire"))      g_pCurrentMode=&g_Fire;
      //else if(!strcmp(server.arg(i).c_str(),"wave"))      g_pCurrentMode=&g_Wave;
      else if(!strcmp(server.arg(i).c_str(),"zepelli"))     g_pCurrentMode=&g_Zepelli;
      else if(!strcmp(server.arg(i).c_str(),"noise"))     g_pCurrentMode=&g_Noise;
      else if(!strcmp(server.arg(i).c_str(),"text"))      g_pCurrentMode=&g_Text;
      // wake up the new mode
      g_pCurrentMode->activate();
      
      // redirect browser to homepage
      server.sendHeader("Location","/");
      server.send(303);
    }
    
  }

  // redirect browser to homepage
  //server.sendHeader("Location","/");
  //server.send(303);

  // better: send "No Content". This holds the browser on the home page and keeps the
  // selections
  server.send(204);
}

// callback for all other URLs
static void handleNotFound()
{
 Serial.printf("url: %s -> not found\n",server.uri().c_str());

  // redirect to home page
  server.sendHeader("Location","/");
  server.send(302);
  //server.send(404, "text/plain", "404: Not found");
}


// Setup Kram ********************************************

static void initWifi()
{
  // todo: parameter to switch between AP and client?
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet, dns);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {       // wait for connection
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("UDP server on port %d\n", localPort);
  g_Udp.begin(localPort);
}

static void initHTTP()
{
  // HTTP server
  server.on("/",    handleRoot); 
  server.on("/cmd", handleCmd);   
  server.onNotFound(handleNotFound);
  server.begin(); 

  Serial.println("HTTP server started");
  
  server.sendHeader("Location","/");
  server.send(303);
}

static void initHW()
{
  // initialize pins
  Serial.println("Initializing Pins");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(g_aLeds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
  FastLED.setBrightness(g_OverallBrightness);
  
  for(int j=0;j<NUM_LEDS; j++)
    g_aLeds[j] = CRGB::Black;           // initialize all LEDs to Off
  FastLED.show();
  Serial.println("Hardware initialized");
}

static void initModes()
{
  Serial.print("Initializing Modes: ");
  // global stuff
  memcpy(mode::s_aWebpage, g_hHead, sizeof(g_hHead));
  mode::s_aColor[0] = DEFAULT_COLOR_1;
  mode::s_aColor[1] = DEFAULT_COLOR_2;
  mode::s_aColor[2] = CRGB::Black;
  mode::s_aColor[3] = CRGB::Black;
  mode::s_aColor[4] = CRGB::Black;

  //initialize each mode
  g_Rainbow.init();
  g_Circle.init();
  //g_Breathe.init();
  //g_Strobe.init();
  //g_Fire.init();
  //g_Wave.init();
  g_Noise.init();
  g_Text.init();
  
  Serial.println("done");
}

static void initOTA()
{
  ArduinoOTA.setHostname("bernie-zylinder");
  ArduinoOTA.setPassword("swag");

  ArduinoOTA.onStart([]() {
      Serial.println("OTA: Start");
      //digitalWrite(BOARD_LED,1);
    });
  ArduinoOTA.onEnd([]() {
      //digitalWrite(BOARD_LED,1);
      Serial.println("\nOTA: End");
    });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      static bool state = false;
      //digitalWrite(BOARD_LED,state);
      state ^= true;
      Serial.printf("OTA: Progress: %u%%\r", (progress / (total / 100)));
    });
  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR)      Serial.println("Auth failed");
      else if (error == OTA_BEGIN_ERROR)    Serial.println("Begin failed");
      else if (error == OTA_CONNECT_ERROR)  Serial.println("Connect failed");
      else if (error == OTA_RECEIVE_ERROR)  Serial.println("Receive failed");
      else if (error == OTA_END_ERROR)    Serial.println("End failed");
    });
  ArduinoOTA.begin();
  Serial.println("OTA initialized");
}

// Programm Kram *******************************************

void setup() 
{
  // init Serial
  Serial.begin(115200);
  delay(10);
  Serial.println("======================\ncompiled at: " __DATE__ " " __TIME__);

  initWifi();

  initHW();

  initModes();
  
  g_pCurrentMode=&g_Rainbow;
  g_pCurrentMode->activate();

  initHTTP();  

#if 0 
  // clear the bobwave
  for(int j=0;j<20;j++)
    g_aBobWave[j]=CRGB::Black;
#endif

  initOTA();        // Over The Air update
}

void loop() 
{
  if(getPacket())                      // check for udp packet
    parsePacket();
  server.handleClient();            // http handling
  
  switch(g_State){                   // state machine
    case E_BOOT:
      // reset to black
      for(int j=0;j<NUM_LEDS; j++){
        g_aLeds[j] = CRGB::Black;
      }
      g_State = E_PRESET;
      Serial.println("Boot successful, going to default mode");
      break;
      
    case E_PRESET:
      g_pCurrentMode->render();   // handle mode
      break;

#if 0
    case E_BOB_POLY:
      g_BobPoly.render();
      
      if(millis()-g_PreviousTime > 10000 && g_PacketCount){         // timeout check
        Serial.printf("Haven't received Packet for ten seconds, rebooting into debug mode\n");
        delay(10);
        resetFunc();
      }
      break;
      
    case E_BOB_MONO:
      g_BobMono.render();
      
      if(millis()-g_PreviousTime > 10000 && g_PacketCount){         // timeout check
        Serial.printf("Haven't received Packet for ten seconds, rebooting into debug mode\n");
        delay(10);
        resetFunc();
      }
      break;
#endif
      
    default: 
      resetFunc();                  // something went wrong
      break;
  }

#if 0 //DEPRECATED! use FastLED.setBrightness instead
  for(int j=0;j<NUM_LEDS; j++){               // reduce overall brightness
    g_aLeds[j].nscale8(g_OverallBrightness);  // it is advised to keep this somewhere between 8 and 64
  }
#endif
 
  ArduinoOTA.handle();              // OTA
  FastLED.show();                   // Actually handle LEDs
  FastLED.delay(1);                 // Use the FastLED delay to enable dithering and stuff
  g_Tick++;

}
