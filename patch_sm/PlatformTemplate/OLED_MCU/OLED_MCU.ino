
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define OLED_MOSI     13
#define OLED_CLK      14
#define OLED_DC       4
#define OLED_CS       15
#define OLED_RST      27

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

enum States
{
  IDLE = 0,
  MAIN = 1,
  PAGE = 2,
  EDIT = 3,
  NUM_STATES = 4
};

enum Events
{
  NO_EVENT   = 0,  
  ENC_BTN    = 1,
  BACK_BTN   = 2,
  ENC_RIGHT  = 3,
  ENC_LEFT   = 4,
  TIMEOUT    = 5,
  NUM_EVENTS = 6
};

enum PageID
{
  ALGORITHM = 0,
  MIDI_SYNC = 1,
  SCREEN = 2,
  GENERAL = 3,
  NUM_PAGES = 4
};

const char* page_name_strings[NUM_PAGES] =
{
  " Algorithm Settings  ",
  " MIDI/Sync Settings  ",
  " Display Settings    ",
  " General Settings    ",
};

const char* page_headers[NUM_PAGES] =
{
  "~~~~~~Algorithm~~~~~~",
  "~~~~~~MIDI/Sync~~~~~~",
  "~~~~~~~Display~~~~~~~",
  "~~~~~~~General~~~~~~~",
};

enum ParameterID
{
  PARAM1 = 0,
  PARAM2 = 1,
  PARAM3 = 2,
  PARAM4 = 3,
  PARAM5 = 4,
  PARAM6 = 5,
  PARAM7 = 6,
  PARAM8 = 7,
  PARAM9 = 8,
  PARAM10 = 9,
  PARAM11 = 10,
  PARAM12 = 11,
  NUM_PARAMS = 12
};

enum ParamMask
{
  PARAM1_MASK = 1 << PARAM1,
  PARAM2_MASK = 1 << PARAM2,
  PARAM3_MASK = 1 << PARAM3,
  PARAM4_MASK = 1 << PARAM4,
  PARAM5_MASK = 1 << PARAM5,
  PARAM6_MASK = 1 << PARAM6,
  PARAM7_MASK = 1 << PARAM7,
  PARAM8_MASK = 1 << PARAM8,
  PARAM9_MASK = 1 << PARAM9,
  PARAM10_MASK = 1 << PARAM10,
  PARAM11_MASK = 1 << PARAM11,
  PARAM12_MASK = 1 << PARAM12
};

const char* parameter_name_strings[NUM_PARAMS] =
{
  " Parameter 1   ",
  " Parameter 2   ",
  " Parameter 3   ",
  " Parameter 4   ",
  " Parameter 5   ",
  " Parameter 6   ",
  " Parameter 7   ",
  " Parameter 8   ",
  " Parameter 9   ",
  " Parameter 10  ",
  " Parameter 11  ",
  " Parameter 12  "
};

const int parameter_min_max_default[NUM_PARAMS][3] =
{
  {0,1,11},  //1
  {0,2,1},  //2
  {0,3,10},  //3
  {0,4,100},  //4
  {0,100,55},  //5
  {0,6,66},  //6 
  {0,7,77},  //7
  {0,8,88},  //8
  {0,9,99},  //9
  {0,10,111}, //10
  {0,11,222}, //11
  {0,12,333}  //12
};

void display_fft (bool random_data)
{
  const int num_bins = 32;
  static int bin_data_l[num_bins] = {0};
  static int bin_data_r[num_bins] = {0};

  if (random_data)
  {
    for (int i = 0; i < num_bins ; i++)
    {
      bin_data_l[i] = random(0,30);
      bin_data_r[i] = random(0,30);
    }
  }

  display.clearDisplay();

  for (int i = 0; i < num_bins ; i++)
  {
    display.drawLine(4+3*i, 32, 4+3*i, 32 - bin_data_l[i], SH110X_WHITE);
    display.drawLine(4+3*i, 64, 4+3*i, 64 - bin_data_r[i], SH110X_WHITE);
  }

  display.drawLine(0, 32, 128, 32, SH110X_WHITE);
  display.display();
}

