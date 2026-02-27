/*******************************************************
 * Chanukah Project – Team 7
 *******************************************************/

#define IR_USE_AVR_TIMER2
#include <IRremote.hpp>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/pgmspace.h>

struct Note3 { uint16_t f, d, g; }; //represents a buzzer note
struct DateCandle { char d[7]; uint8_t c; }; //maps a DDMMYY date string to the number of Chanukah candles

//pin definitions
const uint8_t SOUND_PIN   = 2;
const uint8_t TEMPERATURE_PIN = 3;
const uint8_t DREIDEL_BUTTON_PIN = 4;
const uint8_t DREIDEL_SERVO_PIN  = 6;
const uint8_t IR_RECEIVE_PIN = 7;
const uint8_t BUZZER_PIN     = 8;
const uint8_t CHILD_SERVO_PIN = 9;
const uint8_t LED_PINS[4] = {10, 11, 12, 13};

// LCD size
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Blessing
const char BLESSING[] PROGMEM =
  "Baruch Atah Adonai, Eloheinu Melech Haolam, asher kidshanu b'mitzvotav v'tzivanu l'hadlik ner Chanukah.";
const unsigned long BLESSING_PAGE_MS  = 650; // Blessing scroll interval
const unsigned long BLESSING_TOTAL_MS = 5500;

// Read single character from BLESSING
static inline char pgmChar(const char* p, uint16_t i)
{
  return (char)pgm_read_byte_near(p + i);
}

//// Get length
static inline uint16_t pgmStrLen(const char* p)
{
  return (uint16_t)strlen_P(p);
}


// LCD States
enum DisplayMode
{
  MODE_WAIT_PRESENCE,
  MODE_IDLE_PROMPT_DATE,
  MODE_BLESSING_RUNNING,
  MODE_COUNT_STICKY,
  MODE_OVERHEAT,
  MODE_NOT_CHANUKAH
};

DisplayMode displayMode = MODE_WAIT_PRESENCE;

// Definition of blessing variables for the LCD
unsigned long blessingStartMs = 0;
unsigned long lastBlessingPageMs = 0;
int blessingIndex = 0;
bool blessingTimed = false; // false=loop forever, true=stop after total and show count

// Sound sensor state
bool systemArmed = false;
bool soundPrev = LOW;

// Date to candeles map for the next 3 years
const DateCandle CHANUKAH_MAP[] PROGMEM = {
  {"141225",1},{"041226",1},{"241227",1},
  {"151225",2},{"051226",2},{"251227",2},
  {"161225",3},{"061226",3},{"261227",3},
  {"171225",4},{"071226",4},{"271227",4},
  {"181225",5},{"081226",5},{"281227",5},
  {"191225",6},{"091226",6},{"291227",6},
  {"201225",7},{"101226",7},{"301227",7},
  {"211225",8},{"111226",8},{"311227",8}
};
const uint8_t MAP_SIZE = sizeof(CHANUKAH_MAP) / sizeof(CHANUKAH_MAP[0]);

// Date to candles
uint8_t getCandlesForDate(const char* d6)
{
  DateCandle tmp;
  for (uint8_t i = 0; i < MAP_SIZE; i++) {
    memcpy_P(&tmp, &CHANUKAH_MAP[i], sizeof(DateCandle));
    if (strcmp(d6, tmp.d) == 0) return tmp.c;
  }
  return 0;
}


// MENORAH Charlieplex
const int LED_HI[9] = {0,1,0,2,0,3,1,2,1};
const int LED_LO[9] = {1,0,2,0,3,0,2,1,3};

volatile uint8_t candlesOn = 0;
volatile bool shamashOn = false;

