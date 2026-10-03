// ============================================================================
// BESOMI ACADEMY 2026
// PROACTIVE SUMO ROBOT CONTROLLER
// Arduino UNO Q
// ============================================================================


// ============================================================================
// SENSOR CONFIGURATION
// ============================================================================

// IR SENSORS
// D0 = FRONT LEFT IR
// D4 = FRONT RIGHT IR
// D2 = BACK IR

#define IR_FRONT_LEFT_PIN   D0
#define IR_FRONT_RIGHT_PIN  D4
#define IR_BACK_PIN         D2


// LINE SENSORS
// A3 = FRONT LEFT LINE
// A1 = FRONT RIGHT LINE
// A2 = BACK CENTER LINE

#define LINE_FRONT_LEFT_PIN   A3
#define LINE_FRONT_RIGHT_PIN  A1
#define LINE_BACK_PIN         A2


// ULTRASONIC
// D11 = TRIG
// A4  = ECHO

#define ULTRASONIC_TRIG_PIN  D11
#define ULTRASONIC_ECHO_PIN  A4


// START BUTTON
// D1 -> BUTTON -> GND

#define START_BUTTON_PIN D1


// ============================================================================
// MOTOR CONFIGURATION
// ============================================================================

// LEFT SIDE
#define M1_DIR D7
#define M1_PWM D9

#define M2_DIR D8
#define M2_PWM D10


// RIGHT SIDE
#define M3_DIR D12
#define M3_PWM D5

#define M4_DIR D13
#define M4_PWM D6


// Motor inversion
// M3 and M4 are physically reversed

const bool INVERT_M1 = false;
const bool INVERT_M2 = false;
const bool INVERT_M3 = true;
const bool INVERT_M4 = true;


// ============================================================================
// SENSOR LOGIC
// ============================================================================

const int SENSOR_DETECTED = LOW;
const int LINE_DETECTED   = LOW;


// ============================================================================
// SPEED SETTINGS
// ============================================================================

const int SEARCH_SPEED    = 105;
const int TRACK_SPEED     = 155;
const int TURN_SPEED      = 190;
const int ATTACK_SPEED    = 255;
const int DEFENSE_SPEED   = 210;
const int ESCAPE_SPEED    = 255;
const int INTERCEPT_SPEED = 220;


// ============================================================================
// OPENING MOVE
// ============================================================================

const unsigned long OPENING_MOVE_DURATION = 250;


// ============================================================================
// ULTRASONIC SETTINGS
// ============================================================================

const float SOUND_SPEED_CM_PER_US = 0.0343;

const float MIN_TARGET_DISTANCE = 3.0;
const float MAX_TARGET_DISTANCE = 250.0;

const unsigned long ULTRASONIC_INTERVAL = 30;
const unsigned long ULTRASONIC_TIMEOUT  = 8000;


// ============================================================================
// MOTION DETECTION
// ============================================================================

const float FAST_CLOSING_SPEED = 80.0;
const float SLOW_CLOSING_SPEED = 15.0;
const float MOVING_AWAY_SPEED  = -20.0;

const unsigned long IR_SLOW_THRESHOLD = 180;


// ============================================================================
// LAST-SEEN SETTINGS
// ============================================================================

const unsigned long LAST_SEEN_MEMORY = 500;
const unsigned long LAST_SEEN_TURN_TIME = 250;


// ============================================================================
// ULTRASONIC VARIABLES
// ============================================================================

float ultrasonicDistance = -1.0;
float previousDistance = -1.0;

float closingSpeed = 0.0;
float previousClosingSpeed = 0.0;
float closingAcceleration = 0.0;

unsigned long previousDistanceTime = 0;
unsigned long lastUltrasonicRead = 0;

bool ultrasonicValid = false;


// ============================================================================
// OPPONENT DIRECTION
// ============================================================================

enum OpponentDirection
{
  OPP_NONE,
  OPP_LEFT,
  OPP_FRONT,
  OPP_RIGHT,
  OPP_BACK
};

OpponentDirection opponentDirection = OPP_NONE;
OpponentDirection lastOpponentDirection = OPP_NONE;


// ============================================================================
// OPPONENT MOTION
// ============================================================================

enum OpponentMotion
{
  MOTION_UNKNOWN,
  MOTION_APPROACHING_FAST,
  MOTION_APPROACHING_SLOW,
  MOTION_STATIONARY,
  MOTION_MOVING_AWAY,
  MOTION_CROSSING
};