class Parameter
{
  public:
   void initialise(int id, int min_val, int max_val, int default_val)
   {
    m_id = id;
    m_min_val = min_val;
    m_max_val = max_val;    
    m_default_val = default_val;
    m_current_val = default_val;
   }

   void update_param(int enc_delta)
   {    
    int new_val = 0;
    new_val = m_current_val + enc_delta;

    if (new_val < m_min_val) 
      new_val = m_min_val;
    else if (new_val > m_max_val) 
      new_val = m_max_val;

    m_current_val = new_val;
   }

   int getID()
   {
    return m_id;
   }

   int getVal()
   {
    return m_current_val;
   }

  private:
    int m_current_val;
    int m_default_val;
    int m_max_val;
    int m_min_val;
    int m_id;
};

#define MIN_IDX 0
#define MAX_IDX 1
#define DEFAULT_IDX 2

class Page
{
  public:
   void initialise (PageID page_ID, int num_params, unsigned long long param_bitfield = 0)
   {
    m_page_ID = page_ID;
    m_num_params = num_params;
    m_selected_param = 0;

    int startID = 0;
    for(int param=0; param< m_num_params; param++)
    {
      for (int paramID = startID; paramID < NUM_PARAMS; paramID++)
      {
        if ((param_bitfield >> paramID) & 0x01)
        {
          parameters[param].initialise(paramID, 
                                      parameter_min_max_default[paramID][MIN_IDX], 
                                      parameter_min_max_default[paramID][MAX_IDX], 
                                      parameter_min_max_default[paramID][DEFAULT_IDX]);
          startID = paramID+1;
          break;
        }
      }
    }
   }

   void update (int enc_delta, bool edit_param)
   {
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.clearDisplay();
      display.setTextColor(SH110X_WHITE);
      display.print(page_headers[m_page_ID]);

      if(edit_param ){parameters[m_selected_param].update_param(enc_delta); }
      else { m_selected_param += enc_delta; }
      // sanitise input
      if(m_selected_param >= m_num_params) { m_selected_param = 0; }
      else if(m_selected_param < 0) { m_selected_param = m_num_params - 1; }
            Serial.println(m_selected_param);

      int start_param = 0;
      int end_param = 0;
      //logic to deal with multiple sections, new sections start at parameter 8 etc (index 7)
      if (m_selected_param > 6) { start_param = 7; }
      //logic to set the end paramter index in a section
      if( (start_param + 7) > m_num_params) { end_param = m_num_params; }
      else { end_param = start_param + 7; }
      // draw the params in each section to screen
      for (int i = start_param ; i < end_param; i++)   
      {
        if (i == m_selected_param) {display.setTextColor(SH110X_BLACK, SH110X_WHITE);}
        else                       {display.setTextColor(SH110X_WHITE);}
        display.print(parameter_name_strings[parameters[i].getID()]);

        if ((edit_param)&&(i == m_selected_param)) { display.print(">"); }
        else                           { display.print(" "); }

        int val = parameters[i].getVal();
        display.print(val);

        if ((edit_param)&&(i == m_selected_param)) { display.print("<"); }
        else                           { display.print(" "); }

        if(val<10)        { display.print("   "); }
        else if(val<100)  { display.print("  ");  }
        else if(val<1000) { display.print(" ");   }
      }
      display.display();
    }

    void reset ()
    {
      m_selected_param = 0;
    }

  private:
    int m_num_params;
    int m_selected_param;
    unsigned long long m_param_bitfield;
    PageID m_page_ID;
    static constexpr int page_max_params = 32;
    Parameter parameters[page_max_params];
};