// Disable all Charlieplex pins
void allOffPins()
{
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(LED_PINS[i], INPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
}

// Turn on one Charlieplex LED and release the previous one
void driveLedFast(uint8_t idx)
{
  static int8_t lastHi = -1;
  static int8_t lastLo = -1;

  int8_t hi = LED_HI[idx];
  int8_t lo = LED_LO[idx];

  if (hi == lastHi && lo == lastLo) return;

  if (lastHi >= 0) pinMode(LED_PINS[lastHi], INPUT);
  if (lastLo >= 0) pinMode(LED_PINS[lastLo], INPUT);

  pinMode(LED_PINS[hi], OUTPUT);
  pinMode(LED_PINS[lo], OUTPUT);
  digitalWrite(LED_PINS[hi], HIGH);
  digitalWrite(LED_PINS[lo], LOW);

  lastHi = hi;
  lastLo = lo;
}

// Overheat control
bool overheat = false;
const float OVERHEAT_ON  = 45.0;
const float OVERHEAT_OFF = 43.0;

void scanMenorah()
{
  // Disable all LEDs during overheat condition
  if (overheat)
  {
    for (uint8_t i=0;i<4;i++) 
    {
      pinMode(LED_PINS[i], INPUT);
    }
    return;
  }

  // Timing control for multiplexing
  static unsigned long lastUs = 0;
  static uint8_t phase = 0;
  const unsigned long STEP_US = 450;

  unsigned long nowUs = micros();
  if (nowUs - lastUs < STEP_US) return;
  lastUs = nowUs;

  // Total LEDs to scan (candles + shamash)
  uint8_t total = candlesOn;
  if (shamashOn)
  {
    total += 1;
  }
  if (total == 0)
  {
    for (uint8_t i=0;i<4;i++) 
    {
      pinMode(LED_PINS[i], INPUT);
    }
    return;
  }

  if (phase >= total)
  {
    phase = 0;
  }

// Select shamash or candle for this scan phase
  uint8_t idxToDrive;
  if (shamashOn)
  {
    if (phase == 0) 
    {
      idxToDrive = 8;          // shamash
    } 
    else 
    {
      idxToDrive = phase - 1; // candles
    }
  }
  else
  {
    idxToDrive = phase;       // candles only
  }

  // Drive current LED and advance scan phase
  driveLedFast(idxToDrive);
  phase++;
}

void clearMenorah()
{
  candlesOn = 0;
  shamashOn = false;
  allOffPins();
}

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

unsigned long lastOverheatLcdMs = 0;
const unsigned long OVERHEAT_LCD_PERIOD_MS = 300;
float lastValidTempC = NAN;   // keep last good reading
unsigned long lastTempMs = 0;
const unsigned long TEMP_PERIOD_MS = 800;

float readTempC() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}

// LCD helpers
void showWaitPresence() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Waiting noise");
  lcd.setCursor(0,1); lcd.print("to start...");
}

void showEnterDatePrompt() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Enter DDMMYY");
  lcd.setCursor(0,1); lcd.print("then Enter");
}

void showNotChanukah(const char* s6) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Not Chanukah");
  lcd.setCursor(0,1); lcd.print(s6);
}

void showCountSticky(uint8_t n) {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Chanukah");
  lcd.setCursor(0,1); lcd.print("Candles: ");
  lcd.print(n);
}

void showBlessingPage(int idx)
{
  uint16_t len = pgmStrLen(BLESSING);
  char r0[17], r1[17];

  for (int i = 0; i < 16; i++)
  {
    r0[i] = pgmChar(BLESSING, (idx + i) % len);
    r1[i] = pgmChar(BLESSING, (idx + i + 16) % len);
  }
  r0[16] = '\0'; r1[16] = '\0';

  lcd.clear();
  lcd.setCursor(0,0); lcd.print(r0);
  lcd.setCursor(0,1); lcd.print(r1);
}

void startBlessing(bool timed)
{
  displayMode = MODE_BLESSING_RUNNING;
  blessingTimed = timed;
  blessingStartMs = millis();
  lastBlessingPageMs = 0;
  blessingIndex = 0;
  showBlessingPage(blessingIndex);

  Serial.print("LCD: Blessing start (timed="); Serial.print(timed); Serial.println(")");
}

void updateLcdStateMachine() 
{
  unsigned long now = millis();

    // If blessing is timed and total duration elapsed = finish blessing
  if (displayMode == MODE_BLESSING_RUNNING) {
    if (blessingTimed && (now - blessingStartMs >= BLESSING_TOTAL_MS)) {
      displayMode = MODE_COUNT_STICKY;
      showCountSticky(candlesOn);
      Serial.println("LCD: Blessing done -> COUNT");
      return;
    }

    // Scroll blessing text periodically
    if (now - lastBlessingPageMs >= BLESSING_PAGE_MS) {
      lastBlessingPageMs = now;
      blessingIndex += 8;
      showBlessingPage(blessingIndex);
    }
  }
}

