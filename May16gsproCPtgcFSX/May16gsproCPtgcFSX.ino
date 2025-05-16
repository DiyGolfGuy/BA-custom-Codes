#include <Adafruit_BluefruitLE_SPI.h>

// ------ Bluefruit SPI Pins ------
#define BLUEFRUIT_SPI_CS   8
#define BLUEFRUIT_SPI_IRQ  7  
#define BLUEFRUIT_SPI_RST  4  

// Create the Bluefruit BLE object
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// ------------------- Key Pin Definitions -------------------
// Digital pins
#define BUTTON_M      2    // M key – special handling in Layout 1 (Courseplay)
#define BUTTON_P      3    
#define BUTTON_J      5    // Previously Layout 1 sent R; now changed to L.
#define BUTTON_U      6    
#define BUTTON_I      9    
#define BUTTON_K      10   // Left arrow – will use holdable-no-release behavior
#define BUTTON_A      11   // Right arrow – will use holdable-no-release behavior
#define BUTTON_C      12   // Mapping: I, S, Z, P for Layouts 0, 1, 2, 3 respectively.
#define BUTTON_V      13   // Down arrow

// Analog pins (used as digital inputs)
#define BUTTON_O      A0   // Mapping: Layout0 = A, Layout1 = P, Layout2 = V, Layout3 = Alt+H
#define BUTTON_UP     A1   // Mapping: Layout0 = C, Layout1 = V, Layout2 = E, Layout3 = Q  
#define BUTTON_DOWN   A2   // Mapping: Layout0 = V, Layout1 = F, Layout2 = Enter, Layout3 = F
#define BUTTON_LEFT   A3   // Mapping: Layout0 = J, Layout1 = D, Layout2 = Q, Layout3 = E
#define BUTTON_RIGHT  A4   // Mapping: Layout0 = O, Layout1 = O, Layout2 = H, Layout3 = H

// Layout Cycle Button (also on an analog pin)
#define LAYOUT_BUTTON A5  

// ------------------- Layout Settings -------------------
// Layout 0: GSpro  
// Layout 1: Courseplay  
// Layout 2: TGC2019  
// Layout 3: FXS Play  
int currentLayout = 0;

// ------------------- For Holdable Keys (left/right) -------------------
unsigned long lastRepeatTimeLeft = 0;
unsigned long lastRepeatTimeRight = 0;
bool leftHeld = false;
bool rightHeld = false;
const unsigned long leftRightRepeatDelay = 30;  // 30ms for smooth repetition

// ------------------- For Courseplay M Key -------------------
// For Layout 1 (Courseplay), M key is held for 4 seconds.
unsigned long mStartTime = 0;
bool mProcessing = false;

// ------------------- Function Prototypes -------------------
void processKeyPress(int pin, const char* cmd0, const char* cmd1,
                     const char* cmd2, const char* cmd3);
void sendKeyPress(const char* command);
void processHoldKeyNoRelease(int pin, const char* command, bool &held, 
                             unsigned long &lastTime, unsigned long interval);
void sendKeyPressHoldableNoRelease(const char* command);
void processHoldKey(int pin, const char* command, bool &held, 
                    unsigned long &lastTime, unsigned long interval);
void sendKeyPressHoldable(const char* command);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Multi-Key BLE Keyboard with Smooth Hold Keys");

  // Set digital pins as input with pull-ups.
  pinMode(BUTTON_M, INPUT_PULLUP);
  pinMode(BUTTON_P, INPUT_PULLUP);
  pinMode(BUTTON_J, INPUT_PULLUP);
  pinMode(BUTTON_U, INPUT_PULLUP);
  pinMode(BUTTON_I, INPUT_PULLUP);
  pinMode(BUTTON_K, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_V, INPUT_PULLUP);
  
  // Set analog pins as input with pull-ups.
  pinMode(BUTTON_O, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(LAYOUT_BUTTON, INPUT_PULLUP);

  // Initialize Bluefruit module.
  if (!ble.begin(true)) {
    Serial.println("Couldn't find Bluefruit, check wiring?");
    while (1);
  }
  ble.reset();
  ble.sendCommandCheckOK("AT+BleHIDEn=1");
  ble.sendCommandCheckOK("AT+GAPDEVNAME=BACUSTOMBOX");
  ble.sendCommandCheckOK("AT+GAPSTARTADV");
  Serial.println("Advertising as BACUSTOMBOX...");
}

