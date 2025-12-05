// -------------------- PIN CONFIG --------------------
const byte LIGHTPIN = A2;
const byte SWITCH_PIN = 6;
const byte RED = 3;
const byte GREEN = 4;

// -------------------- TIME CONSTANTS --------------------
const unsigned long MINUTE = 60000UL;

// -------------------- STATE VARIABLES --------------------
bool nightRunning = false;
bool waitingForDay = false;
byte cycleNumber = 0;

unsigned long stageStartTime = 0;
unsigned long stageDuration = 0;

unsigned long lastBatteryBlink = 0;
bool batteryLedState = false;

bool lampOn = false;

void setup() {
  Serial.begin(9600);

  pinMode(LIGHTPIN, INPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);

  digitalWrite(SWITCH_PIN, LOW);

  randomSeed(analogRead(A1));
}

void loop() {

  StanBateria();   // check battery each loop

  int LIGHT = readLight();
  //Serial.println(LIGHT); only for debugging

  // -------------------- RESET WHEN DAYLIGHT RETURNS --------------------
  if (waitingForDay && LIGHT > 300) {
    waitingForDay = false;
    cycleNumber = 0;
    nightRunning = false;
    Serial.println("System reset — new day detected.");
  }

  // If night sequence is completed, do nothing until day returns
  if (waitingForDay) return;

  // -------------------- START NIGHT SEQUENCE --------------------
  if (!nightRunning && LIGHT < 25) {
    nightRunning = true;
    cycleNumber = 0;
    startLampOnCycle();
  }

  // -------------------- RUN CURRENT NIGHT CYCLE --------------------
  if (nightRunning) {
    handleCycleStateMachine();
  }
}

// =====================================================================
// STATE MACHINE FOR LAMP TIMING
// =====================================================================

void handleCycleStateMachine() {

  unsigned long now = millis();

  if (now - stageStartTime >= stageDuration) {

    if (lampOn) {
      // Lamp was ON → now turn it off and start 40 min OFF
      digitalWrite(SWITCH_PIN, LOW);
      lampOn = false;
      Serial.println("Lamp OFF (40 min)");
      stageDuration = 40UL * MINUTE;
      stageStartTime = now;

    } else {
      // OFF finished → go to next cycle
      cycleNumber++;

      if (cycleNumber >= 5) {
        Serial.println("All 5 cycles completed. Waiting for daylight.");
        nightRunning = false;
        waitingForDay = true;
      } else {
        // Start next ON cycle
        startLampOnCycle();
      }
    }
  }
}

void startLampOnCycle() {

  unsigned long randMinutes = random(10, 30);

  Serial.print("Cycle ");
  Serial.print(cycleNumber + 1);
  Serial.print(" started. Lamp ON for ");
  Serial.print(randMinutes);
  Serial.println(" minutes.");

  digitalWrite(SWITCH_PIN, HIGH);
  lampOn = true;

  stageStartTime = millis();
  stageDuration = randMinutes * MINUTE;
}

// =====================================================================
// READ LIGHT
// =====================================================================
int readLight() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(LIGHTPIN);
    delay(5);
  }
  return sum / 10;
}

// =====================================================================
// BATTERY CHECK
// =====================================================================
void StanBateria() {

  static bool lowBattery = false;

  double odczyt = analogRead(A0) * (5.0 / 1024.0);

  // Determine battery condition
  lowBattery = (odczyt < 3.00);

  unsigned long now = millis();

  // Time to toggle LED?
  if (now - lastBatteryBlink >= 1000) {   // 1-second interval
    lastBatteryBlink = now;

    // Turn LED on for a short blink
    if (lowBattery) {
      digitalWrite(RED, HIGH);
      delay(50);                     // Short LED pulse (50 ms)
      digitalWrite(RED, LOW);
    } else {
      digitalWrite(GREEN, HIGH);
      delay(50);                     // Short LED pulse (50 ms)
      digitalWrite(GREEN, LOW);
    }
  }
}
