#include <Adafruit_BluefruitLE_SPI.h>

// ------ Bluefruit SPI Pins ------
#define BLUEFRUIT_SPI_CS   8
#define BLUEFRUIT_SPI_IRQ  7  
#define BLUEFRUIT_SPI_RST  4  

// Create the Bluefruit BLE object
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// ------ Key Pin Definitions ------
// Digital pins
#define BUTTON_M      2  
#define BUTTON_P      3  
#define BUTTON_J      5  
#define BUTTON_U      6  
#define BUTTON_I      9  
#define BUTTON_K      10 
#define BUTTON_A      11 
#define BUTTON_C      12   // For this button, layout 3 will now send P.
#define BUTTON_V      13 
// Analog pins used as digital inputs
#define BUTTON_O      A0    // Mapping: Layout0=A, Layout1=P, Layout2=V, Layout3=Alt+H
#define BUTTON_UP     A1    // Mapping: Layout0=C, Layout1=G, Layout2=E, Layout3=Q
#define BUTTON_DOWN   A2    // Mapping: Layout0=V, Layout1=F, Layout2=Enter, Layout3=F
#define BUTTON_LEFT   A3    // Mapping: Layout0=J, Layout1=D, Layout2=Q, Layout3=E
#define BUTTON_RIGHT  A4    // Mapping: Layout0=O, Layout1=O, Layout2=H, Layout3=H
// Layout Cycle Button
#define LAYOUT_BUTTON A5  

// Layout state: 0, 1, 2, or 3
// With the following names: 0 = GSpro, 1 = Courseplay, 2 = TGC2019, 3 = FXS Play
int currentLayout = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting Multi-Key BLE Keyboard..."));

  // Set digital pins as INPUT_PULLUP.
  pinMode(BUTTON_M, INPUT_PULLUP);
  pinMode(BUTTON_P, INPUT_PULLUP);
  pinMode(BUTTON_J, INPUT_PULLUP);
  pinMode(BUTTON_U, INPUT_PULLUP);
  pinMode(BUTTON_I, INPUT_PULLUP);
  pinMode(BUTTON_K, INPUT_PULLUP);
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_V, INPUT_PULLUP);

  // Set the analog pins (A0-A4) as digital inputs with pull-ups.
  pinMode(BUTTON_O, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(LAYOUT_BUTTON, INPUT_PULLUP);

  // Initialize Bluefruit module
  if (!ble.begin(true)) {
    Serial.println(F("Couldn't find Bluefruit, check wiring?"));
    while (1);
  }
  ble.reset();
  ble.sendCommandCheckOK(F("AT+BleHIDEn=1"));
  ble.sendCommandCheckOK(F("AT+GAPDEVNAME=BACUSTOMBOX"));
  ble.sendCommandCheckOK(F("AT+GAPSTARTADV"));

  Serial.println(F("Advertising as BACUSTOMBOX..."));
}