void loop() {
  // --- Handle layout change using the Layout Button (A5) ---
  if (digitalRead(LAYOUT_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(LAYOUT_BUTTON) == LOW) {
      currentLayout = (currentLayout + 1) % 4;
      Serial.print("Layout changed to: ");
      Serial.println(currentLayout);
      mProcessing = false; // Reset M processing on layout change.
      while (digitalRead(LAYOUT_BUTTON) == LOW) { delay(10); }
      delay(50);
    }
  }
  
  // --- Process left/right arrow keys using no-release routine ---
  processHoldKeyNoRelease(BUTTON_K, "AT+BLEKEYBOARDCODE=00-00-50", leftHeld, lastRepeatTimeLeft, leftRightRepeatDelay);
  processHoldKeyNoRelease(BUTTON_A, "AT+BLEKEYBOARDCODE=00-00-4F", rightHeld, lastRepeatTimeRight, leftRightRepeatDelay);
  
  // --- Process the M key ---
  if (currentLayout == 1) {
    // In Courseplay, hold M for 4 seconds regardless.
    if (digitalRead(BUTTON_M) == LOW && !mProcessing) {
      mProcessing = true;
      mStartTime = millis();
      Serial.println("Courseplay M key pressed: HOLDING for 4 seconds");
      ble.sendCommandCheckOK("AT+BLEKEYBOARDCODE=00-00-10"); // M press
    }
    if (mProcessing && (millis() - mStartTime >= 4000)) {
      Serial.println("Courseplay M key released automatically after 4 seconds");
      ble.sendCommandCheckOK("AT+BLEKEYBOARDCODE=00-00-00"); // Release M
      mProcessing = false;
    }
  } else {
    // In other layouts, process M as a discrete press.
    processKeyPress(BUTTON_M,
                    "AT+BLEKEYBOARDCODE=01-00-10",   // Layout 0: Ctrl+M
                    "AT+BLEKEYBOARDCODE=00-00-10",   // Layout 1 not used here (handled above)
                    "AT+BLEKEYBOARDCODE=00-00-29",   // Layout 2: Esc
                    "AT+BLEKEYBOARDCODE=00-00-29");  // Layout 3: Esc
  }
  
  // --- Process remaining discrete keys ---
  processKeyPress(BUTTON_P,
                  "AT+BLEKEYBOARDCODE=00-00-1C",   // Layout 0: Y
                  "AT+BLEKEYBOARDCODE=00-00-0A",   // Layout 1: G
                  "AT+BLEKEYBOARDCODE=00-00-0A",   // Layout 2: G
                  "AT+BLEKEYBOARDCODE=00-00-0A");  // Layout 3: G

  processKeyPress(BUTTON_J,
                  "AT+BLEKEYBOARDCODE=00-00-18",   // Layout 0: U
                  "AT+BLEKEYBOARDCODE=00-00-0F",   // Layout 1: L (changed from R)
                  "AT+BLEKEYBOARDCODE=00-00-13",   // Layout 2: P
                  "AT+BLEKEYBOARDCODE=00-00-15");  // Layout 3: R

  processKeyPress(BUTTON_U,
                  "AT+BLEKEYBOARDCODE=00-00-0E",   // Layout 0: K
                  "AT+BLEKEYBOARDCODE=00-00-1B",   // Layout 1: X
                  "AT+BLEKEYBOARDCODE=00-00-1B",   // Layout 2: X
                  "AT+BLEKEYBOARDCODE=04-00-0A");  // Layout 3: Alt+G

  processKeyPress(BUTTON_I,
                  "AT+BLEKEYBOARDCODE=00-00-52",   // Layout 0: Up Arrow
                  "AT+BLEKEYBOARDCODE=00-00-52",   // Layout 1: Up Arrow
                  "AT+BLEKEYBOARDCODE=00-00-1A",   // Layout 2: W
                  "AT+BLEKEYBOARDCODE=00-00-52");  // Layout 3: Up Arrow

  processKeyPress(BUTTON_C,
                  "AT+BLEKEYBOARDCODE=00-00-0C",   // Layout 0: I
                  "AT+BLEKEYBOARDCODE=00-00-16",   // Layout 1: S
                  "AT+BLEKEYBOARDCODE=00-00-1D",   // Layout 2: Z
                  "AT+BLEKEYBOARDCODE=00-00-13");  // Layout 3: P

  processKeyPress(BUTTON_V,
                  "AT+BLEKEYBOARDCODE=00-00-51",   // Layout 0: Down Arrow
                  "AT+BLEKEYBOARDCODE=00-00-51",   // Layout 1: Down Arrow
                  "AT+BLEKEYBOARDCODE=00-00-16",   // Layout 2: S
                  "AT+BLEKEYBOARDCODE=00-00-51");  // Layout 3: Down Arrow
  
  // --- Process analog (discrete) keys ---
  processKeyPress(BUTTON_O,
                  "AT+BLEKEYBOARDCODE=00-00-04",   // Layout 0: A
                  "AT+BLEKEYBOARDCODE=00-00-13",   // Layout 1: P
                  "AT+BLEKEYBOARDCODE=00-00-19",   // Layout 2: V
                  "AT+BLEKEYBOARDCODE=04-00-0B");  // Layout 3: Alt+H

  processKeyPress(BUTTON_UP,
                  "AT+BLEKEYBOARDCODE=00-00-06",   // Layout 0: C
                  "AT+BLEKEYBOARDCODE=00-00-19",   // Layout 1: V
                  "AT+BLEKEYBOARDCODE=00-00-08",   // Layout 2: E
                  "AT+BLEKEYBOARDCODE=00-00-14");  // Layout 3: Q

  processKeyPress(BUTTON_DOWN,
                  "AT+BLEKEYBOARDCODE=00-00-19",   // Layout 0: V
                  "AT+BLEKEYBOARDCODE=00-00-09",   // Layout 1: F
                  "AT+BLEKEYBOARDCODE=00-00-28",   // Layout 2: Enter
                  "AT+BLEKEYBOARDCODE=00-00-09");  // Layout 3: F

  processKeyPress(BUTTON_LEFT,
                  "AT+BLEKEYBOARDCODE=00-00-0D",   // Layout 0: J
                  "AT+BLEKEYBOARDCODE=00-00-07",   // Layout 1: D
                  "AT+BLEKEYBOARDCODE=00-00-14",   // Layout 2: Q
                  "AT+BLEKEYBOARDCODE=00-00-08");  // Layout 3: E

  processKeyPress(BUTTON_RIGHT,
                  "AT+BLEKEYBOARDCODE=00-00-12",   // Layout 0: O
                  "AT+BLEKEYBOARDCODE=00-00-12",   // Layout 1: O
                  "AT+BLEKEYBOARDCODE=00-00-0B",   // Layout 2: H
                  "AT+BLEKEYBOARDCODE=00-00-0B");  // Layout 3: H
  
  delay(10);
}

