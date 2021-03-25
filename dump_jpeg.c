/* quick and dirty program intended to extract unencrypted
 * JPEG image files from a binary data file */

#include <stdio.h>
#define MAX_FILES 9999


int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("usage: %s FILENAME\n", argv[0]);
    return 1;
  }

  FILE *fpIn, *fpOut = NULL;
  long pos = 0, size = 0;
  int i = 1;
  unsigned char buf[1];
  unsigned char c;
  char outname[32];
  const unsigned char footer[2] = { 0xFF, 0xD9 };
  const unsigned char header[3] = { 0xFF, 0xD8, 0xFF };

  if ((fpIn = fopen(argv[1], "rb")) == NULL) {
    perror("fopen()");
    return 1;
  }

  fseek(fpIn, 0, SEEK_END);
  size = ftell(fpIn);
  rewind(fpIn);

  while (i <= MAX_FILES && ftell(fpIn) < size) {
    if ((c = fgetc(fpIn)) == EOF) {
      break;
    }

    // check for header or footer
    if (c == 0xFF) {
      pos = ftell(fpIn);

      // JPEG footer 0xFF, 0xD9
      if (fpOut && fgetc(fpIn) == 0xD9) {
        fwrite(&footer, 1, 2, fpOut);
        fclose(fpOut);
        fpOut = NULL;
        i++;
        continue;
      }

      fseek(fpIn, pos, SEEK_SET);

      // JPEG header 0xFF, 0xD8, 0xFF
      if (!fpOut && fgetc(fpIn) == 0xD8 && fgetc(fpIn) == 0xFF) {
        snprintf(outname, sizeof(outname) - 1, "%04d.jpg", i);

        if ((fpOut = fopen(outname, "wb")) == NULL) {
          perror("fopen()");
          fclose(fpIn);
          return 1;
        }

        if (fwrite(&header, 1, 3, fpOut) != 3) {
          perror("fwrite()");
          fclose(fpIn);
          fclose(fpOut);
          return 1;
        }

        printf("%s\n", outname);

        continue;
      }

      fseek(fpIn, pos, SEEK_SET);
    }

    buf[0] = c;

    if (fpOut && fwrite(&buf, 1, 1, fpOut) != 1) {
      perror("fwrite()");
      fclose(fpIn);
      fclose(fpOut);
      return 1;
    }
  }

  if (fpIn) fclose(fpIn);
  if (fpOut) fclose(fpOut);

  return 0;
}
