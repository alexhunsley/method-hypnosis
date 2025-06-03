#ifndef SCROLL_BLUELINE_H
#define SCROLL_BLUELINE_H

extern void setBrightness(int b);
extern void setMethodIndex(int index);
extern void halt();

extern int blueline_sleep_time;
extern int selectedMethodIdx;
extern int selectedMethodPNCount;

extern MD_MAX72XX mx;

inline void setSpeed(int s) {
  blueline_sleep_time = s;
}

// NB value is clamped before we get in here
inline void setBrightness(int b) {
  Serial.print(F("BR: "));
  Serial.println(b);
  mx.control(MD_MAX72XX::INTENSITY, b);
}

// triggers method change
inline void setMethodIndex(int methodIndex) {
  selectedMethodIdx = methodIndex;
  // do this last! (but maybe mono-threaded, anyway)
  selectedMethodPNCount = 0;
}


#endif
