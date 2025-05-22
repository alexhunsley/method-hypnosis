#include "MenuSystem.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

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
};


// ---- Forward declarations of the submenu Menu structs ----
extern Menu submenu1;
extern Menu submenu2;

// ---- Now define main menu items (using the submenus above) ----
MenuItem mainMenuItems[] = {
  {"XXX Go to sub1", &submenu1},
  {"XXX Go to sub2", &submenu2},
  {"Exit", nullptr}
};

// ---- Now define submenu items ----
MenuItem submenu1Items[] = {
  {"Sub1-Option1", nullptr},
  {"Sub1-Option2", nullptr},
  {"Back", nullptr}
};

MenuItem submenu2Items[] = {
  {"Sub2-OptionA", nullptr},
  {"Sub2-OptionB", nullptr},
  {"Back", nullptr}
};

// ---- Now define the actual submenus (after the arrays exist) ----
Menu submenu1 = {"Submenu 1", submenu1Items, 3};
Menu submenu2 = {"Submenu 2", submenu2Items, 3};

// ---- And the main menu ----
Menu mainMenu = {"Main Menu", mainMenuItems, 3};

// State
Menu* currentMenu = &mainMenu;
Menu* parentMenus[5];
int menuDepth = 0;
int currentItem = 0;
int lastPosition = -1;
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
}

int debug_tick = 0;

void loop_menu() {
  // encoder.tick();
  long newPosition = encoder.read() / 4;  // divide by 4 to debounce steps

  if (debug_tick == 0) {
    Serial.println(newPosition); 
  }
  debug_tick = (debug_tick + 1) % 20;

  if (newPosition != lastPosition) {
    int menuCount = currentMenu->itemCount;
    if (newPosition > lastPosition) {
      currentItem = (currentItem + 1) % menuCount;
    } else {
      currentItem = (currentItem - 1 + menuCount) % menuCount;
    }
    lastPosition = newPosition;
    updateMenuDisplay();
  }

  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    handleSelection();
  } else if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }

  delay(50);
}