// SERVO: Child hand motion
Servo childServo;
int servoPos = 0;
int servoTarget = 0;
bool servoMoving = false;

unsigned long lastServoStepMs = 0;
const unsigned long SERVO_STEP_MS = 15;
const int SERVO_STEP_DEG = 2;

// Request servo movement
void startServoMoveTo(int target)
{
  servoTarget = constrain(target, 0, 180); // Request servo movement to a valid angle (0–180°)
  servoMoving = true;
}

// Non blocking gradual servo movement
void updateServoMotion()
{
  if (overheat)
  {
    return;
  }
  if (!servoMoving)
  {
    return;
  }
  unsigned long now = millis();
  if (now - lastServoStepMs < SERVO_STEP_MS)
  {
    return;
  } 
  lastServoStepMs = now;

  if (servoPos < servoTarget) 
  {
    servoPos += SERVO_STEP_DEG;
    if (servoPos >= servoTarget)
    { 
      servoPos = servoTarget; 
      servoMoving = false; 
    }
  } 
  else if (servoPos > servoTarget)
  {
    servoPos -= SERVO_STEP_DEG;
    if (servoPos <= servoTarget)
    { 
      servoPos = servoTarget; 
      servoMoving = false; 
    }
  } 
  else 
  {
    servoMoving = false;
  }
  childServo.write(servoPos);
}

// Dreidel servo + button
Servo dreidel;

const int DREIDEL_STOP = 90;
const int DREIDEL_SPIN = 180;
const unsigned long DREIDEL_SPIN_MS     = 1200;
const unsigned long DREIDEL_DEBOUNCE_MS = 30;

bool dreidelSpinning = false;
unsigned long dreidelSpinStart = 0;

bool dreidelLastStable = HIGH;
bool dreidelLastReading = HIGH;
unsigned long dreidelLastChange = 0;

void updateDreidel()
{
  //stop dreidel during overheat
  if (overheat) 
  {
    dreidel.write(DREIDEL_STOP);
    dreidelSpinning = false;
    return;
  }

  unsigned long now = millis();
  bool reading = digitalRead(DREIDEL_BUTTON_PIN);

  // Detect button state change
  if (reading != dreidelLastReading)
  {
    dreidelLastReading = reading;
    dreidelLastChange = now;
  }

  // Check if button state is stable
  if (now - dreidelLastChange > DREIDEL_DEBOUNCE_MS) 
  {
    // Detect a new button press
    if (!dreidelSpinning && dreidelLastStable == HIGH && reading == LOW) 
    {
      dreidelSpinning = true;
      dreidelSpinStart = now;
      dreidel.write(DREIDEL_SPIN);
      Serial.println("DREIDEL: Spin!");
    }
    dreidelLastStable = reading;
  }
  
  // Stop dreidel after fixed spin duration
  if (dreidelSpinning && (now - dreidelSpinStart >= DREIDEL_SPIN_MS)) 
  {
    dreidelSpinning = false;
    dreidel.write(DREIDEL_STOP);
    Serial.println("DREIDEL: Stop.");
  }
}

// BUZZER
// Read a single note from song array (base)
Note3 readNotePGM(const Note3* base, uint16_t i) {
  Note3 n;
  memcpy_P(&n, base + i, sizeof(Note3));
  return n;
}

// Songs
const Note3 song1[] PROGMEM = {
  {349,124,110},{349,135,132},{392,456,59},{415,149,100},{415,135,115},
  {392,415,68},{523,120,138},{523,127,131},{392,90,14},{349,101,33},
  {415,87,176},{415,135,125},{392,489,28},{349,129,115},{349,136,105},
  {392,428,61},{415,149,90},{415,147,117},{466,441,84},{523,109,139},
  {523,102,147},{523,209,23},{262,203,33},{349,118,136},{349,104,165},
  {349,329,165},{466,110,128},{466,120,129},{554,146,96},{523,153,113},
  {523,130,119},{392,98,23},
};

