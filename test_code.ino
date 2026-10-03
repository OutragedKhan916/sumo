// ============================================================
// UNO Q SUMO ROBOT - SWITCHABLE TEST CODE
// 1 = MOTOR TEST
// 2 = SENSOR TEST
// ============================================================


// ============================================================
// IR SENSORS
// ============================================================

#define IR_FRONT_LEFT   0    // D0
#define IR_FRONT_RIGHT  4    // D4
#define IR_BACK         2    // D2


// ============================================================
// LINE SENSORS
// ============================================================

#define LINE_FRONT_LEFT   A1
#define LINE_FRONT_RIGHT  A3
#define LINE_BACK         A5


// ============================================================
// ULTRASONIC
// ============================================================

#define ULTRASONIC_TRIG  11   // D11
#define ULTRASONIC_ECHO  A4   // A4


// ============================================================
// MOTORS
// ============================================================

#define M1_DIR  7
#define M1_PWM  9

#define M2_DIR  8
#define M2_PWM  10

#define M3_DIR  12
#define M3_PWM  5

#define M4_DIR  13
#define M4_PWM  6


// ============================================================
// MOTOR INVERSION
// ============================================================

#define INVERT_M1 false
#define INVERT_M2 false
#define INVERT_M3 true
#define INVERT_M4 true


// ============================================================
// MOTOR TEST SPEED
// ============================================================

#define MOTOR_TEST_SPEED 120


// ============================================================
// TEST MODES
// ============================================================

#define MOTOR_MODE  1
#define SENSOR_MODE 2

int testMode = MOTOR_MODE;


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(9600);

  delay(2000);


  // -------------------------
  // IR
  // -------------------------

  pinMode(IR_FRONT_LEFT, INPUT);
  pinMode(IR_FRONT_RIGHT, INPUT);
  pinMode(IR_BACK, INPUT);


  // -------------------------
  // LINE
  // -------------------------

  pinMode(LINE_FRONT_LEFT, INPUT);
  pinMode(LINE_FRONT_RIGHT, INPUT);
  pinMode(LINE_BACK, INPUT);


  // -------------------------
  // ULTRASONIC
  // -------------------------

  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);


  // -------------------------
  // MOTORS
  // -------------------------

  pinMode(M1_DIR, OUTPUT);
  pinMode(M1_PWM, OUTPUT);

  pinMode(M2_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);

  pinMode(M3_DIR, OUTPUT);
  pinMode(M3_PWM, OUTPUT);

  pinMode(M4_DIR, OUTPUT);
  pinMode(M4_PWM, OUTPUT);


  stopMotors();


  Serial.println();
  Serial.println("========================================");
  Serial.println("       UNO Q SUMO TEST SYSTEM");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Press 1 = MOTOR TEST");
  Serial.println("Press 2 = SENSOR TEST");
  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // ==========================================================
  // CHECK FOR KEYBOARD COMMAND
  // ==========================================================

  if (Serial.available() > 0) {

    char command = Serial.read();


    // -------------------------
    // MOTOR MODE
    // -------------------------

    if (command == '1') {

      stopMotors();

      testMode = MOTOR_MODE;

      Serial.println();
      Serial.println("========================================");
      Serial.println("        MOTOR TEST MODE");
      Serial.println("========================================");
      Serial.println("Press 2 to switch to SENSOR TEST");
      Serial.println();
    }


    // -------------------------
    // SENSOR MODE
    // -------------------------

    else if (command == '2') {

      stopMotors();

      testMode = SENSOR_MODE;

      Serial.println();
      Serial.println("========================================");
      Serial.println("        SENSOR TEST MODE");
      Serial.println("========================================");
      Serial.println("Press 1 to switch to MOTOR TEST");
      Serial.println();
    }
  }


  // ==========================================================
  // RUN SELECTED MODE
  // ==========================================================

  if (testMode == MOTOR_MODE) {

    motorTest();

  }
  else if (testMode == SENSOR_MODE) {

    sensorTest();

  }
}


// ============================================================
// MOTOR TEST
// ============================================================