OpponentMotion opponentMotion = MOTION_UNKNOWN;


// ============================================================================
// PREVIOUS IR STATES
// ============================================================================

bool previousFL = false;
bool previousFR = false;
bool previousL = false;
bool previousR = false;
bool previousBL = false;


// ============================================================================
// TRACKING TIMERS
// ============================================================================

unsigned long lastOpponentSeen = 0;
unsigned long lastPatternChangeTime = 0;
unsigned long lastTrackingUpdate = 0;


// ============================================================================
// BOUNDARY ESCAPE
// ============================================================================

enum EscapeState
{
  ESCAPE_NONE,
  ESCAPE_BACK_FROM_FRONT,
  ESCAPE_TURN_FROM_FRONT,
  ESCAPE_FORWARD_FROM_BACK,
  ESCAPE_TURN_FROM_BACK
};

EscapeState escapeState = ESCAPE_NONE;

unsigned long escapeStarted = 0;
unsigned long escapeDuration = 0;

bool escapeTurnLeft = false;


// ============================================================================
// ROBOT STATE
// ============================================================================

bool robotStarted = false;

bool openingMoveActive = false;
unsigned long openingMoveStarted = 0;


// ============================================================================
// SEARCH STATE
// ============================================================================

bool searchDirectionLeft = false;
unsigned long searchDirectionChanged = 0;

unsigned long lastSeenTurnStarted = 0;
bool lastSeenTurnActive = false;


// ============================================================================
// MOTOR CONTROL
// ============================================================================

void setMotor(
  int dirPin,
  int pwmPin,
  int speed,
  bool invert
)
{
  speed = constrain(speed, -255, 255);

  if (invert)
  {
    speed = -speed;
  }

  if (speed > 0)
  {
    digitalWrite(dirPin, HIGH);
    analogWrite(pwmPin, speed);
  }
  else if (speed < 0)
  {
    digitalWrite(dirPin, LOW);
    analogWrite(pwmPin, -speed);
  }
  else
  {
    analogWrite(pwmPin, 0);
  }
}


// ============================================================================
// DRIVE
// ============================================================================

void drive(int leftSpeed, int rightSpeed)
{
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  setMotor(M1_DIR, M1_PWM, leftSpeed, INVERT_M1);
  setMotor(M2_DIR, M2_PWM, leftSpeed, INVERT_M2);

  setMotor(M3_DIR, M3_PWM, rightSpeed, INVERT_M3);
  setMotor(M4_DIR, M4_PWM, rightSpeed, INVERT_M4);
}


// ============================================================================
// MOVEMENT
// ============================================================================

void stopMotors()
{
  drive(0, 0);
}

void forward(int speed)
{
  drive(speed, speed);
}

void backward(int speed)
{
  drive(-speed, -speed);
}

void rotateLeft(int speed)
{
  drive(-speed, speed);
}

void rotateRight(int speed)
{
  drive(speed, -speed);
}


// ============================================================================
// IR READ FUNCTIONS
// ============================================================================

bool FL()
{
  return digitalRead(IR_FRONT_LEFT_PIN) == SENSOR_DETECTED;
}

bool FR()
{
  return digitalRead(IR_FRONT_RIGHT_PIN) == SENSOR_DETECTED;
}

bool L()
{
  return false;
}

bool R()
{
  return false;
}

bool BL()
{
  // BACK IR SIGNAL IS FLIPPED
  return !digitalRead(IR_BACK_PIN);
}


// ============================================================================
// LINE READ FUNCTIONS
// ============================================================================

bool frontLeftLine()
{
  return digitalRead(LINE_FRONT_LEFT_PIN) == LINE_DETECTED;
}

bool frontRightLine()
{
  return digitalRead(LINE_FRONT_RIGHT_PIN) == LINE_DETECTED;
}

bool backLine()
{
  return digitalRead(LINE_BACK_PIN) == LINE_DETECTED;
}


// ============================================================================
// ULTRASONIC READ
// ============================================================================

float readUltrasonic()
{
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  unsigned long duration = pulseIn(
    ULTRASONIC_ECHO_PIN,
    HIGH,
    ULTRASONIC_TIMEOUT
  );

  if (duration == 0)
  {
    return -1.0;
  }

  float distance =
    (duration * SOUND_SPEED_CM_PER_US) / 2.0;

  if (
    distance < MIN_TARGET_DISTANCE ||
    distance > MAX_TARGET_DISTANCE
  )
  {
    return -1.0;
  }

  return distance;
}


