#include "MenuSystem.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>


// LCD and encoder setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Encoder setup (CLK = 2, DT = 3)
Encoder encoder(2, 3);
const int buttonPin = 4;


///////////////////////////////////////////////
// LCD display sleep
unsigned long lastActivityTime = 0;
const unsigned long inactivityTimeout = 5000; // 60 seconds
const unsigned long sleepMessageDuration = 500;
unsigned long sleepMessageStart = 0;
String sleepMessage = "Sleeping...";

bool displayIsOff = false;

// returns true if the display woke up (might use this to ignore this activity for usability reasons)
bool registerActivity() {
  lastActivityTime = millis();
  if (!displayIsOff) {
    return false;
  }

  displayIsOff = false;
  lcd.display();  // Wake up display
  lcd.backlight();
  Serial.println("<< WOKE DISPLAY >>");
  return true;
}
///////////////////////////////////////////////


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
Menu submenu1 = {"Method", submenu1Items, ARRAY_LEN(submenu1Items)};
Menu submenu2 = {"Brightness", submenu2Items, ARRAY_LEN(submenu2Items)};

// ---- Now define main menu items (using the submenus above) ----
MenuItem mainMenuItems[] = {
  {"Method", &submenu1},
  {"Brightness", &submenu2}
};

// ---- And the main menu ----
Menu mainMenu = {"Main Menu", mainMenuItems, ARRAY_LEN(mainMenuItems)};

// State
Menu* currentMenu = &mainMenu;
Menu* parentMenus[5];
int menuDepth = 0;
int currentItem = 0;
int lastPosition = 0;
bool buttonPressed = false;

void updateMenuDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentMenu->title);
  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(currentMenu->items[currentItem].label);
}

void handleSelection() {
  if (registerActivity()) {
    // ignore selection if display was woken up
    return;
  }

  MenuItem* selected = &currentMenu->items[currentItem];

  if (strcmp(selected->label, "Back") == 0 && menuDepth > 0) {
    currentMenu = parentMenus[--menuDepth];
  } else if (selected->submenu) {
    parentMenus[menuDepth++] = currentMenu;
    currentMenu = selected->submenu;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Selected:");
    lcd.setCursor(0, 1);
    lcd.print(selected->label);
    delay(1000);
  }

  encoder.write(0);
  currentItem = 0;
  lastPosition = -1;
  updateMenuDisplay();
}

void start_menu() {
  lcd.init();
  lcd.backlight();
  pinMode(buttonPin, INPUT_PULLUP);
  encoder.write(0);
  updateMenuDisplay();
  lastActivityTime = millis();
}

int debug_tick = 0;

void loop_menu() {
  // encoder.tick();
  long newPosition = encoder.read() / 4;  // divide by 4 to debounce steps

  // if (debug_tick == 0) {
  //   Serial.println(newPosition); 
  // }
  // debug_tick = (debug_tick + 1) % 10;

  if (newPosition != lastPosition) {
    if (registerActivity()) {
      // ignore selection if display was woken up
      return;
    }

    int menuCount = currentMenu->itemCount;
    if (newPosition > lastPosition) {
      currentItem = (currentItem + 1) % menuCount;
    } else {
      currentItem = (currentItem - 1 + menuCount) % menuCount;
    }
    lastPosition = newPosition;
    updateMenuDisplay();

    Serial.print("Item: ");
    Serial.println(currentItem);
  }

  if (!displayIsOff && millis() - lastActivityTime > inactivityTimeout) {
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
