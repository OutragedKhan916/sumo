// ============================================================
// UNO Q AGGRESSIVE SUMO ROBOT
// ============================================================
//
// Behavior:
// 1. Ultrasonic boots immediately when powered on
// 2. Robot waits for START button
// 3. Ultrasonic keeps running while waiting
// 4. Button pressed -> 5 second countdown
// 5. Ultrasonic keeps running during countdown
// 6. After GO -> sensors control robot
// 7. LINE SENSORS ALWAYS HAVE PRIORITY
// 8. Enemy detected -> ATTACK HARD
// 9. No enemy -> SEARCH / SWEEP
// 10. Back IR is inverted
//
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
#define LINE_BACK         A2


// ============================================================
// ULTRASONIC
// ============================================================

#define ULTRASONIC_TRIG  11   // D11
#define ULTRASONIC_ECHO  A4   // A4


// ============================================================
// START BUTTON
// ============================================================

#define START_BUTTON     1    // D1


// ============================================================
// MOTORS
// ============================================================

// Motor 1
#define M1_DIR  7
#define M1_PWM  9

// Motor 2
#define M2_DIR  8
#define M2_PWM  10

// Motor 3
#define M3_DIR  12
#define M3_PWM  5

// Motor 4
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
// SPEED SETTINGS
// ============================================================

#define ATTACK_SPEED 255
#define SEARCH_SPEED 180
#define TURN_SPEED   230
#define ESCAPE_SPEED 255


// ============================================================
// ULTRASONIC SETTINGS
// ============================================================

#define ENEMY_DISTANCE 100
#define ULTRA_TIMEOUT 8000


// ============================================================
// SEARCH SETTINGS
// ============================================================

unsigned long lastSearchChange = 0;

bool searchRight = true;


// ============================================================
// ROBOT START STATE
// ============================================================

bool robotStarted = false;


// ============================================================
// ULTRASONIC VARIABLES
// ============================================================

float currentDistance = -1;

bool ultrasonicValid = false;

unsigned long lastUltraRead = 0;

#define ULTRA_INTERVAL 30


// ============================================================
// MOTOR CONTROL
// ============================================================

void setMotor(
  int dirPin,
  int pwmPin,
  int speed,
  bool inverted
) {

  speed = constrain(speed, -255, 255);

  bool forward = speed >= 0;

  if (inverted) {
    forward = !forward;
  }

  digitalWrite(
    dirPin,
    forward ? HIGH : LOW
  );

  analogWrite(
    pwmPin,
    abs(speed)
  );
}


void drive(
  int leftSpeed,
  int rightSpeed
) {

  setMotor(
    M1_DIR,
    M1_PWM,
    leftSpeed,
    INVERT_M1
  );

  setMotor(
    M2_DIR,
    M2_PWM,
    leftSpeed,
    INVERT_M2
  );

  setMotor(
    M3_DIR,
    M3_PWM,
    rightSpeed,
    INVERT_M3
  );

  setMotor(
    M4_DIR,
    M4_PWM,
    rightSpeed,
    INVERT_M4
  );
}


void stopMotors() {

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  analogWrite(M3_PWM, 0);
  analogWrite(M4_PWM, 0);
}


// ============================================================
// MOVEMENT
// ============================================================

void forward() {

  drive(
    ATTACK_SPEED,
    ATTACK_SPEED
  );
}


void backward() {

  drive(
    -ESCAPE_SPEED,
    -ESCAPE_SPEED
  );
}


void turnLeft() {

  drive(
    -TURN_SPEED,
    TURN_SPEED
  );
}


void turnRight() {

  drive(
    TURN_SPEED,
    -TURN_SPEED
  );
}


// ============================================================
// IR DETECTION
// ============================================================

bool frontLeftEnemy() {

  return digitalRead(
    IR_FRONT_LEFT
  ) == LOW;
}


bool frontRightEnemy() {

  return digitalRead(
    IR_FRONT_RIGHT
  ) == LOW;
}


bool backEnemy() {

  // BACK IR IS FLIPPED

  return digitalRead(
    IR_BACK
  ) == HIGH;
}


// ============================================================
// LINE DETECTION
// ============================================================

bool frontLeftLine() {

  return digitalRead(
    LINE_FRONT_LEFT
  ) == LOW;
}


bool frontRightLine() {

  return digitalRead(
    LINE_FRONT_RIGHT
  ) == LOW;
}


bool backLine() {

  return digitalRead(
    LINE_BACK
  ) == LOW;
}


// ============================================================
// ULTRASONIC RAW READ
// ============================================================

float getDistance() {

  digitalWrite(
    ULTRASONIC_TRIG,
    LOW
  );

  delayMicroseconds(2);

  digitalWrite(
    ULTRASONIC_TRIG,
    HIGH
  );

  delayMicroseconds(10);

  digitalWrite(
    ULTRASONIC_TRIG,
    LOW
  );


  unsigned long duration =
    pulseIn(
      ULTRASONIC_ECHO,
      HIGH,
      ULTRA_TIMEOUT
    );


  if (duration == 0) {

    return -1;
  }


  float distance =
    duration * 0.0343 / 2.0;


  return distance;
}