// ============================================================================
// ULTRASONIC UPDATE
// ============================================================================
//
// This function can run BEFORE the robot starts.
//
// It does NOT move the motors.
//
// During the button wait and 5-second countdown, the ultrasonic sensor
// continues updating its values.
//

void updateUltrasonic()
{
  unsigned long now = millis();

  if (
    now - lastUltrasonicRead <
    ULTRASONIC_INTERVAL
  )
  {
    return;
  }

  lastUltrasonicRead = now;

  float newDistance = readUltrasonic();

  if (newDistance < 0)
  {
    ultrasonicValid = false;
    return;
  }

  ultrasonicValid = true;

  if (previousDistance > 0)
  {
    unsigned long dt =
      now - previousDistanceTime;

    if (dt > 0)
    {
      float instantClosingSpeed =
        (previousDistance - newDistance) /
        (dt / 1000.0);

      closingSpeed =
        (closingSpeed * 0.65) +
        (instantClosingSpeed * 0.35);

      float instantAcceleration =
        (closingSpeed - previousClosingSpeed) /
        (dt / 1000.0);

      closingAcceleration =
        (closingAcceleration * 0.70) +
        (instantAcceleration * 0.30);

      previousClosingSpeed =
        closingSpeed;
    }
  }

  previousDistance = newDistance;
  previousDistanceTime = now;

  ultrasonicDistance = newDistance;
}


// ============================================================================
// OPPONENT DETECTION
// ============================================================================

bool opponentDetected()
{
  if (
    FL() ||
    FR() ||
    L() ||
    R() ||
    BL()
  )
  {
    return true;
  }

  if (
    ultrasonicValid &&
    ultrasonicDistance >= MIN_TARGET_DISTANCE &&
    ultrasonicDistance <= MAX_TARGET_DISTANCE
  )
  {
    return true;
  }

  return false;
}


// ============================================================================
// OPPONENT DIRECTION
// ============================================================================

OpponentDirection getOpponentDirection()
{
  bool fl = FL();
  bool fr = FR();
  bool l = L();
  bool r = R();
  bool bl = BL();

  if (fl && fr)
  {
    return OPP_FRONT;
  }

  if (
    ultrasonicValid &&
    ultrasonicDistance >= MIN_TARGET_DISTANCE &&
    ultrasonicDistance <= MAX_TARGET_DISTANCE
  )
  {
    return OPP_FRONT;
  }

  if (fl)
  {
    return OPP_LEFT;
  }

  if (fr)
  {
    return OPP_RIGHT;
  }

  if (l)
  {
    return OPP_LEFT;
  }

  if (r)
  {
    return OPP_RIGHT;
  }

  if (bl)
  {
    return OPP_BACK;
  }

  return OPP_NONE;
}


// ============================================================================
// IR MOTION CLASSIFICATION
// ============================================================================

OpponentMotion classifyIRMotion(unsigned long now)
{
  bool fl = FL();
  bool fr = FR();
  bool l = L();
  bool r = R();
  bool bl = BL();

  bool patternChanged =
    (fl != previousFL) ||
    (fr != previousFR) ||
    (l  != previousL) ||
    (r  != previousR) ||
    (bl != previousBL);

  if (patternChanged)
  {
    lastPatternChangeTime = now;
  }

  bool currentFront = fl || fr;
  bool previousFront = previousFL || previousFR;

  bool currentSide = l || r;
  bool previousSide = previousL || previousR;

  bool currentBack = bl;
  bool previousBack = previousBL;

  if (previousBack && currentFront)
  {
    return MOTION_APPROACHING_FAST;
  }

  if (previousSide && currentFront)
  {
    return MOTION_APPROACHING_SLOW;
  }

  if (
    (previousFL && r) ||
    (previousFR && l) ||
    (previousL && fr) ||
    (previousR && fl)
  )
  {
    return MOTION_CROSSING;
  }

  if (currentFront && !previousFront)
  {
    return MOTION_APPROACHING_SLOW;
  }

  if (opponentDetected())
  {
    unsigned long stableTime =
      now - lastPatternChangeTime;

    if (stableTime >= IR_SLOW_THRESHOLD)
    {
      return MOTION_STATIONARY;
    }

    return MOTION_APPROACHING_SLOW;
  }

  return MOTION_UNKNOWN;
}


