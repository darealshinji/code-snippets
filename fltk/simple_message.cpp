#include <FL/Fl.H>
#include <FL/fl_ask.H>

// set visibility in Fl_Export.H to "hidden"
// cmake settings: MinSizeRel, -Os -fPIC -DPIC -ffunction-sections -fdata-sections -fvisibility=hidden
// g++ -Os $(fltk-config --cflags) -shared -o simple_message.so simple_message.cpp -Wl,--gc-sections $(fltk-config --ldflags) -s

extern "C" __attribute__ ((visibility ("default")))
void simple_message(const char *title, const char *message) {
  fl_message_title(title);
  fl_alert("%s", message);
  Fl::run();
}

