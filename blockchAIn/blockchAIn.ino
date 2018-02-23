
/*************************************************************
  File:      blockchAIn.ino
  Contents:  ME210 final robot code

  History:
  when     who  what/why
  ----     ---  ---------------------------------------------
  2/22     JL   tape sensor code
 ************************************************************/

// TAPE SENSOR CONSTANTS
//#define BLACK_THRES    10 //unused because we're defining minimums
#define  GREY_THRES   25
#define  WHITE_THRES  90

// PIN CONFIG
int TAPE_READ_FRONT_MIDDLE_PIN = A0;
int TAPE_READ_FRONT_LEFT_PIN = A1;
int TAPE_READ_FRONT_RIGHT_PIN = A2;
int TAPE_READ_SIDE_PIN = A3;

//state definitions
typedef enum { 
  STATE_WAITING, STATE_MOVING, STATE_TURNING, STATE_BUZZWORD_A, STATE_BUZZWORD_B
} States_t;
States_t state;

typedef enum { 
  LINE_BLACK, LINE_WHITE, LINE_GREY
} Linecolor;

Linecolor front_middle;
Linecolor front_left;
Linecolor front_right;
Linecolor side;

//DEBUG FLAGS
bool DEBUG_TAPE_SENSOR = true;

void setup() {
  Serial.begin(9600);
  state = STATE_WAITING;
}

void loop() {
  switch (state) { 
    case STATE_WAITING:
      break;
    case STATE_MOVING:
      break;
    case STATE_TURNING:
      break;
    case STATE_BUZZWORD_A:
      break;
    case STATE_BUZZWORD_B:
      break;
    }
  
  readTape();
}

void readTape() { 
  //reads from all tape sensors 
  int fmVal = analogRead(TAPE_READ_FRONT_MIDDLE_PIN);
  int flVal = analogRead(TAPE_READ_FRONT_LEFT_PIN);
  int frVal = analogRead(TAPE_READ_FRONT_RIGHT_PIN);
  int sVal = analogRead(TAPE_READ_SIDE_PIN);

  if (DEBUG_TAPE_SENSOR) { 
    Serial.print("Front value: ");
    Serial.println(fmVal);
    Serial.print("Left value: ");
    Serial.println(flVal);
    Serial.print("Right value: ");
    Serial.println(frVal);
    Serial.print("Side value: ");
    Serial.println(sVal);
    delay(1000);
  } 
  
  updateTapeValues(fmVal, front_middle);
  updateTapeValues(flVal, front_left);
  updateTapeValues(frVal, front_right);
  updateTapeValues(sVal, side);
}

void updateTapeValues(int val, Linecolor lc) { 
  //updates the Linecolor variables accordingly
  if (val >= WHITE_THRES) { 
    lc = LINE_WHITE;
  } else if (val >= GREY_THRES) { 
    lc = LINE_GREY;
  } else { 
    lc = LINE_BLACK;
  }
}
