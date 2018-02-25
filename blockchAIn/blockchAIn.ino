
/*************************************************************
  File:      blockchAIn.ino
  Contents:  ME210 final robot code

  History:
  when     who  what/why
  ----     ---  ---------------------------------------------
  2/22     JL   tape sensor code
  2/25     JL   integrate kyles motor code
 ************************************************************/

// TAPE SENSOR CONSTANTS
//#define BLACK_THRES    10 //unused because we're defining minimums
#define  GREY_THRES   25
#define  WHITE_THRES  90

// MOTOR CONSTANTS
#define  LEFT_MOTOR_SPEED_ON  60
#define  RIGHT_MOTOR_SPEED_ON 70


int front_max = 100;
int left_max = 100;
int right_max = 100;
int side_max = 80; //this one is lower than the rest for some reason

// PIN CONFIG
int TAPE_READ_FRONT_MIDDLE_PIN = A0;
int TAPE_READ_FRONT_LEFT_PIN = A1;
int TAPE_READ_FRONT_RIGHT_PIN = A2;
int TAPE_READ_SIDE_PIN = A3;

int LMOT_OUT1 = A9; // Arduino PWM output pin A6; connect to IBT-2 pin 1 (RPWM)
int LMOT_OUT2 = A8; // Arduino PWM output pin A5; connect to IBT-2 pin 2 (LPWM)
int RMOT_OUT1 = A7;
int RMOT_OUT2 = A6;

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
bool DEBUG_TAPE_SENSOR = false;

void setup() {
  Serial.begin(9600);
  state = STATE_WAITING;
  
  pinMode(LMOT_OUT1, OUTPUT);
  pinMode(LMOT_OUT2, OUTPUT);
  pinMode(RMOT_OUT1, OUTPUT);
  pinMode(RMOT_OUT2, OUTPUT);
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

  if (Serial.available()) respToKey();
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
    delay(500);
  } 
  
  updateTapeValues(fmVal, front_middle);
  updateTapeValues(flVal, front_left);
  updateTapeValues(frVal, front_right);
  updateTapeValues(sVal, side);
}

void updateMaxTape(int val, int tape_max) {
  if (val > tape_max) {
    tape_max = val;
    }
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

void forwardMotors() {   
    analogWrite(LMOT_OUT1, 0);
    analogWrite(LMOT_OUT2, LEFT_MOTOR_SPEED_ON);
    analogWrite(RMOT_OUT1, 0);
    analogWrite(RMOT_OUT2, RIGHT_MOTOR_SPEED_ON);
  }

void backwardMotors() { 
    analogWrite(LMOT_OUT1, LEFT_MOTOR_SPEED_ON);
    analogWrite(LMOT_OUT2, 0);
    analogWrite(RMOT_OUT1, RIGHT_MOTOR_SPEED_ON);
    analogWrite(RMOT_OUT2, 0);
}
  
void stopMotors() {
    analogWrite(LMOT_OUT1, 0);
    analogWrite(LMOT_OUT2, 0);
    analogWrite(RMOT_OUT1, 0);
    analogWrite(RMOT_OUT2, 0);
  } 

void turnRightMotors() {
    analogWrite(LMOT_OUT1, LEFT_MOTOR_SPEED_ON);
    analogWrite(LMOT_OUT2, 0);
    analogWrite(RMOT_OUT1, 0);
    analogWrite(RMOT_OUT2, RIGHT_MOTOR_SPEED_ON);
    delay(1480);
    stopMotors();
    //todo replace with forwardMotor() when done
  }

//for debugging...
void respToKey() {
  int key = Serial.read();
    switch (key) { 
    case 'f':
      Serial.println("forward");
      forwardMotors();
      break;
    case 'b':
      Serial.println("back");
      backwardMotors();
      break;
    case 's':
      Serial.println("stop");
      stopMotors();
      break;
    case 'r':
      Serial.println("turn");
      turnRightMotors();
      break;
    }
}