const Note3 song2[] PROGMEM = {
  {466,618,82},{523,741,12},{349,673,18},{392,299,37},{415,342,52},
  {392,674,27},{311,879,149},{311,323,36},{466,643,67},{466,652,62},
  {523,674,45},{587,634,77},{622,1396,15},{466,586,73},{415,342,10},
  {415,357,4},{392,1033,36},{415,330,12},{466,936,125},{466,338,42},
  {392,365,21},{415,975,83},{415,330,49},{392,637,31},{415,372,18},
};

const uint16_t SONG1_LEN = sizeof(song1)/sizeof(song1[0]);
const uint16_t SONG2_LEN = sizeof(song2)/sizeof(song2[0]);

const Note3* currentSong = nullptr;
uint16_t currentLen = 0;
uint16_t songIdx = 0;
bool songPlaying = false;

unsigned long noteStartMs = 0;
bool notePhaseTone = true; // true=tone(d), false=gap(g)
Note3 curNote;

// Buzzer output timing and state control
bool buzzerLevel = false;
unsigned long nextToggleUs = 0;
unsigned long halfPeriodUs = 0;

// Immediately stop buzzer
static inline void buzzerOff() 
{
  digitalWrite(BUZZER_PIN, LOW);
  buzzerLevel = false;
  halfPeriodUs = 0;
  nextToggleUs = 0;
}

// Set buzzer frequency
static inline void buzzerSetFreq(uint16_t f) 
{
  if (f == 0)  
  {
    buzzerOff(); 
    return; 
  }
  halfPeriodUs = 500000UL / (unsigned long)f;
  if (halfPeriodUs < 50)
  {
    halfPeriodUs = 50;
  } 
  nextToggleUs = micros() + halfPeriodUs;
}

// Start song
void startSong(const Note3* song, uint16_t len) 
{
  // Do not start playback during overheat
  if (overheat)
  {
    return;
  }
  currentSong = song;
  currentLen = len;
  songIdx = 0;
  songPlaying = (song != nullptr && len > 0);

  noteStartMs = 0;
  notePhaseTone = true;
  buzzerOff();

  Serial.print("song: start len="); Serial.println(len);
}

void stopSong() {
  songPlaying = false;
  currentSong = nullptr;
  currentLen = 0;
  buzzerOff();
  Serial.println("song: stop");
}

void updateSongPlayer() {
  if (overheat) 
  { 
    if (songPlaying)
    {
      stopSong();
    }  
    return; 
  }
  if (!songPlaying)
  {
    return;
  } 

  unsigned long nowMs = millis();

  // Initialize playback of the first note
  if (noteStartMs == 0) 
  {
    curNote = readNotePGM(currentSong, songIdx);
    noteStartMs = nowMs;
    notePhaseTone = true;

    buzzerSetFreq(curNote.f);
    return;
  }

  // Maintain buzzer output timing during note playback
  if (notePhaseTone && halfPeriodUs > 0) 
  {
    unsigned long nowUs = micros();
    while ((long)(nowUs - nextToggleUs) >= 0) 
    {
      buzzerLevel = !buzzerLevel;
      digitalWrite(BUZZER_PIN, buzzerLevel ? HIGH : LOW);
      nextToggleUs += halfPeriodUs;
    }
  }

  // advance note timing
  if (notePhaseTone) {
    if (nowMs - noteStartMs >= curNote.d) 
    {
      buzzerOff();
      noteStartMs = nowMs;
      notePhaseTone = false;
    }
  }
  else // Advance to next note or stop when song ends 
  {
    if (nowMs - noteStartMs >= curNote.g) 
    {
      songIdx++;
      if (songIdx >= currentLen) 
      { 
        stopSong(); 
        return; 
      }
      curNote = readNotePGM(currentSong, songIdx);
      noteStartMs = nowMs;
      notePhaseTone = true;
      buzzerSetFreq(curNote.f);
    }
  }
}

// IR: button 1=0x45, button 2=0x46
const uint8_t IR_NUM_1 = 0x45;
const uint8_t IR_NUM_2 = 0x46;