// ============================================================================
// OPPONENT MOTION UPDATE
// ============================================================================

void classifyOpponentMotion()
{
  unsigned long now = millis();

  if (ultrasonicValid)
  {
    if (closingSpeed >= FAST_CLOSING_SPEED)
    {
      opponentMotion =
        MOTION_APPROACHING_FAST;

      return;
    }

    if (closingSpeed >= SLOW_CLOSING_SPEED)
    {
      opponentMotion =
        MOTION_APPROACHING_SLOW;

      return;
    }

    if (closingSpeed <= MOVING_AWAY_SPEED)
    {
      opponentMotion =
        MOTION_MOVING_AWAY;

      return;
    }
  }

  opponentMotion =
    classifyIRMotion(now);
}


// ============================================================================
// OPPONENT TRACKING
// ============================================================================

void updateOpponentTracking()
{
  unsigned long now = millis();

  bool currentFL = FL();
  bool currentFR = FR();
  bool currentL = L();
  bool currentR = R();
  bool currentBL = BL();

  bool patternChanged =
    (currentFL != previousFL) ||
    (currentFR != previousFR) ||
    (currentL != previousL) ||
    (currentR != previousR) ||
    (currentBL != previousBL);

  if (patternChanged)
  {
    lastPatternChangeTime = now;
  }

  bool detected =
    opponentDetected();

  if (detected)
  {
    lastOpponentSeen = now;

    OpponentDirection currentDirection =
      getOpponentDirection();

    if (currentDirection != OPP_NONE)
    {
      opponentDirection =
        currentDirection;

      lastOpponentDirection =
        currentDirection;
    }
  }
  else
  {
    opponentDirection = OPP_NONE;
  }

  if (detected)
  {
    classifyOpponentMotion();
  }
  else
  {
    opponentMotion = MOTION_UNKNOWN;
  }

  previousFL = currentFL;
  previousFR = currentFR;
  previousL = currentL;
  previousR = currentR;
  previousBL = currentBL;

  lastTrackingUpdate = now;
}


// ============================================================================
// START ESCAPE
// ============================================================================

void startFrontEscape(
  unsigned long duration,
  bool turnLeft
)
{
  escapeState =
    ESCAPE_BACK_FROM_FRONT;

  escapeStarted =
    millis();

  escapeDuration =
    duration;

  escapeTurnLeft =
    turnLeft;
}


void startRearEscape(
  unsigned long duration,
  bool turnLeft
)
{
  escapeState =
    ESCAPE_FORWARD_FROM_BACK;

  escapeStarted =
    millis();

  escapeDuration =
    duration;

  escapeTurnLeft =
    turnLeft;
}


// ============================================================================
// BOUNDARY ESCAPE
// ============================================================================

bool handleBoundary()
{
  bool fl = frontLeftLine();
  bool fr = frontRightLine();
  bool back = backLine();

  // --------------------------------------------------------------------------
  // ALREADY ESCAPING
  // --------------------------------------------------------------------------

  if (escapeState != ESCAPE_NONE)
  {
    unsigned long elapsed =
      millis() - escapeStarted;

    // While backing from front, watch rear
    if (escapeState == ESCAPE_BACK_FROM_FRONT)
    {
      if (back)
      {
        startRearEscape(250, false);
        return true;
      }
    }

    // While moving forward from rear, watch front
    if (escapeState == ESCAPE_FORWARD_FROM_BACK)
    {
      if (fl || fr)
      {
        if (fl && !fr)
        {
          startFrontEscape(250, false);
        }
        else if (fr && !fl)
        {
          startFrontEscape(250, true);
        }
        else
        {
          startFrontEscape(300, true);
        }

        return true;
      }
    }

    // Current phase finished
    if (elapsed >= escapeDuration)
    {
      if (escapeState == ESCAPE_BACK_FROM_FRONT)
      {
        escapeState =
          ESCAPE_TURN_FROM_FRONT;

        escapeStarted =
          millis();

        escapeDuration = 300;

        return true;
      }

      if (escapeState == ESCAPE_FORWARD_FROM_BACK)
      {
        escapeState =
          ESCAPE_TURN_FROM_BACK;

        escapeStarted =
          millis();

        escapeDuration = 300;

        return true;
      }

      escapeState = ESCAPE_NONE;

      stopMotors();

      return true;
    }

    // Execute current phase
    switch (escapeState)
    {
      case ESCAPE_BACK_FROM_FRONT:
        backward(ESCAPE_SPEED);
        break;

      case ESCAPE_TURN_FROM_FRONT:

        if (escapeTurnLeft)
        {
          rotateLeft(ESCAPE_SPEED);
        }
        else
        {
          rotateRight(ESCAPE_SPEED);
        }

        break;

      case ESCAPE_FORWARD_FROM_BACK:
        forward(ESCAPE_SPEED);
        break;

      case ESCAPE_TURN_FROM_BACK:

        if (escapeTurnLeft)
        {
          rotateLeft(ESCAPE_SPEED);
        }
        else
        {
          rotateRight(ESCAPE_SPEED);
        }

        break;

      default:
        stopMotors();
        break;
    }

    return true;
  }


  // --------------------------------------------------------------------------
  // NEW FRONT ESCAPE
  // --------------------------------------------------------------------------

  if (fl && fr)
  {
    startFrontEscape(400, true);
    return true;
  }

  if (fl)
  {
    startFrontEscape(300, false);
    return true;
  }

  if (fr)
  {
    startFrontEscape(300, true);
    return true;
  }


  // --------------------------------------------------------------------------
  // NEW REAR ESCAPE
  // --------------------------------------------------------------------------

  if (back)
  {
    startRearEscape(300, false);
    return true;
  }

  return false;
}