void loop() {
  // Handle the layout cycle button on A5.
  if (digitalRead(LAYOUT_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(LAYOUT_BUTTON) == LOW) {
      currentLayout = (currentLayout + 1) % 4;
      Serial.print("Current Layout: ");
      Serial.println(currentLayout);
      while (digitalRead(LAYOUT_BUTTON) == LOW) {
        delay(10);
      }
      delay(50);
    }
  }
  
  // Process key presses for each button.
  processKeyPress(BUTTON_M, "AT+BLEKEYBOARDCODE=01-00-10", "AT+BLEKEYBOARDCODE=00-00-10", "AT+BLEKEYBOARDCODE=00-00-29", "AT+BLEKEYBOARDCODE=00-00-29");  // Pin 2: Ctrl+M, M, Esc, Esc
  processKeyPress(BUTTON_P, "AT+BLEKEYBOARDCODE=00-00-1C", "AT+BLEKEYBOARDCODE=00-00-0A", "AT+BLEKEYBOARDCODE=00-00-0A", "AT+BLEKEYBOARDCODE=00-00-0A");  // Pin 3: Y, G, G, G
  processKeyPress(BUTTON_J, "AT+BLEKEYBOARDCODE=00-00-18", "AT+BLEKEYBOARDCODE=00-00-15", "AT+BLEKEYBOARDCODE=00-00-13", "AT+BLEKEYBOARDCODE=00-00-15");  // Pin 5: U, R, P, R
  processKeyPress(BUTTON_U, "AT+BLEKEYBOARDCODE=00-00-0E", "AT+BLEKEYBOARDCODE=00-00-1B", "AT+BLEKEYBOARDCODE=00-00-1B", "AT+BLEKEYBOARDCODE=04-00-0A");  // Pin 6: K, X, X, Alt+G
  processKeyPress(BUTTON_I, "AT+BLEKEYBOARDCODE=00-00-52", "AT+BLEKEYBOARDCODE=00-00-52", "AT+BLEKEYBOARDCODE=00-00-1A", "AT+BLEKEYBOARDCODE=00-00-52");  // Pin 9: Up Arrow, Up Arrow, W, Up Arrow
  processKeyPress(BUTTON_K, "AT+BLEKEYBOARDCODE=00-00-50", "AT+BLEKEYBOARDCODE=00-00-50", "AT+BLEKEYBOARDCODE=00-00-50", "AT+BLEKEYBOARDCODE=00-00-50");  // Pin 10: Left Arrow, Left Arrow, Left Arrow, Left Arrow
  processKeyPress(BUTTON_A, "AT+BLEKEYBOARDCODE=00-00-4F", "AT+BLEKEYBOARDCODE=00-00-4F", "AT+BLEKEYBOARDCODE=00-00-4F", "AT+BLEKEYBOARDCODE=00-00-4F");  // Pin 11: Right Arrow, Right Arrow, Right Arrow, Right Arrow
  // For BUTTON_C (Pin 12), the fourth parameter (Layout 3) is now set to send P.
  processKeyPress(BUTTON_C, "AT+BLEKEYBOARDCODE=00-00-0C", 
                             "AT+BLEKEYBOARDCODE=00-00-16", 
                             "AT+BLEKEYBOARDCODE=00-00-1D", 
                             "AT+BLEKEYBOARDCODE=00-00-13");
  processKeyPress(BUTTON_V, "AT+BLEKEYBOARDCODE=00-00-51", "AT+BLEKEYBOARDCODE=00-00-51", "AT+BLEKEYBOARDCODE=00-00-16", "AT+BLEKEYBOARDCODE=00-00-51");  // Pin 13: Down Arrow, Down Arrow, S, Down Arrow
  
  // Process analog pin keys:
  // BUTTON_O (A0): Layout0=A, Layout1=P, Layout2=V, Layout3=Alt+H
  processKeyPress(BUTTON_O, "AT+BLEKEYBOARDCODE=00-00-04", "AT+BLEKEYBOARDCODE=00-00-13", "AT+BLEKEYBOARDCODE=00-00-19", "AT+BLEKEYBOARDCODE=04-00-0B");
  // BUTTON_UP (A1): Layout0=C, Layout1=G, Layout2=E, Layout3=Q
  processKeyPress(BUTTON_UP, "AT+BLEKEYBOARDCODE=00-00-06", "AT+BLEKEYBOARDCODE=00-00-0A", "AT+BLEKEYBOARDCODE=00-00-08", "AT+BLEKEYBOARDCODE=00-00-14");
  // BUTTON_DOWN (A2): Layout0=V, Layout1=F, Layout2=Enter, Layout3=F
  processKeyPress(BUTTON_DOWN, "AT+BLEKEYBOARDCODE=00-00-19", "AT+BLEKEYBOARDCODE=00-00-09", "AT+BLEKEYBOARDCODE=00-00-28", "AT+BLEKEYBOARDCODE=00-00-09");
  // BUTTON_LEFT (A3): Layout0=J, Layout1=D, Layout2=Q, Layout3=E
  processKeyPress(BUTTON_LEFT, "AT+BLEKEYBOARDCODE=00-00-0D", "AT+BLEKEYBOARDCODE=00-00-07", "AT+BLEKEYBOARDCODE=00-00-14", "AT+BLEKEYBOARDCODE=00-00-08");
  // BUTTON_RIGHT (A4): Layout0=O, Layout1=O, Layout2=H, Layout3=H
  processKeyPress(BUTTON_RIGHT, "AT+BLEKEYBOARDCODE=00-00-12", "AT+BLEKEYBOARDCODE=00-00-12", "AT+BLEKEYBOARDCODE=00-00-0B", "AT+BLEKEYBOARDCODE=00-00-0B");
  
  delay(10);
}

void processKeyPress(int button, const char* layout0, const char* layout1, 
                     const char* layout2, const char* layout3) {
  if (digitalRead(button) == LOW) {
    const char* cmds[4] = { layout0, layout1, layout2, layout3 };
    sendKeyStandard(cmds[currentLayout]);
  }
}

void sendKeyStandard(const char* command) {
  Serial.print("Sending Command: ");
  Serial.println(command);
  ble.sendCommandCheckOK(command);
  delay(150);
  ble.sendCommandCheckOK("AT+BLEKEYBOARDCODE=00-00-00");
  delay(70);
}





