/* Arduino UNO Alarm Clock for Telecomm course
*/
#define JOYSTICK_X_PIN A0 
#define JOYSTICK_Y_PIN A1
#define TEMPERATURE_PIN A2
#define FLAME_SENSOR_PIN A5
#define JOYSTICK_BUTTON_PIN 3

#define BUZZER_PIN 4  // Buzzer
#define IR_RECEIVE_PIN 7 // IR Remote Receiver
#define POWER_PIN 8   // Extra power pin
#define SERVO_PIN 10  // Servo PWM

// IR Remote buttons
#define CH_DOWN 69
#define CH_CHANNEL 70
#define CH_UP 71
#define PREV 68
#define NEXT 64
#define PLAY 67
#define VOL_DOWN 7
#define VOL_UP 21
#define EQ 9
#define BUTTON_0 22
#define BUTTON_100 25
#define BUTTON_200 13
#define BUTTON_1 12
#define BUTTON_2 24
#define BUTTON_3 94
#define BUTTON_4 8
#define BUTTON_5 28
#define BUTTON_6 90
#define BUTTON_7 66
#define BUTTON_8 82
#define BUTTON_9 74

#include <IRremote.h>



struct joystick_struct {
  int x;
  int y;
};
joystick_struct joystick;
bool joystick_button_pressed = false;

long start_millis = 0;  // Timer parameters for temperature sensor
long current_millis = 0;
int period = 10000;     // 10 Seconds period

char str[10] = { 0 };  // Array for received serial data bytes

int servo_angle = 100;

// --------------------------------------------------------
// -- SETUP() ---------------------------------------------
// --------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Im Alive!");

  // INITIALIZE PINS
  // Extra Power pin
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Joystick button with interrupt
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(JOYSTICK_BUTTON_PIN), joystickButton, FALLING);


  // INITIALIZATION FUNCTIONS
  start_millis = millis();
  IrReceiver.begin(IR_RECEIVE_PIN);
  analogWrite(SERVO_PIN, servo_angle);
  //sweepServo();
  //playBuzzer();
}

// --------------------------------------------------------
// -- LOOP() ----------------------------------------------
// --------------------------------------------------------

void loop() {
  // Read and Transmit temperature every period
  current_millis = millis();
  if (current_millis - start_millis >= period) {
    Serial.write('2');
    Serial.print(analogRead(TEMPERATURE_PIN));
    Serial.write('x');
    start_millis = current_millis;
  }

  if (IrReceiver.decode()) {  // IR Remote Control
    IrReceiver.resume();
    int cmd = IrReceiver.decodedIRData.command;
    IRDoCommand(cmd);
  }
  // Read from serial, first byte is ID, the rest is stored in array until 'x' is received.
  int receivedData = 0;
  while (Serial.available()) {
    receivedData = Serial.read();
    if (receivedData == '0') {  // ID 0 = SERVO
      Serial.readBytesUntil('x', str, 10);
      servo_angle = atoi(str);
      if (servo_angle > 254) { // Servo is unresponsive beyond these limits
        servo_angle = 254;
      } else if (servo_angle < 56) {
        servo_angle = 56;
      }
      analogWrite(SERVO_PIN, servo_angle);
    } else if (receivedData == '1') {  // ID 1 = BUZZER
      Serial.readBytesUntil('x', str, 10);
      if (str[0] == '1') { // Start Buzzer
        tone(BUZZER_PIN, 500);
      } else if (str[0] == '0') {
        noTone(BUZZER_PIN); // Turn Off Buzzer
        IrReceiver.begin(IR_RECEIVE_PIN); // tone() also uses timer, thus IR receiver re-initialised
      }
    }
  }
  // Reset data array to {0}
  for (int i = 0; i < 10; i++) {
    str[i] = 0;
  }

  readJoystick(&joystick);
  if (joystick_button_pressed) {
    playBuzzer();
    joystick_button_pressed = false;
  }
}