// ------------------- Function Definitions -------------------

// Process a discrete key press.
void processKeyPress(int pin, const char* cmd0, const char* cmd1,
                     const char* cmd2, const char* cmd3) {
  if (digitalRead(pin) == LOW) {
    const char* cmds[4] = { cmd0, cmd1, cmd2, cmd3 };
    sendKeyPress(cmds[currentLayout]);
  }
}

// Process a holdable key with no release between repeats.
// When released, send a final release command.
void processHoldKeyNoRelease(int pin, const char* command, bool &held, 
                             unsigned long &lastTime, unsigned long interval) {
  if (digitalRead(pin) == LOW) {
    unsigned long now = millis();
    if (!held) {
      held = true;
      lastTime = now;
      sendKeyPressHoldableNoRelease(command);
    } else if (now - lastTime >= interval) {
      lastTime = now;
      sendKeyPressHoldableNoRelease(command);
    }
  } else {
    if (held) {
      held = false;
      Serial.print("Sending Final Release for ");
      Serial.println(command);
      ble.sendCommandCheckOK("AT+BLEKEYBOARDCODE=00-00-00");
    }
  }
}

// Send a holdable key command (no release command in between).
void sendKeyPressHoldableNoRelease(const char* command) {
  Serial.print("Sending Hold Command (No Release): ");
  Serial.println(command);
  ble.sendCommandCheckOK(command);
  // No release is sent here.
}

// Process a holdable key normally.
void processHoldKey(int pin, const char* command, bool &held, 
                    unsigned long &lastTime, unsigned long interval) {
  if (digitalRead(pin) == LOW) {
    unsigned long now = millis();
    if (!held) {
      held = true;
      lastTime = now;
      sendKeyPressHoldable(command);
    } else if (now - lastTime >= interval) {
      lastTime = now;
      sendKeyPressHoldable(command);
    }
  } else {
    held = false;
  }
}

// Send a holdable key command with immediate release.
void sendKeyPressHoldable(const char* command) {
  Serial.print("Sending Hold Command: ");
  Serial.println(command);
  ble.sendCommandCheckOK(command);
  ble.sendCommandCheckOK("AT+BLEKEYBOARDCODE=00-00-00");
  delay(20);
  delay(10);
}

// Send a discrete key press.
void sendKeyPress(const char* command) {
  Serial.print("Sending Command: ");
  Serial.println(command);
  ble.sendCommandCheckOK(command);
  delay(150);
  ble.sendCommandCheckOK("AT+BLEKEYBOARDCODE=00-00-00");
  delay(70);
}