// ============================================================================
// OPENING MOVE
// ============================================================================

void openingMove()
{
  unsigned long elapsed =
    millis() - openingMoveStarted;

  if (elapsed < OPENING_MOVE_DURATION)
  {
    forward(ATTACK_SPEED);
    return;
  }

  openingMoveActive = false;
}


// ============================================================================
// LAST-SEEN PURSUIT
// ============================================================================

bool handleLastSeen()
{
  if (lastOpponentSeen == 0)
  {
    return false;
  }

  unsigned long now = millis();

  unsigned long timeSinceSeen =
    now - lastOpponentSeen;

  if (timeSinceSeen > LAST_SEEN_MEMORY)
  {
    lastSeenTurnActive = false;
    return false;
  }

  if (lastOpponentDirection == OPP_FRONT)
  {
    forward(TRACK_SPEED);
    return true;
  }

  if (lastOpponentDirection == OPP_LEFT)
  {
    if (!lastSeenTurnActive)
    {
      lastSeenTurnActive = true;
      lastSeenTurnStarted = now;
    }

    if (
      now - lastSeenTurnStarted <
      LAST_SEEN_TURN_TIME
    )
    {
      rotateLeft(TURN_SPEED);
      return true;
    }
  }

  if (lastOpponentDirection == OPP_RIGHT)
  {
    if (!lastSeenTurnActive)
    {
      lastSeenTurnActive = true;
      lastSeenTurnStarted = now;
    }

    if (
      now - lastSeenTurnStarted <
      LAST_SEEN_TURN_TIME
    )
    {
      rotateRight(TURN_SPEED);
      return true;
    }
  }

  if (lastOpponentDirection == OPP_BACK)
  {
    if (!lastSeenTurnActive)
    {
      lastSeenTurnActive = true;
      lastSeenTurnStarted = now;
    }

    if (
      now - lastSeenTurnStarted <
      LAST_SEEN_TURN_TIME
    )
    {
      rotateLeft(TURN_SPEED);
      return true;
    }
  }

  lastSeenTurnActive = false;

  return false;
}


// ============================================================================
// SEARCH
// ============================================================================

void searchOpponent()
{
  unsigned long now = millis();

  if (handleLastSeen())
  {
    return;
  }

  if (
    now - searchDirectionChanged >
    650
  )
  {
    searchDirectionLeft =
      !searchDirectionLeft;

    searchDirectionChanged =
      now;
  }

  if (searchDirectionLeft)
  {
    rotateLeft(SEARCH_SPEED);
  }
  else
  {
    rotateRight(SEARCH_SPEED);
  }
}


// ============================================================================
// ATTACK
// ============================================================================

void attackOpponent()
{
  switch (opponentDirection)
  {
    case OPP_LEFT:

      drive(
        60,
        ATTACK_SPEED
      );

      break;

    case OPP_RIGHT:

      drive(
        ATTACK_SPEED,
        60
      );

      break;

    case OPP_FRONT:

      forward(ATTACK_SPEED);

      break;

    case OPP_BACK:

      rotateLeft(TURN_SPEED);

      break;

    default:

      forward(ATTACK_SPEED);

      break;
  }
}