/* sweepServo() sweeps the servo at SERVO_PIN back and forth one time
*/
void sweepServo() {
  for (int dutyCycle = 0; dutyCycle < 255; dutyCycle++) {
    // changing the LED brightness with PWM
    analogWrite(SERVO_PIN, dutyCycle);
    delay(2);
  }
  for (int dutyCycle = 255; dutyCycle > 0; dutyCycle--) {
    // changing the LED brightness with PWM
    analogWrite(SERVO_PIN, dutyCycle);
    delay(2);
  }
}

/* playBuzzer() plays the Imperial March intro on the buzzer at BUZZER_PIN
*/
void playBuzzer() {
  int a = 440; // Note frequencies
  int f = 349;
  int c = 523;

  int beat = 800; // period of one beat
  // Imperial March intro
  tone(BUZZER_PIN, a);
  delay(beat);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, a);
  delay(beat);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, a);
  delay(beat);
  noTone(BUZZER_PIN);
  delay(100);

  tone(BUZZER_PIN, f);
  delay(beat / 2);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, c);
  delay(beat / 2);
  noTone(BUZZER_PIN);
  delay(100);

  tone(BUZZER_PIN, a);
  delay(beat);
  noTone(BUZZER_PIN);
  delay(100);

  tone(BUZZER_PIN, f);
  delay(beat / 2);
  noTone(BUZZER_PIN);
  delay(100);
  tone(BUZZER_PIN, c);
  delay(beat / 2);
  noTone(BUZZER_PIN);
  delay(100);

  tone(BUZZER_PIN, a);
  delay(beat * 2);
  noTone(BUZZER_PIN);
  delay(100);

  IrReceiver.begin(IR_RECEIVE_PIN); // tone() also uses timer, thus IR receiver re-initialised
}
void playBuzzerShort() {
  tone(BUZZER_PIN, 600);
  delay(500);
  noTone(BUZZER_PIN);

  IrReceiver.begin(IR_RECEIVE_PIN); // tone() also uses timer, thus IR receiver re-initialised
}

/* readJoystick() stores the x and y values of the joystick in the struct
* Avg x = 514, y = 518, max = 1023
* x = 0 at "left" / wire input
* y = 0 at "up"
*/
void readJoystick(joystick_struct* joystick) {
  joystick->x = analogRead(JOYSTICK_X_PIN);
  joystick->y = analogRead(JOYSTICK_Y_PIN);
}
// Function is called by interrupt when joystick button is pressed
void joystickButton() {
  joystick_button_pressed = true;
}

/*Switch case to handle possible IR Remote Commands
*/
void IRDoCommand(int command) {
  switch (command) {
    case (CH_DOWN):
      break;
    case (CH_CHANNEL):
      break;
    case (CH_UP):
      break;
    case (PREV):
      break;
    case (NEXT):
      break;
    case (PLAY): // Play buzzer
      playBuzzer();
      break;
    case (VOL_DOWN): // Control Servo
      servo_angle--;
      if (servo_angle > 255) {
        servo_angle = 255;
      } else if (servo_angle < 0) {
        servo_angle = 0;
      }
      analogWrite(SERVO_PIN, servo_angle);
      break;
    case (VOL_UP): // Control Servo
      servo_angle++;
      if (servo_angle > 254) {
        servo_angle = 254;
      } else if (servo_angle < 56) {
        servo_angle = 56;
      }
      analogWrite(SERVO_PIN, servo_angle);
      break;
    case (EQ): // Play Buzzer
      playBuzzerShort();
      break;
    case (BUTTON_0):
      break;
    case (BUTTON_100): // Control Servo
      servo_angle = 100;
      analogWrite(SERVO_PIN, servo_angle);
      break;
    case (BUTTON_200): // Control Servo
      servo_angle = 200;
      analogWrite(SERVO_PIN, servo_angle);
      break;
    case (BUTTON_1):
      break;
    case (BUTTON_2):
      break;
    case (BUTTON_3):
      break;
    case (BUTTON_4):
      break;
    case (BUTTON_5):
      break;
    case (BUTTON_6):
      break;
    case (BUTTON_7):
      break;
    case (BUTTON_8):
      break;
    case (BUTTON_9):
      break;
    default:
      break;
  }
}
