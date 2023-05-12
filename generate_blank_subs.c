/* quick and dirty code to print out ASS subtitle lines for every 200ms,
 * which can be used to ease up manual timing of hardcoded subtitles
 * (in case an OCR tool can't handle the source material)
 */
#include <stdio.h>

void lines(int h, int m_max)
{
  int m, m2, s, s2;

  for (m = 0; m < m_max; m++) {
    for (s = 0; s < 60; s++) {
      m2 = m;
      s2 = s + 1;

      if (s == 59) {
        m2++;
        s2 = 0;
      }

      printf("Dialogue: 0,%d:%02d:%02d.00,%d:%02d:%02d.20,Default,,0,0,0,,\r\n", h, m, s, h, m, s);
      printf("Dialogue: 0,%d:%02d:%02d.20,%d:%02d:%02d.40,Default,,0,0,0,,\r\n", h, m, s, h, m, s);
      printf("Dialogue: 0,%d:%02d:%02d.60,%d:%02d:%02d.80,Default,,0,0,0,,\r\n", h, m, s, h, m, s);
      printf("Dialogue: 0,%d:%02d:%02d.80,", h, m, s);

      if (m2 == 60) {
        m2 = 0;
        h++;
      }

      printf("%d:%02d:%02d.00,Default,,0,0,0,,\r\n", h, m2, s2);
    }
  }
}

int main()
{
  printf(
    "[Script Info]\r\n"
    "Title: Default Aegisub file\r\n"
    "ScriptType: v4.00+\r\n"
    "\r\n"
    "[V4+ Styles]\r\n"
    "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, "
       "BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, "
       "BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\r\n"
    "Style: Default,Arial,48,&H00FFFFFF,&H000000FF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,2,2,10,10,10,1\r\n"
    "\r\n"
    "[Events]\r\n"
    "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\r\n"
  );

  lines(0, 60); /* 0h, 60m */
  lines(1, 5);  /* 1h, 5m */

  return 0;
}
