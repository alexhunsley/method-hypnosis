#include "MenuSystem.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

// TO FIX
// when coming out a sub menu like methods, there's an auto-scroll and the upper menu changes its selection, causing a visual jump

byte smiley[8] = {
  B00000,
  B01010,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

byte bell[8] = {
  B00000,
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00100,
};

// LCD and encoder setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Encoder setup (CLK = 2, DT = 3)
Encoder encoder(2, 3);
const int buttonPin = 4;

// Menu system definitions
struct Menu;

struct MenuItem {
  const char* label;
  Menu* submenu;
};

struct Menu {
  const char* title;
  MenuItem* items;
  int itemCount;
  // for restoring when we come out a submenu
  int activeItem;
};

// ---- Forward declarations of the submenu Menu structs ----
extern Menu submenu1;
extern Menu submenu2;

// ---- Now define submenu items ----
MenuItem submenu1Items[] = {
  {"Bristol", nullptr},
  {"Stedman", nullptr},
  {"Back", nullptr}
};

MenuItem submenu2Items[] = {
  {"Sub2-OptionA", nullptr},
  {"Sub2-OptionB", nullptr},
  {"Back", nullptr}
};

// ---- Now define the actual submenus (after the arrays exist) ----
Menu submenu1 = {"Method:", submenu1Items, ARRAY_LEN(submenu1Items)};
Menu submenu2 = {"Brightness", submenu2Items, ARRAY_LEN(submenu2Items)};

// ---- Now define main menu items (using the submenus above) ----
char mainTitle[] = {'M', 'e', 't', 'h', 'o', 'd', ' ', 'h', 'y', 'p', 'n', 'o', 's', 'i', 's', '\1'};
MenuItem mainMenuItems[] = {
  {"Choose method", &submenu1},
  {"Brightness", &submenu2}
};

Menu mainMenu = {mainTitle, mainMenuItems, ARRAY_LEN(mainMenuItems)};

Menu* currentMenu = &mainMenu;
Menu* parentMenus[5];
int menuDepth = 0;
int currentMenuIndex = 0;
int lastRotaryPosition = 0;
bool buttonPressed = false;

void updateMenuDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  // lcd.write((uint8_t)1); // to add bell char
  lcd.print(currentMenu->title);
  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(currentMenu->items[currentMenuIndex].label);
}

///////////////////////////////////////////////
// LCD display sleep
unsigned long lastActivityTime = 0;
const unsigned long inactivityTimeout = 30000;
const unsigned long sleepMessageDuration = 3000;
unsigned long sleepMessageStart = 0;
const String sleepMessage = "  Sleeping...";

bool displayIsOff = false;

// returns true if the display woke up (might use this to ignore this activity for usability reasons)
bool registerActivity() {
  lastActivityTime = millis();
  // reset sleep message timer (in case sleep msg showing when user interacts)
  sleepMessageStart = 0;
  if (!displayIsOff) {
    return false;
  }
  displayIsOff = false;
  updateMenuDisplay();
  lcd.display();  // Wake up display
  lcd.backlight();
  Serial.println("<< WOKE DISPLAY >>");
  return true;
}
///////////////////////////////////////////////

void handleSelection() {
  if (registerActivity()) {
    // ignore selection if display was woken up
    return;
  }

  MenuItem* selected = &currentMenu->items[currentMenuIndex];

  if (strcmp(selected->label, "Back") == 0 && menuDepth > 0) {
    currentMenu = parentMenus[--menuDepth];
    currentMenuIndex = currentMenu->activeItem;
    Serial.print("Back out a menu, restored currentMenuIndex = ");
    Serial.println(currentMenuIndex);
  } else if (selected->submenu) {
    parentMenus[menuDepth++] = currentMenu;
    currentMenu->activeItem = currentMenuIndex;
    currentMenu = selected->submenu;
    currentMenuIndex = 0;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Selected:");
    lcd.setCursor(0, 1);
    lcd.print(selected->label);
    // TODO this is not good! but is temp code, anyway
    delay(1000);
  }

  encoder.write(0);
  lastRotaryPosition = 0;
  updateMenuDisplay();
}

void start_menu() {
  lcd.init();
  lcd.backlight();
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.createChar(0, smiley);
  lcd.createChar(1, bell);

  encoder.write(0);
  updateMenuDisplay();
  lastActivityTime = millis();
}

int debug_tick = 0;

void loop_menu() {
  // encoder.tick();
  long newRotaryPosition = encoder.read() / 4;  // divide by 4 to debounce steps

  // if (debug_tick == 0) {
  //   Serial.println(newRotaryPosition); 
  // }
  // debug_tick = (debug_tick + 1) % 10;

  if (newRotaryPosition != lastRotaryPosition) {
    Serial.print(">>>>>>> Change in pos detected: ");
    Serial.print(lastRotaryPosition);
    Serial.print(" to ");
    Serial.println(newRotaryPosition);

    if (registerActivity()) {
      // ignore selection if display was woken up
      return;
    }

    int menuCount = currentMenu->itemCount;
    if (lastRotaryPosition >= 0) {
      if (newRotaryPosition > lastRotaryPosition) {
        currentMenuIndex = (currentMenuIndex + 1) % menuCount;
      } else {
        currentMenuIndex = (currentMenuIndex - 1 + menuCount) % menuCount;
      }
    }
    lastRotaryPosition = newRotaryPosition;
    updateMenuDisplay();

    Serial.print("Item: ");
    Serial.println(currentMenuIndex);
  }

  if (!displayIsOff && sleepMessageStart == 0 && millis() - lastActivityTime > inactivityTimeout) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(sleepMessage);
    sleepMessageStart = millis();
    Serial.println("<< DISPLAY about to sleep... >>");
  }
  if (sleepMessageStart > 0 && millis() - sleepMessageStart > sleepMessageDuration) {
    sleepMessageStart = 0;
    displayIsOff = true;
    lcd.noBacklight();
    lcd.noDisplay();
    Serial.println("<< DISPLAY SLEPT >>");
  }
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    handleSelection();
  } else if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }

  delay(50);
}
