#include "MenuSystem.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>
#include <MD_MAX72xx.h>
#include "util.h"
#include "scroll_blueline.h"

// TO FIX
// when coming out a sub menu like methods, there's an auto-scroll and the upper menu changes its selection, causing a visual jump

// byte smiley[8] = {
//   B00000,
//   B01010,
//   B00000,
//   B00000,
//   B10001,
//   B01110,
//   B00000,
// };

byte bell[8] = {
  B00000,
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00100,
};

int tick_duration = 50;

int lcdBrightness = 1;

char* leafScreenName = NULL;

// #define MENU_BRIGHTNESS F("Brightness")
// #define MENU_SPEED F("Speed")
// #define MENU_METHOD F("Select method")

#define MENU_BRIGHTNESS "Brightness"
#define MENU_SPEED "Speed"
#define MENU_METHOD "Select method"

// Mac OS USB connection struggles above level 3 (current supply)
#define MAX_BRIGHTNESS 3

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
MenuItem methodsMenuItems[] = {
  {"Bristol", nullptr},
  {"Stedman", nullptr},
  {"Double Norwich", nullptr},
  {"Back", nullptr}
};

MenuItem submenu2Items[] = {
  {"Sub2-OptionA", nullptr},
  {"Sub2-OptionB", nullptr},
  {"Back", nullptr}
};

// ---- Now define the actual submenus (after the arrays exist) ----
Menu methodsMenu = {"Method:", methodsMenuItems, ARRAY_LEN(methodsMenuItems)};
// Menu submenu2 = {"Brightness", nullptr, 0};  //submenu2Items, ARRAY_LEN(submenu2Items)};

// ---- Now define main menu items (using the submenus above) ----
char mainTitle[] = {'M', 'e', 't', 'h', 'o', 'd', ' ', 'h', 'y', 'p', 'n', 'o', 's', 'i', 's', '\1'};
MenuItem mainMenuItems[] = {
  {MENU_METHOD, &methodsMenu},
  {MENU_SPEED, nullptr},
  // null means a leaf screen with non-menu handling
  {MENU_BRIGHTNESS, nullptr}
};

Menu mainMenu = {mainTitle, mainMenuItems, ARRAY_LEN(mainMenuItems)};

Menu* currentMenu = &mainMenu;
Menu* parentMenus[5];
int menuDepth = 0;
int currentMenuIndex = 0;
int lastRotaryPosition = 0;
bool buttonPressed = false;

// unsigned long rotaryDebounceStartTime = 0;
// unsigned long rotaryDebounceDuration = 100;

// bool strcmp_PF(const char* ramString, const __FlashStringHelper* flashString) {
//   char temp[32];  // adjust size to fit longest expected string
//   strncpy_P(temp, (const char*)flashString, sizeof(temp));
//   temp[sizeof(temp) - 1] = '\0';  // ensure null-termination
//   return strcmp(ramString, temp) == 0;
// }

#define SCREEN_IDX_MAIN       0
#define SCREEN_IDX_BRIGHTNESS 1
#define SCREEN_IDX_SPEED      2

static int lastDetailScreenIndex = -1;
static int lastValueSeen = -1;

void updateMenuDisplay() {

  PRINT("updateMenuDisplay()");
  
  // TODO only update what needs updating here.
  // e.g. just the value, or even not that, if it was the old value (e.g. brightness clamp)

  PRINT_VAR("Leaf screen name: ", leafScreenName);

  // if (!strcmp(leafScreenName, "")) {

  // if (!strcmp_PF(leafScreenName, "")) {
  // if (!strcmp(leafScreenName, "")) {
  if (leafScreenName) {
    PRINT_VAR("Got into non-empty str bit, leaf = ", leafScreenName);
    if (!strcmp(leafScreenName, MENU_BRIGHTNESS)) {
      PRINT(" ... got into BRIGHTNESS");
      if (lastDetailScreenIndex != SCREEN_IDX_BRIGHTNESS) {
        // lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("   Brightness   "));
      }
      if (lastDetailScreenIndex != 0 || lastValueSeen != lcdBrightness) {
        lcd.setCursor(0, 1);
        // assuming 1 char brightness currently!
        lcd.print(F("       "));
        lcd.print(lcdBrightness);
        lcd.print(F("        "));
      }
      lastValueSeen = lcdBrightness;
      lastDetailScreenIndex = SCREEN_IDX_BRIGHTNESS;
      return;
    }
    else if (!strcmp(leafScreenName, MENU_SPEED)) {
      PRINT(" ... got into SPEED");
      lcd.setCursor(0, 0);
      lcd.print(F("  Scroll speed  "));
      lcd.setCursor(0, 1);
      lcd.print(F("                "));
      lcd.setCursor(0, 1);
      lcd.print(F("       "));
      lcd.print(tick_duration);

      lastValueSeen = tick_duration;
      lastDetailScreenIndex = 1;
    }
    return;
  }

  // if (strcmp(lastScreenTitle, currentMenu->title)) { //} != 0 || lastValueSeen != lcdBrightness) {
    // lcd.write((uint8_t)1); // to add bell char
  PRINT_VAR("Current menu title: ", currentMenu->title);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentMenu->title);
  lcd.setCursor(0, 1);
  lcd.print(F("> "));
  lcd.print(currentMenu->items[currentMenuIndex].label);
  // }
}