// Handle IR remote
void updateIR() {
  if (!systemArmed || overheat)
  {
    return;
  } 
  // Check if a new IR command was received
  if (IrReceiver.decode()) 
  {
    auto &d = IrReceiver.decodedIRData;
    if (d.protocol == NEC && !(d.flags & IRDATA_FLAGS_IS_REPEAT)) 
    {
      //button 1
      if (d.command == IR_NUM_1) 
      {
        Serial.println("IR: 1 > Song1");
        startSong(song1, SONG1_LEN);
      } 
      // button 2
      else if (d.command == IR_NUM_2) 
      {
        Serial.println("IR: 2 > Song2");
        startSong(song2, SONG2_LEN);
      }
    }
    IrReceiver.resume();
  }
}

// Overheat handling
void updateTemperature() 
{
  // Sample temperature
  unsigned long now = millis();
  if (now - lastTempMs < TEMP_PERIOD_MS)
  {
    return;
  } 
  lastTempMs = now;
  
  // Read temperature from temperature sensor
  float t = readTempC();

  if (t == 85.0 || t <= -127.0)
  {
    return;
  } 
  lastValidTempC = t;

  // Enter overheat mode when temperature exceeds upper threshold
  if (!overheat && t >= OVERHEAT_ON) 
  {
    // Immediately stop all active outputs and motion
    overheat = true;
    clearMenorah();
    stopSong();
    servoMoving = false;
    dreidel.write(DREIDEL_STOP);
    dreidelSpinning = false;

    // Display overheat warning with current temperature
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("OVERHEAT!");
    lcd.setCursor(0,1); lcd.print("T="); lcd.print(t, 1); lcd.print((char)223); lcd.print("C");

    Serial.print("OVERHEAT! T="); Serial.println(t);

    displayMode = MODE_OVERHEAT;
    lastOverheatLcdMs = now;   // start periodic temperature updates
  }

  // Exit overheat mode when temperature drops below lower threshold
  if (overheat && t <= OVERHEAT_OFF) 
  {
    overheat = false;
    Serial.print("Temp OK. T="); Serial.println(t);

    if (!systemArmed) 
    {
      displayMode = MODE_WAIT_PRESENCE;
      showWaitPresence();
    } 
    else 
    {
      displayMode = MODE_IDLE_PROMPT_DATE;
      showEnterDatePrompt();
    }
  }
}

// Periodically update temperature display during overheat state
void updateOverheatScreen() 
{
  if (!overheat) 
  {
    return;
  }
  unsigned long now = millis();
  if (now - lastOverheatLcdMs < OVERHEAT_LCD_PERIOD_MS)
  {
    return;
  } 
  lastOverheatLcdMs = now;

  // Display last valid temperature reading
  lcd.setCursor(0, 1);
  lcd.print("T=");
  if (!isnan(lastValidTempC)) 
  {
    lcd.print(lastValidTempC, 1);
  }
  else 
  {
    lcd.print("--.-");
  }
  lcd.print((char)223); // degree symbol
  lcd.print("C        "); // pad to clear rest of line
}


// Background runner 
void runBackgroundTasks() 
{
  scanMenorah();
  updateServoMotion();
  updateSongPlayer();
  updateTemperature();
  updateOverheatScreen();
  updateDreidel();
  updateIR();
  updateLcdStateMachine();
}

// Non-blocking delay that keeps background tasks running
void waitMsWithTasks(unsigned long ms) 
{
  unsigned long t0 = millis();
  while (millis() - t0 < ms) runBackgroundTasks();
}

// Hand + candle sync animation + blessing during lighting
const int HAND_REST_ANGLE  = 0;
const int HAND_LIGHT_ANGLE = 120;

const unsigned long HAND_GO_MS   = 550;
const unsigned long HAND_HOLD_MS = 250;
const unsigned long HAND_BACK_MS = 550;