// ============================================================================
// DEFENSIVE ATTACK
// ============================================================================

void defensiveAttack()
{
  switch (opponentDirection)
  {
    case OPP_LEFT:

      drive(
        DEFENSE_SPEED / 2,
        DEFENSE_SPEED
      );

      break;

    case OPP_RIGHT:

      drive(
        DEFENSE_SPEED,
        DEFENSE_SPEED / 2
      );

      break;

    case OPP_FRONT:

      forward(DEFENSE_SPEED);

      break;

    default:

      forward(DEFENSE_SPEED);

      break;
  }
}


// ============================================================================
// FAST APPROACH
// ============================================================================

void trapFastOpponent()
{
  switch (opponentDirection)
  {
    case OPP_LEFT:

      drive(
        -INTERCEPT_SPEED,
        INTERCEPT_SPEED
      );

      break;

    case OPP_RIGHT:

      drive(
        INTERCEPT_SPEED,
        -INTERCEPT_SPEED
      );

      break;

    case OPP_FRONT:

      forward(INTERCEPT_SPEED);

      break;

    default:

      forward(INTERCEPT_SPEED);

      break;
  }
}


// ============================================================================
// PURSUIT
// ============================================================================

void pursueOpponent()
{
  switch (opponentDirection)
  {
    case OPP_LEFT:

      drive(
        TRACK_SPEED / 2,
        ATTACK_SPEED
      );

      break;

    case OPP_RIGHT:

      drive(
        ATTACK_SPEED,
        TRACK_SPEED / 2
      );

      break;

    case OPP_FRONT:

      forward(ATTACK_SPEED);

      break;

    default:

      forward(TRACK_SPEED);

      break;
  }
}


// ============================================================================
// CROSSING INTERCEPT
// ============================================================================

void interceptCrossingOpponent()
{
  if (opponentDirection == OPP_LEFT)
  {
    rotateLeft(TURN_SPEED);
  }
  else if (opponentDirection == OPP_RIGHT)
  {
    rotateRight(TURN_SPEED);
  }
  else
  {
    forward(ATTACK_SPEED);
  }
}


// ============================================================================
// MAIN CONTROLLER
// ============================================================================

void proactiveController()
{
  // Update ultrasonic
  updateUltrasonic();

  // Update IR tracking
  updateOpponentTracking();


  // Boundary has absolute priority
  if (handleBoundary())
  {
    return;
  }


  // Opening move
  if (openingMoveActive)
  {
    openingMove();
    return;
  }


  // No opponent
  if (!opponentDetected())
  {
    searchOpponent();
    return;
  }


  // Opponent detected
  switch (opponentMotion)
  {
    case MOTION_APPROACHING_FAST:

      trapFastOpponent();

      break;

    case MOTION_APPROACHING_SLOW:

      defensiveAttack();

      break;

    case MOTION_CROSSING:

      interceptCrossingOpponent();

      break;

    case MOTION_MOVING_AWAY:

      pursueOpponent();

      break;

    case MOTION_STATIONARY:

      attackOpponent();

      break;

    default:

      attackOpponent();

      break;
  }
}


// ============================================================================
// STARTUP ULTRASONIC MONITOR
// ============================================================================
//
// IMPORTANT:
// The ultrasonic sensor works BEFORE the robot starts.
//
// The robot remains stopped.
// We continuously update the ultrasonic distance while waiting.
//
// This means that when the button is pressed and the 5-second countdown
// starts, the ultrasonic sensor is already operating.
//

void startupUltrasonicUpdate()
{
  updateUltrasonic();

  // Optional serial display
  static unsigned long lastPrint = 0;

  unsigned long now = millis();

  if (now - lastPrint >= 250)
  {
    lastPrint = now;

    if (ultrasonicValid)
    {
      Serial.print("Ultrasonic: ");
      Serial.print(ultrasonicDistance);
      Serial.println(" cm");
    }
    else
    {
      Serial.println("Ultrasonic: NO TARGET");
    }
  }
}


// ============================================================================
// WAIT FOR START
// ============================================================================
//
// Button:
// D1 -> button -> GND
//
// INPUT_PULLUP:
// HIGH = released
// LOW  = pressed
//
// Ultrasonic continues operating during this entire function.
//