// ============================================================
// ULTRASONIC UPDATE
// ============================================================
//
// Ultrasonic runs continuously.
//
// This function is called:
// - while robot is waiting
// - during countdown
// - after GO
//
// ============================================================

void updateUltrasonic() {

  unsigned long now =
    millis();


  if (
    now - lastUltraRead <
    ULTRA_INTERVAL
  ) {

    return;
  }


  lastUltraRead =
    now;


  float distance =
    getDistance();


  if (
    distance > 0 &&
    distance <= 250
  ) {

    currentDistance =
      distance;

    ultrasonicValid =
      true;

  }
  else {

    currentDistance =
      -1;

    ultrasonicValid =
      false;
  }
}


// ============================================================
// ULTRASONIC ENEMY DETECTION
// ============================================================

bool ultrasonicEnemy() {

  if (
    ultrasonicValid &&
    currentDistance <= ENEMY_DISTANCE
  ) {

    return true;
  }


  return false;
}


// ============================================================
// LINE SAFETY
// ============================================================

bool boundaryDetected() {

  if (
    frontLeftLine()
  ) {

    return true;
  }


  if (
    frontRightLine()
  ) {

    return true;
  }


  if (
    backLine()
  ) {

    return true;
  }


  return false;
}


// ============================================================
// BOUNDARY ESCAPE
// ============================================================

void escapeBoundary() {

  bool leftLine =
    frontLeftLine();

  bool rightLine =
    frontRightLine();

  bool rearLine =
    backLine();


  // BOTH FRONT LINE SENSORS

  if (
    leftLine &&
    rightLine
  ) {

    backward();

    delay(300);

    turnRight();

    delay(350);

    return;
  }


  // FRONT LEFT

  if (leftLine) {

    backward();

    delay(280);

    turnRight();

    delay(300);

    return;
  }


  // FRONT RIGHT

  if (rightLine) {

    backward();

    delay(280);

    turnLeft();

    delay(300);

    return;
  }


  // BACK

  if (rearLine) {

    forward();

    delay(300);

    return;
  }
}


// ============================================================
// ENEMY DETECTION
// ============================================================

bool enemyDetected() {

  if (
    frontLeftEnemy()
  ) {

    return true;
  }


  if (
    frontRightEnemy()
  ) {

    return true;
  }


  if (
    backEnemy()
  ) {

    return true;
  }


  if (
    ultrasonicEnemy()
  ) {

    return true;
  }


  return false;
}


// ============================================================
// AGGRESSIVE ATTACK
// ============================================================

void attackEnemy() {

  bool left =
    frontLeftEnemy();

  bool right =
    frontRightEnemy();

  bool rear =
    backEnemy();


  // BOTH FRONT IR
  // FULL ATTACK

  if (
    left &&
    right
  ) {

    forward();

    return;
  }


  // FRONT LEFT

  if (left) {

    drive(
      120,
      ATTACK_SPEED
    );

    return;
  }


  // FRONT RIGHT

  if (right) {

    drive(
      ATTACK_SPEED,
      120
    );

    return;
  }


  // ENEMY BEHIND

  if (rear) {

    turnLeft();

    return;
  }


  // ULTRASONIC ONLY
  // CHARGE FORWARD

  forward();
}


// ============================================================
// SEARCH
// ============================================================

void searchForEnemy() {

  unsigned long now =
    millis();


  // Change direction every 700 ms

  if (
    now - lastSearchChange >=
    700
  ) {

    lastSearchChange =
      now;

    searchRight =
      !searchRight;
  }


  if (searchRight) {

    drive(
      SEARCH_SPEED,
      -SEARCH_SPEED
    );

  }
  else {

    drive(
      -SEARCH_SPEED,
      SEARCH_SPEED
    );
  }
}


// ============================================================
// ULTRASONIC STARTUP MONITOR
// ============================================================
//
// Robot does NOT move.
//
// Ultrasonic keeps reading while waiting for button.
//
// ============================================================

void startupUltrasonic() {

  updateUltrasonic();


  static unsigned long lastPrint =
    0;


  unsigned long now =
    millis();


  // Print distance every 250 ms

  if (
    now - lastPrint >=
    250
  ) {

    lastPrint =
      now;


    if (ultrasonicValid) {

      Serial.print(
        "Ultrasonic: "
      );

      Serial.print(
        currentDistance
      );

      Serial.println(
        " cm"
      );

    }
    else {

      Serial.println(
        "Ultrasonic: NO TARGET"
      );
    }
  }
}