class Menu
{
  public:         
    Menu()
    {
      m_selected_page = 0;
      pages[ALGORITHM].initialise(ALGORITHM, 10, (0 | PARAM1_MASK | PARAM2_MASK | PARAM3_MASK | PARAM4_MASK | PARAM5_MASK  | PARAM6_MASK  | PARAM7_MASK  | PARAM8_MASK | PARAM9_MASK  | PARAM10_MASK) );
      pages[MIDI_SYNC].initialise(MIDI_SYNC, 2, (0 | PARAM5_MASK | PARAM3_MASK) );
      pages[SCREEN].initialise(SCREEN, 0);
      pages[GENERAL].initialise(GENERAL, 0);
    }

    void update(int enc_delta)
    {
      m_selected_page += enc_delta;
      if(m_selected_page >= m_num_pages)
        m_selected_page = 0;
      else if(m_selected_page < 0)
        m_selected_page = m_num_pages-1;

      display.setTextSize(1);
      display.setCursor(0, 0);
      display.clearDisplay();
      for (int i = 0 ; i < m_num_pages; i++)   
      {
        if (i == m_selected_page){display.setTextColor(SH110X_BLACK, SH110X_WHITE);}
        else                  {display.setTextColor(SH110X_WHITE);}
        display.print(page_name_strings[i]);
      }
      display.display();
    }

    void updatePage(int enc_delta, bool edit_param)
    {
      pages[m_selected_page].update(enc_delta, edit_param);
    }

    void reset()
    {
      m_selected_page = 0;
      for (int page = 0; page < m_num_pages; page++)
      {
        pages[page].reset();
      }
    }

  private:
    static constexpr int m_num_pages = 4;
    Page pages[m_num_pages];
    int m_selected_page;
};

class DisplayState 
{
  public:
    void update(States state, int enc_delta = 0)
    {
      switch (state)
      {
        case IDLE:
          display_fft(1);
          break;
        case MAIN:
          m_menu.update(enc_delta);
          break;
        case PAGE:
          m_menu.updatePage(enc_delta,false);
          break;
        case EDIT:
          m_menu.updatePage(enc_delta,true);
          break;
        default:
          display.clearDisplay();
          display.display();
        break;
      }
    }

    void resetMenu()
    {
      m_menu.reset();
    }

  private:
    Menu m_menu;
};


// handles events, updates state, and initiates UI Updates
class StateMachine {    
  
  public:          
    States m_current_state;
    int m_current_row;
    
    void handler(Events event)
    {
      States next_state;
      int enc_delta = 0; 
      next_state = m_current_state;
      switch(event)
      {
        case ENC_BTN:
          if (m_current_state < EDIT) { transition( (States)( (int)next_state + 1 ) );}
          break;
        case BACK_BTN:
          if (m_current_state > IDLE) { transition( (States)( (int)next_state - 1 ) );}
          break;
        case ENC_RIGHT:
          enc_delta = 1;
          updateDisplay(enc_delta); 
          break;
        case ENC_LEFT:
          enc_delta = -1;
          updateDisplay(enc_delta); 
          break;
        case TIMEOUT:
          break;
        case NO_EVENT:
          if (m_current_state == IDLE)
          {
            updateDisplay(); 
          }
        default:
         break;
      }
    }

  private:
    DisplayState m_display_state;

    void transition(States next_state)
    {
      if(next_state < NUM_STATES)
      {
        m_current_state = next_state;
        if (m_current_state < PAGE)
        {
          m_display_state.resetMenu();
        }
      }
      updateDisplay();
    }

    void updateDisplay(int enc_delta=0)
    {
      m_display_state.update(m_current_state, enc_delta);
    }

};


//########################################################################################################################
//###############################         Arduino Specific Layer          ################################################
//########################################################################################################################

#define ENC_BTN_PIN 34
#define BACK_BTN_PIN 5 
#define ENC_PIN_A 25
#define ENC_PIN_B 26


volatile int encoderPosCount = 0;
int lastEncoded = 0;
unsigned long previousMillis = 0;        
unsigned long interval = 30UL; // (milliseconds)

StateMachine stateMachine;