///////////////////////////////////////////////
// LCD display sleep
unsigned long lastActivityTime = 0;
const unsigned long inactivityTimeout = 15000;
const unsigned long sleepMessageDuration = 2000;
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
  // Serial.println(F("<< WOKE DISPLAY >>"));
  return true;
}
///////////////////////////////////////////////

void handleSelection() {
  // TODO revert this, just to stop me having to unplug it all the time!
  // halt();

  if (registerActivity()) {
    // ignore selection if display was woken up
    return;
  }

  MenuItem* selected = &currentMenu->items[currentMenuIndex];

  // exiting a leaf node?
  if (leafScreenName != NULL) {
    leafScreenName = NULL;
  }
  else if (strcmp(selected->label, "Back") == 0 && menuDepth > 0) {
    currentMenu = parentMenus[--menuDepth];
    currentMenuIndex = currentMenu->activeItem;
    Serial.print(F("AAA Back out a menu, restored currentMenuIndex = "));
    Serial.println(currentMenuIndex);
  }
  else if (selected->submenu != nullptr) {
    PRINT("BBB into submenu");

    parentMenus[menuDepth++] = currentMenu;
    currentMenu->activeItem = currentMenuIndex;
    currentMenu = selected->submenu;
    currentMenuIndex = 0;
  }
  else {
    PRINT("CCC a leaf...");

    if (currentMenu == &methodsMenu) {
      // PRINT_VAR("HAROOOOO! METHOD SEL: ", currentMenuIndex);
      // TODO need order of method items to match what is defined in scroll_blueino.ino.
      // Fix this properly later.
      setMethodIndex(currentMenuIndex);
      // don't attempt to updateMenuDisplay
      return;
    }
    else {
      leafScreenName = selected->label;
    }
  }

  // does this need doing at all? try without
  // encoder.write(0);
  // lastRotaryPosition = 0;
  updateMenuDisplay();
}

void start_menu() {
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  // lcd.createChar(0, smiley);
  lcd.createChar(1, bell);

  encoder.write(0);
  updateMenuDisplay();
  lastActivityTime = millis();
}

int debug_tick = 0;

void loop_menu() {
  // Serial.println("ENTER loop menu");
  // encoder.tick();

// think the lib will handle this?
  // unsigned long rotaryDebounceStartTime = 0;
  // unsigned long rotaryDebounceDuration = 100;
  // if (rotaryDebounceStartTime == 0 || (millis() - rotaryDebounceStartTime >= rotaryDebounceDuration)) {
    // rotaryDebounceStartTime = millis();

    long newRotaryPosition = encoder.read() / 4;  // divide by 4 to debounce steps

    // if (debug_tick == 0) {
    //   Serial.print("Rot pos: ");
    //   Serial.println(newRotaryPosition); 
    // }
    // debug_tick = (debug_tick + 1) % 10;

    if (newRotaryPosition != lastRotaryPosition) {
      Serial.print(F(">>>>>>> Change in pos detected: "));
      Serial.print(lastRotaryPosition);
      Serial.print(F(" to "));
      Serial.println(newRotaryPosition);

      if (registerActivity()) {
        // ignore selection if display was woken up
        lastRotaryPosition = newRotaryPosition;
        return;
      }

      // leaf screen?
      if (leafScreenName == "Brightness") {
        lcdBrightness += lastRotaryPosition - newRotaryPosition;
        lcdBrightness = max(min(lcdBrightness, MAX_BRIGHTNESS), 0);
        setBrightness(lcdBrightness);
      }
      else if (leafScreenName == "Speed") {
        tick_duration += (lastRotaryPosition - newRotaryPosition) * 5;
        tick_duration = max(min(tick_duration, 200), 0);
        setSpeed(tick_duration);
      }
      else {
        int menuCount = currentMenu->itemCount;
        currentMenuIndex = (currentMenuIndex + lastRotaryPosition - newRotaryPosition + menuCount) % menuCount;
        Serial.print(F("Item: "));
        Serial.println(currentMenuIndex);
      }

      lastRotaryPosition = newRotaryPosition;
      updateMenuDisplay();
    }
  // // }

  if (!displayIsOff && sleepMessageStart == 0 && millis() - lastActivityTime > inactivityTimeout) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(sleepMessage);
    sleepMessageStart = millis();
    // force redraw on wake up
    lastDetailScreenIndex = -1;
    lastValueSeen = -1;
    PRINT("<< DISPLAY about to sleep... >>");
  }
  if (sleepMessageStart > 0 && millis() - sleepMessageStart > sleepMessageDuration) {
    sleepMessageStart = 0;
    displayIsOff = true;
    lcd.noBacklight();
    lcd.noDisplay();
    PRINT("<< DISPLAY SLEPT >>");
  }

  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    Serial.println(F("<< button pressed"));
    buttonPressed = true;
    handleSelection();
  } else if (digitalRead(buttonPin) == HIGH) {
    // Serial.println("<< button released");
    buttonPressed = false;
  }
  // Serial.println(" ... EXIT loop menu");
}