void waitForStart()
{
  stopMotors();

  Serial.println();
  Serial.println("==============================");
  Serial.println("BESOMI SUMO ROBOT READY");
  Serial.println("ULTRASONIC ACTIVE");
  Serial.println("PRESS START BUTTON");
  Serial.println("==============================");


  // --------------------------------------------------------------------------
  // WAIT FOR BUTTON
  // --------------------------------------------------------------------------

  while (
    digitalRead(START_BUTTON_PIN) == HIGH
  )
  {
    stopMotors();

    // Ultrasonic continues running
    startupUltrasonicUpdate();

    delay(5);
  }


  // --------------------------------------------------------------------------
  // DEBOUNCE
  // --------------------------------------------------------------------------

  delay(50);


  // --------------------------------------------------------------------------
  // WAIT FOR BUTTON RELEASE
  // --------------------------------------------------------------------------

  while (
    digitalRead(START_BUTTON_PIN) == LOW
  )
  {
    stopMotors();

    // Ultrasonic continues running
    startupUltrasonicUpdate();

    delay(5);
  }


  // --------------------------------------------------------------------------
  // 5 SECOND COUNTDOWN
  // --------------------------------------------------------------------------

  Serial.println();
  Serial.println("STARTING IN:");

  for (int i = 5; i >= 1; i--)
  {
    Serial.print(i);
    Serial.println("...");

    // Keep ultrasonic running during countdown
    unsigned long countdownStart =
      millis();

    while (
      millis() - countdownStart <
      1000
    )
    {
      stopMotors();

      startupUltrasonicUpdate();

      delay(5);
    }
  }


  // --------------------------------------------------------------------------
  // GO
  // --------------------------------------------------------------------------

  Serial.println("GO!");


  // --------------------------------------------------------------------------
  // START ROBOT
  // --------------------------------------------------------------------------

  robotStarted = true;

  openingMoveActive = true;

  openingMoveStarted = millis();

  searchDirectionChanged = millis();


  // --------------------------------------------------------------------------
  // IMPORTANT
  // --------------------------------------------------------------------------
  // The ultrasonic reading collected immediately before GO remains available.
  // The main controller will continue updating it every 30 ms.
}


// ============================================================================
// SETUP
// ============================================================================

void setup()
{
  Serial.begin(115200);


  // --------------------------------------------------------------------------
  // MOTOR PINS
  // --------------------------------------------------------------------------

  pinMode(M1_DIR, OUTPUT);
  pinMode(M1_PWM, OUTPUT);

  pinMode(M2_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);

  pinMode(M3_DIR, OUTPUT);
  pinMode(M3_PWM, OUTPUT);

  pinMode(M4_DIR, OUTPUT);
  pinMode(M4_PWM, OUTPUT);


  // --------------------------------------------------------------------------
  // IR SENSORS
  // --------------------------------------------------------------------------

  pinMode(
    IR_FRONT_LEFT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    IR_FRONT_RIGHT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    IR_BACK_PIN,
    INPUT_PULLUP
  );


  // --------------------------------------------------------------------------
  // LINE SENSORS
  // --------------------------------------------------------------------------

  pinMode(
    LINE_FRONT_LEFT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    LINE_FRONT_RIGHT_PIN,
    INPUT_PULLUP
  );

  pinMode(
    LINE_BACK_PIN,
    INPUT_PULLUP
  );


  // --------------------------------------------------------------------------
  // START BUTTON
  // --------------------------------------------------------------------------

  pinMode(
    START_BUTTON_PIN,
    INPUT_PULLUP
  );


  // --------------------------------------------------------------------------
  // ULTRASONIC
  // --------------------------------------------------------------------------

  pinMode(
    ULTRASONIC_TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ULTRASONIC_ECHO_PIN,
    INPUT
  );

  digitalWrite(
    ULTRASONIC_TRIG_PIN,
    LOW
  );


  // --------------------------------------------------------------------------
  // INITIAL MOTOR STATE
  // --------------------------------------------------------------------------

  stopMotors();

  delay(500);


  Serial.println();
  Serial.println("SYSTEM INITIALIZED");


  // --------------------------------------------------------------------------
  // WAIT FOR START
  // --------------------------------------------------------------------------

  waitForStart();
}


// ============================================================================
// LOOP
// ============================================================================

void loop()
{
  if (!robotStarted)
  {
    stopMotors();

    // Even if robotStarted somehow becomes false,
    // ultrasonic remains active.
    updateUltrasonic();

    return;
  }


  proactiveController();
}