void setup()   {
  Serial.begin(9600);
  pinMode(ENC_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(ENC_PIN_A, INPUT); 
  pinMode(ENC_PIN_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), updateEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_B), updateEncoder, RISING);

  //display.setContrast (0); // dim display
  display.begin(0, true); // we dont use the i2c address but we will reset!
  display.clearDisplay();
}

void loop() {

  if (millis() - previousMillis > interval) {
    previousMillis += interval;  
    stateMachine.handler(handleHardwareEvent());
  }

}

Events handleHardwareEvent ()
{
  Events event;
  event = checkBackButton();
  if (event != NO_EVENT){ 
    return event; // buttons take priority
  }
  event = checkEncButton();
  if (event != NO_EVENT){ 
    return event; // buttons take priority
  }
  event = checkRotaryEncoder();
  if (event != NO_EVENT){ 
    return event;
  }
  //no more events to check
  return event;
}

Events checkEncButton()
{
  static unsigned long btn_timer = 0;
  static bool btn_ready = 1;
  unsigned long btn_hold_length = 30UL;
  unsigned long btn_off_length = 20UL;
  if ((digitalRead(ENC_BTN_PIN) == 0) && (btn_ready==1))
  {
    if(btn_timer == 0)
    {
      btn_timer = millis();
    }
    if( millis() - btn_timer > btn_hold_length)
    {    
      btn_timer = 0;
      btn_ready = 0;
    }
  }
  else if ((digitalRead(ENC_BTN_PIN) == 1) && (btn_ready==0))
  {
    if(btn_timer == 0)
    {
      btn_timer = millis();
    }
    if((millis() - btn_timer) > btn_off_length)
    {
      btn_timer = 0;
      btn_ready = 1;
      return ENC_BTN; // only send event when button released + btn_off_length
    }
  }
  return NO_EVENT;
}

Events checkBackButton()
{
  static unsigned long btn_timer = 0;
  static bool btn_ready = 1;
  unsigned long btn_hold_length = 30UL;
  unsigned long btn_off_length = 20UL;
  if ((digitalRead(BACK_BTN_PIN) == 0) && (btn_ready==1))
  {
    if(btn_timer == 0)
    {
      btn_timer = millis();
    }
    if( millis() - btn_timer > btn_hold_length)
    {    
      btn_timer = 0;
      btn_ready = 0;
    }
  }
  else if ((digitalRead(BACK_BTN_PIN) == 1) && (btn_ready==0))
  {
    if(btn_timer == 0)
    {
      btn_timer = millis();
    }
    if((millis() - btn_timer) > btn_off_length)
    {
      btn_timer = 0;
      btn_ready = 1;
      return BACK_BTN; // only send event when button released + btn_off_length
    }
  }
  return NO_EVENT;
}

Events checkRotaryEncoder()
{
  static unsigned long enc_timer = 0;
  static bool encoder_ready = 1;
  unsigned long enc_off_length = 20UL;
  //todo off time

  static int lastReportedPos = -1;
  if(encoder_ready)
  {
    if (encoderPosCount != lastReportedPos) 
    {
      if (lastReportedPos > encoderPosCount) 
      {
        lastReportedPos = encoderPosCount;
        encoder_ready = 0; 
        return ENC_RIGHT;
      }
      else if (lastReportedPos < encoderPosCount) 
      {
        lastReportedPos = encoderPosCount;
        encoder_ready = 0; 
        return ENC_LEFT;
      }
    }
  }
  else
  {
    if(enc_timer == 0)
    {
      enc_timer = millis();
    }
    if((millis() - enc_timer) > enc_off_length)
    {
      lastReportedPos = encoderPosCount; // reset for next event 
      encoder_ready = 1;
      enc_timer = 0;
    }
  }
  return NO_EVENT;
}

void updateEncoder() {
  int MSB = digitalRead(ENC_PIN_A); // MSB = most significant bit
  int LSB = digitalRead(ENC_PIN_B); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB; // Converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; // Adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPosCount++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPosCount--;

  lastEncoded = encoded; // Store this value for next time
}
