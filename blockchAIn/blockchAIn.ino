#include <Servo.h>

/*************************************************************
  File:      blockchAIn.ino
  Contents:  ME210 final robot code

  History:
  when     who  what/why
  ----     ---  ---------------------------------------------
  2/22     JL   tape sensor code
  2/25     JL   integrate kyles motor, servo code, add fsm
 ************************************************************/

// TAPE SENSOR CONSTANTS
//#define BLACK_THRES  0 //unused because we're defining minimums
#define  GREY_THRES   10
#define  WHITE_THRES  40

//SERVO CONSTANTS
#define LSERVO        9
#define RSERVO        10

#define L_CLOSED      90
#define L_OPEN        180
#define R_CLOSED      180
#define R_OPEN        0


// MOTOR CONSTANTS
#define  RIGHT_MOTOR_SPEED_ON  61
#define  LEFT_MOTOR_SPEED_ON 60


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
Servo leftServo;
Servo rightServo;

IntervalTimer leftServoTimer;
IntervalTimer rightServoTimer;
IntervalTimer backToMovingTimer;

//state definitions
typedef enum { 
  STATE_WAITING, STATE_MOVING, STATE_TURNING, STATE_BUZZWORD_A, STATE_BUZZWORD_B, STATE_BUZZWORD_WAITING
} States_t;
States_t state;

typedef enum { 
  LINE_BLACK, LINE_WHITE, LINE_GREY
} Linecolor;

Linecolor front_middle;
Linecolor front_left;
Linecolor front_right;
Linecolor side;

//BUZZWORD FLAGS
bool ROUND_A_DONE = false;
bool DONE_B_WAITING = false;
//DEBUG FLAGS
bool DEBUG_TAPE_SENSOR = false;
bool DEBUG_TAPE = false;
bool DEBUG_STATE = false;
bool USE_STATES = false;


void setup() {
  Serial.begin(9600);
  state = STATE_WAITING;
  
  pinMode(LMOT_OUT1, OUTPUT);
  pinMode(LMOT_OUT2, OUTPUT);
  pinMode(RMOT_OUT1, OUTPUT);
  pinMode(RMOT_OUT2, OUTPUT);
  
  leftServo.attach(LSERVO);
  rightServo.attach(RSERVO);
  leftServo.write(L_CLOSED);
  rightServo.write(R_CLOSED);
}

void loop() {
  
 
  readTape();
  if (USE_STATES) {
      switch (state) { 
    case STATE_WAITING:
      //action
        stopMotors();
      //transition
      if (front_middle == LINE_BLACK) {
        //Serial.println("TRANSITION: MOVING");
        state = STATE_MOVING;
        }
      break;
    case STATE_MOVING:
      //adjustment actions
      if (front_left == LINE_WHITE && front_right == LINE_WHITE) { 
        Serial.println("Straight");
        forwardMotors();
      }
      else if (front_left == LINE_BLACK || front_left == LINE_GREY) {
        Serial.println("Adjusting Left");
        adjustLeft();
        }
      else if (front_right == LINE_BLACK  || front_right == LINE_GREY) {
        Serial.println("Adjusting Right");
        adjustRight();        
        } else { 
          Serial.println("sadness");
          }

      //transition
      if (side == LINE_GREY && !ROUND_A_DONE) {
        //Serial.println("TRANSITION: BUZZWORD A");
        state = STATE_BUZZWORD_A;
      } else if (side == LINE_GREY && ROUND_A_DONE) {
        //Serial.println("TRANSITION: BUZZWORD B");
        state = STATE_BUZZWORD_B;
      } else if (side == LINE_BLACK && front_middle == LINE_BLACK) {
        //Serial.println("TRANSITION: TURNING");
        state = STATE_TURNING;  
      }
      break;
    case STATE_BUZZWORD_WAITING:
        if (DONE_B_WAITING) state = STATE_MOVING;
        break;
    case STATE_TURNING:
      //action
        turnRightMotors();
      //transition
      ROUND_A_DONE = false;
      state = STATE_WAITING;
      break;
    case STATE_BUZZWORD_A:
      //action
      stopMotors();
      //release servo
      openLeftServo();
      ROUND_A_DONE = true;
      //transition
      backToMovingTimer.begin(moving, 1000000);
      state = STATE_BUZZWORD_WAITING;
      break;
    case STATE_BUZZWORD_B:
      //action
      stopMotors();
      //release servo
      openRightServo();
      //transition
      backToMovingTimer.begin(moving, 1000000);
      state = STATE_BUZZWORD_WAITING;
      break;
    }
    
  if (DEBUG_STATE) {
    Serial.print("state: ");
    Serial.println(state);
  } 
  
  }
  if (DEBUG_TAPE) {
    Serial.print("Front: ");
    Serial.println(front_middle);
    Serial.print("Left: ");
    Serial.println(front_left);
    Serial.print("Right: ");
    Serial.println(front_right);
    Serial.print("Side: ");
    Serial.println(side);
    delay(500);
    }
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
  if (fmVal > front_max) front_max = fmVal;
  if (flVal > left_max) left_max = flVal;
  if (frVal > right_max) right_max = frVal;
  if (sVal > side_max) side_max = sVal;
  
  front_middle = updateTapeValues(fmVal, front_max, "front");
  front_left = updateTapeValues(flVal, left_max, "left");
  front_right = updateTapeValues(frVal, right_max, "right");
  side = updateTapeValues(sVal, side_max, "side");
}