// ============================================================
// WAIT FOR BUTTON
// ============================================================
//
// Ultrasonic is ACTIVE while waiting.
//
// D1:
// HIGH = button not pressed
// LOW  = button pressed
//
// ============================================================

void waitForStart() {

  stopMotors();


  Serial.println();
  Serial.println("==============================");
  Serial.println("BESOMI SUMO ROBOT");
  Serial.println("ULTRASONIC ACTIVE");
  Serial.println("PRESS BUTTON ON D1");
  Serial.println("==============================");


  // ==========================================================
  // WAIT FOR BUTTON PRESS
  // ==========================================================

  while (
    digitalRead(START_BUTTON) == HIGH
  ) {

    stopMotors();

    startupUltrasonic();

    delay(5);
  }


  // ==========================================================
  // BUTTON DEBOUNCE
  // ==========================================================

  delay(50);


  // ==========================================================
  // WAIT FOR BUTTON RELEASE
  // ==========================================================

  while (
    digitalRead(START_BUTTON) == LOW
  ) {

    stopMotors();

    startupUltrasonic();

    delay(5);
  }


  // ==========================================================
  // 5 SECOND COUNTDOWN
  // ==========================================================

  Serial.println();
  Serial.println("STARTING IN");


  for (
    int i = 5;
    i >= 1;
    i--
  ) {

    Serial.print(i);
    Serial.println("...");


    unsigned long countdownStart =
      millis();


    // Keep ultrasonic alive
    // for the entire second

    while (
      millis() - countdownStart <
      1000
    ) {

      stopMotors();

      startupUltrasonic();

      delay(5);
    }
  }


  // ==========================================================
  // GO
  // ==========================================================

  Serial.println("GO!");


  robotStarted =
    true;


  // Reset search timer

  lastSearchChange =
    millis();
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(9600);


  // ==========================================================
  // IR SENSORS
  // ==========================================================

  pinMode(
    IR_FRONT_LEFT,
    INPUT_PULLUP
  );

  pinMode(
    IR_FRONT_RIGHT,
    INPUT_PULLUP
  );

  pinMode(
    IR_BACK,
    INPUT_PULLUP
  );


  // ==========================================================
  // LINE SENSORS
  // ==========================================================

  pinMode(
    LINE_FRONT_LEFT,
    INPUT_PULLUP
  );

  pinMode(
    LINE_FRONT_RIGHT,
    INPUT_PULLUP
  );

  pinMode(
    LINE_BACK,
    INPUT_PULLUP
  );


  // ==========================================================
  // ULTRASONIC
  // ==========================================================

  pinMode(
    ULTRASONIC_TRIG,
    OUTPUT
  );

  pinMode(
    ULTRASONIC_ECHO,
    INPUT
  );


  digitalWrite(
    ULTRASONIC_TRIG,
    LOW
  );


  // ==========================================================
  // BUTTON
  // ==========================================================

  pinMode(
    START_BUTTON,
    INPUT_PULLUP
  );


  // ==========================================================
  // MOTORS
  // ==========================================================

  pinMode(
    M1_DIR,
    OUTPUT
  );

  pinMode(
    M1_PWM,
    OUTPUT
  );


  pinMode(
    M2_DIR,
    OUTPUT
  );

  pinMode(
    M2_PWM,
    OUTPUT
  );


  pinMode(
    M3_DIR,
    OUTPUT
  );

  pinMode(
    M3_PWM,
    OUTPUT
  );


  pinMode(
    M4_DIR,
    OUTPUT
  );

  pinMode(
    M4_PWM,
    OUTPUT
  );


  // ==========================================================
  // STOP MOTORS
  // ==========================================================

  stopMotors();


  // ==========================================================
  // IMPORTANT:
  // START ULTRASONIC IMMEDIATELY
  // ==========================================================

  Serial.println();
  Serial.println("SYSTEM POWERED");
  Serial.println("STARTING ULTRASONIC...");


  // Take an initial ultrasonic reading

  updateUltrasonic();


  delay(100);


  // ==========================================================
  // WAIT FOR BUTTON
  // ==========================================================

  waitForStart();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  // ==========================================================
  // ROBOT HAS NOT STARTED
  // ==========================================================

  if (!robotStarted) {

    stopMotors();

    // Keep ultrasonic alive

    updateUltrasonic();

    return;
  }


  // ==========================================================
  // 1. UPDATE ULTRASONIC FIRST
  // ==========================================================

  updateUltrasonic();


  // ==========================================================
  // 2. LINE SENSORS ALWAYS HAVE PRIORITY
  // ==========================================================

  if (
    boundaryDetected()
  ) {

    escapeBoundary();

    return;
  }


  // ==========================================================
  // 3. LOOK FOR ENEMY
  // ==========================================================

  if (
    enemyDetected()
  ) {

    attackEnemy();

    return;
  }


  // ==========================================================
  // 4. NO ENEMY
  // SEARCH
  // ==========================================================

  searchForEnemy();
}