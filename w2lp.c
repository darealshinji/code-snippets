/* w2lp.c
 *
 * 2017 by Lars Stockmann
 * released in the public domain
 */

/* https://gist.github.com/larsonmars/bf9b691c359bbd4be31e1e1e36277bbf
 */

#include <stdio.h>

#define PRE_PATH "/mnt/"

int main(int argC, const char* argV[])
{
	FILE* out = stdout;
	int i, errC = 0;
	for (i = 1; i < argC; ++i) {
		const char* pSrc = argV[i];
		int c = *pSrc++;
		if (c < 'a')
			c += 'a' - 'A';

		if (c < 'a' || c > 'z' || *pSrc++ != ':') {
			++errC;
			continue;
		}

		if (i > 1)
			putc(' ', out);

		fwrite(PRE_PATH, 1, sizeof PRE_PATH - 1, out);

		putc((char)c, out);

		while((c = *pSrc++))
			putc(c == '\\' ? '/' : (char)c, out);
	}
	putc('\n', out);
	return errC;
}