Linecolor updateTapeValues(int val, int maxVal, String which) { 
  //updates the Linecolor variables accordingly
  int percent = val * 100 / maxVal;

    
  if (DEBUG_TAPE_SENSOR) { 
    Serial.print(which);
    Serial.print(" val: ");
    Serial.print(val);
    Serial.print(", percent : ");
    Serial.println(percent);
  } 
  
  if (percent >= WHITE_THRES ) { 
    return LINE_WHITE;
  } else if (percent >= GREY_THRES) { 
    return LINE_GREY;
  } else { 
    return LINE_BLACK;
  }

}

void backwardMotors() {   
    analogWrite(LMOT_OUT1, 0);
    analogWrite(LMOT_OUT2, LEFT_MOTOR_SPEED_ON);
    analogWrite(RMOT_OUT1, 0);
    analogWrite(RMOT_OUT2, RIGHT_MOTOR_SPEED_ON);
}

void adjustLeft() {   
    analogWrite(LMOT_OUT1, LEFT_MOTOR_SPEED_ON+10);
    analogWrite(LMOT_OUT2, 0);
    analogWrite(RMOT_OUT1, RIGHT_MOTOR_SPEED_ON);
    analogWrite(RMOT_OUT2, 0);
}

void adjustRight() {   
    analogWrite(LMOT_OUT1, LEFT_MOTOR_SPEED_ON);
    analogWrite(LMOT_OUT2, 0);
    analogWrite(RMOT_OUT1, RIGHT_MOTOR_SPEED_ON+10);
    analogWrite(RMOT_OUT2, 0);
}

void forwardMotors() { 
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
    analogWrite(LMOT_OUT1, 0);
    analogWrite(LMOT_OUT2, LEFT_MOTOR_SPEED_ON);
    analogWrite(RMOT_OUT1, RIGHT_MOTOR_SPEED_ON);
    analogWrite(RMOT_OUT2, 0);
    delay(1480);
    forwardMotors();
    delay(5000);
    stopMotors();
  }

void openLeftServo(){
  leftServo.write(L_OPEN);
//  Serial.println("open left");
  leftServoTimer.begin(closeLeft, 2000000);
}
void openRightServo(){
  rightServo.write(R_OPEN);
//    Serial.println("open right");
  rightServoTimer.begin(closeRight, 2000000);
}

void closeLeft() {
  Serial.println("closing left");
  leftServo.write(L_CLOSED);
  leftServoTimer.end();
}

void closeRight() {
  Serial.println("closing right");
  rightServo.write(R_CLOSED);
  rightServoTimer.end();
}

void moving() {
  DONE_B_WAITING = true;
  Serial.println("DONE WAITING!");
  backToMovingTimer.end();
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
    case 'q': 
      Serial.println("left servo");
      openLeftServo();
      break;
    case 'w': 
      Serial.println("right servo");
      openRightServo();
      break;
    case '[': 
      Serial.println("adjust left");
      adjustLeft();
      break;
    case ']': 
      Serial.println("adjust right");
      adjustRight();
      break;
    }
}
