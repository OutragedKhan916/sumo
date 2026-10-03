// ============================================================
// UNO Q SUMO ROBOT - FULL TEST CODE
// Sensors + Ultrasonic + Motors
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

#define LINE_FRONT_LEFT   A3
#define LINE_FRONT_RIGHT  A1
#define LINE_BACK         A2


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
  Serial.println("        FULL HARDWARE TEST");
  Serial.println("========================================");
  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // ==========================================================
  // SENSOR TEST
  // ==========================================================

  int irFL = digitalRead(IR_FRONT_LEFT);

  int irFR = digitalRead(IR_FRONT_RIGHT);

  // Back IR is flipped
  int irBack = !digitalRead(IR_BACK);


  int lineFL = digitalRead(LINE_FRONT_LEFT);

  int lineFR = digitalRead(LINE_FRONT_RIGHT);

  int lineBack = digitalRead(LINE_BACK);


  // ==========================================================
  // ULTRASONIC
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
  // PRINT SENSOR VALUES
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


  // ==========================================================
  // MOTOR TEST
  // ==========================================================

  Serial.println();
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


  // ==========================================================
  // STOP
  // ==========================================================

  stopMotors();

  Serial.println();
  Serial.println("ALL MOTORS STOPPED");

  Serial.println("========================================");


  // Wait 1 second before repeating
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
// STOP ALL MOTORS
// ============================================================

void stopMotors() {

  analogWrite(M1_PWM, 0);

  analogWrite(M2_PWM, 0);

  analogWrite(M3_PWM, 0);

  analogWrite(M4_PWM, 0);
}