void motorTest() {

  // -------------------------
  // Check if user pressed 2
  // -------------------------

  if (Serial.available() > 0) {

    char command = Serial.read();

    if (command == '2') {

      stopMotors();

      testMode = SENSOR_MODE;

      Serial.println();
      Serial.println("Switching to SENSOR TEST...");
      Serial.println();

      return;
    }
  }


  Serial.println("----------------------------------------");
  Serial.println("MOTOR TEST");
  Serial.println("Keep robot lifted!");
  Serial.println();


  // -------------------------
  // MOTOR 1
  // -------------------------

  Serial.println("M1 running...");

  runMotor(
    M1_DIR,
    M1_PWM,
    INVERT_M1
  );

  delay(1000);


  // Check for mode switch
  if (checkForSensorMode()) return;


  // -------------------------
  // MOTOR 2
  // -------------------------

  Serial.println("M2 running...");

  runMotor(
    M2_DIR,
    M2_PWM,
    INVERT_M2
  );

  delay(1000);


  if (checkForSensorMode()) return;


  // -------------------------
  // MOTOR 3
  // -------------------------

  Serial.println("M3 running...");

  runMotor(
    M3_DIR,
    M3_PWM,
    INVERT_M3
  );

  delay(1000);


  if (checkForSensorMode()) return;


  // -------------------------
  // MOTOR 4
  // -------------------------

  Serial.println("M4 running...");

  runMotor(
    M4_DIR,
    M4_PWM,
    INVERT_M4
  );

  delay(1000);


  if (checkForSensorMode()) return;


  // -------------------------
  // STOP
  // -------------------------

  stopMotors();

  Serial.println();
  Serial.println("ALL MOTORS STOPPED");
  Serial.println("Repeating motor test...");
  Serial.println();

  delay(1000);
}


// ============================================================
// SENSOR TEST
// ============================================================

void sensorTest() {

  // -------------------------
  // Check for motor command
  // -------------------------

  if (Serial.available() > 0) {

    char command = Serial.read();

    if (command == '1') {

      stopMotors();

      testMode = MOTOR_MODE;

      Serial.println();
      Serial.println("Switching to MOTOR TEST...");
      Serial.println();

      return;
    }
  }


  // ==========================================================
  // READ IR
  // ==========================================================

  int irFL = digitalRead(IR_FRONT_LEFT);

  int irFR = digitalRead(IR_FRONT_RIGHT);

  // Back IR is flipped
  int irBack = !digitalRead(IR_BACK);


  // ==========================================================
  // READ LINE SENSORS
  // ==========================================================

  int lineFL = digitalRead(LINE_FRONT_LEFT);

  int lineFR = digitalRead(LINE_FRONT_RIGHT);

  int lineBack = digitalRead(LINE_BACK);


  // ==========================================================
  // READ ULTRASONIC
  // ==========================================================

  digitalWrite(ULTRASONIC_TRIG, LOW);

  delayMicroseconds(2);

  digitalWrite(ULTRASONIC_TRIG, HIGH);

  delayMicroseconds(10);

  digitalWrite(ULTRASONIC_TRIG, LOW);


  unsigned long duration =
    pulseIn(ULTRASONIC_ECHO, HIGH, 8000);


  float distance = duration * 0.0343 / 2.0;


  // ==========================================================
  // PRINT
  // ==========================================================

  Serial.println("----------------------------------------");

  Serial.print("IR    FL=");
  Serial.print(irFL);

  Serial.print("  FR=");
  Serial.print(irFR);

  Serial.print("  BACK=");
  Serial.println(irBack);


  Serial.print("LINE  FL=");
  Serial.print(lineFL);

  Serial.print("  FR=");
  Serial.print(lineFR);

  Serial.print("  BACK=");
  Serial.println(lineBack);


  Serial.print("ULTRA = ");

  if (duration == 0) {

    Serial.println("NO ECHO");

  }
  else {

    Serial.print(distance);

    Serial.println(" cm");
  }


  Serial.println();
  Serial.println("Press 1 = MOTOR TEST");
  Serial.println("Press 2 = SENSOR TEST");


  // ==========================================================
  // 1 SECOND SENSOR INTERVAL
  // ==========================================================

  delay(1000);
}


// ============================================================
// RUN ONE MOTOR
// ============================================================

void runMotor(
  int dirPin,
  int pwmPin,
  bool inverted
) {

  bool direction = HIGH;


  if (inverted) {

    direction = !direction;
  }


  digitalWrite(
    dirPin,
    direction
  );


  analogWrite(
    pwmPin,
    MOTOR_TEST_SPEED
  );


  // Motor runs for 1 second
  delay(1000);


  // Stop motor
  analogWrite(
    pwmPin,
    0
  );
}


// ============================================================
// CHECK FOR SENSOR MODE
// ============================================================

bool checkForSensorMode() {

  if (Serial.available() > 0) {

    char command = Serial.read();

    if (command == '2') {

      stopMotors();

      testMode = SENSOR_MODE;

      Serial.println();
      Serial.println("========================================");
      Serial.println("        SENSOR TEST MODE");
      Serial.println("========================================");
      Serial.println();

      return true;
    }
  }

  return false;
}


// ============================================================
// STOP ALL MOTORS
// ============================================================

void stopMotors() {

  analogWrite(M1_PWM, 0);

  analogWrite(M2_PWM, 0);

  analogWrite(M3_PWM, 0);

  analogWrite(M4_PWM, 0);
}