// Animate hand movement for a single candle
void doHandForCandle(uint8_t candleIndex) 
{
  if (overheat) 
  {
    return;
  }
  // Move hand forward toward the candle
  Serial.print("LIGHT: candle "); 
  Serial.print(candleIndex); 
  Serial.println(" hand > front");
  startServoMoveTo(HAND_LIGHT_ANGLE);
  waitMsWithTasks(HAND_GO_MS);

  // Hold hand in lighting position
  Serial.println("LIGHT: hold");
  waitMsWithTasks(HAND_HOLD_MS);

  // Return hand to resting position
  Serial.println("LIGHT: hand->back");
  startServoMoveTo(HAND_REST_ANGLE);
  waitMsWithTasks(HAND_BACK_MS);
}

// Animate full candle lighting sequence up to the given number
void animateLightingTo(uint8_t n) 
{
  if (overheat) 
  {
    return;
  }
  // Limit number of candles to valid Chanukah range
  n = constrain(n, 1, 8);

  // blessing should run during lighting
  startBlessing(false);

  shamashOn = true;
  waitMsWithTasks(200);

  for (uint8_t i = 1; i <= n; i++)
  {
    doHandForCandle(i);
    candlesOn = i;
    Serial.print("ANIM: candlesOn="); 
    Serial.println(candlesOn);
    waitMsWithTasks(200);
  }
  startBlessing(true);
}

// Serial date input (DDMMYY)
char dateBuf[7] = {0};
uint8_t dateIdx = 0;

void processDateString(const char* s6) 
{
  if (!systemArmed || overheat) 
  {
    return;
  }
  if (strlen(s6) != 6) 
  {
    return;
  }

  Serial.print("DATE: got "); 
  Serial.println(s6);

  uint8_t c = getCandlesForDate(s6);
  Serial.print("DATE: map -> "); 
  Serial.println(c);

  if (c == 0) {
    clearMenorah();
    showNotChanukah(s6);
    displayMode = MODE_NOT_CHANUKAH;
    Serial.println("DATE: Not Chanukah");
    return;
  }

  clearMenorah();
  animateLightingTo(c);
}


// Read and validate date from Serial input
void readSerialDate() 
{
  if (!systemArmed || overheat) 
  {
    return;
  }
  
  // Process all available characters from Serial buffer
  while (Serial.available() > 0) 
  {
    char ch = (char)Serial.read();
    // Handle input termination characters
    if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') 
    {
      if (dateIdx == 6) 
      {
        dateBuf[6] = '\0';
        processDateString(dateBuf);
      } 
      else if (dateIdx > 0) 
      {
        Serial.println("DATE: need 6 digits DDMMYY");
      }
      dateIdx = 0; // Reset buffer for next input
      continue;
    }

    if (ch >= '0' && ch <= '9') {
      if (dateIdx < 6) dateBuf[dateIdx++] = ch;

      if (dateIdx == 6) 
      {
        dateBuf[6] = '\0';
        processDateString(dateBuf);
        dateIdx = 0;
      }
    }
  }
}

// Sound based system activation
void updateSoundLatch() 
{
  if (systemArmed || overheat) 
  {
    return;
  }
  bool soundNow = (digitalRead(SOUND_PIN) == HIGH);
  // Detect rising edge
  if (soundNow && !soundPrev) 
  { 
    systemArmed = true;
    displayMode = MODE_IDLE_PROMPT_DATE;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("System Active");

    waitMsWithTasks(650);

    showEnterDatePrompt();
    Serial.println("Enter date DDMMYY");
  }
  // Store previous sound state for edge detection
  soundPrev = soundNow;
}

// SETUP and LOOP
void setup() 
{
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  showWaitPresence();

  pinMode(SOUND_PIN, INPUT);
  delay(50);
  soundPrev = (digitalRead(SOUND_PIN) == HIGH);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  sensors.begin();

  childServo.attach(CHILD_SERVO_PIN);
  servoPos = HAND_REST_ANGLE;
  childServo.write(servoPos);

  pinMode(DREIDEL_BUTTON_PIN, INPUT_PULLUP);
  dreidel.attach(DREIDEL_SERVO_PIN);
  dreidel.write(DREIDEL_STOP);

  clearMenorah();

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  Serial.println("BOOT: READY.");
  Serial.println("BOOT: Waiting for sound pulse to arm...");
}

void loop() 
{
  runBackgroundTasks();

  if (!systemArmed) 
  {
    updateSoundLatch();
    return;
  }

  readSerialDate();
}
