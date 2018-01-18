/**
 * implementation of basename/dirname using only std::string features
 */
#include <string>
#include <iostream>

//#define UNICODE 1

#ifdef UNICODE
typedef std::wstring STRING;
#define NPOS  std::wstring::npos
#define COUT  std::wcout
#define T(x)  L##x
#else
typedef std::string STRING;
#define NPOS  std::string::npos
#define COUT  std::cout
#define T(x)  x
#endif

STRING basename_dirname(STRING str, bool dir, bool win32)
{
	std::size_t pos;
	const STRING delim = win32 ? T("/\\") : T("/");

	if (str.empty() || str.length() == 0) {
		return T(".");
	}

	// return unmodified string if there are no delimiters
	if (str.find_first_of(delim) == NPOS) {
		return str;
	}

	// delete trailing delimiters
	if ((pos = str.find_last_not_of(delim)) != NPOS) {
		str.erase(pos + 1);
	} else {
		// the whole string was just delimiters;
		// error (empty string) on Windows, root directory on Unix
		return str = win32 ? T("") : T("/");
	}

	// split at last delimiter
	pos = str.find_last_of(delim);
	if (dir) {
		if (pos != NPOS) {
			str.erase(pos + 1);
		} else {
			return str = T(".");
		}
	} else {
		if (pos != NPOS) {
			str.erase(0, pos + 1);
		}
	}

	// delete trailing delimiters from dirname
	if (dir && (pos = str.find_last_not_of(delim)) != NPOS) {
		str.erase(pos + 1);
	}

	return str;
}

STRING basename_unix(STRING str) {
	return basename_dirname(str, false, false);
}

STRING basename_win32(STRING str) {
	return basename_dirname(str, false, true);
}

STRING dirname_unix(STRING str) {
	return basename_dirname(str, true, false);
}

STRING dirname_win32(STRING str) {
	return basename_dirname(str, true, true);
}


int main(void)
{
	STRING path_unix = T("/usr/bin/bash");
	STRING path_win32 = T("C:\\Windows\\explorer.exe");

	COUT << "original       >>> " << path_unix
		<< "\nbasename_unix  >>> " << basename_unix(path_unix)
		<< "\ndirname_unix   >>> " << dirname_unix(path_unix)
		<< "\n\n"
			"original       >>> " << path_win32
		<< "\nbasename_win32 >>> " << basename_win32(path_win32)
		<< "\ndirname_win32  >>> " << dirname_win32(path_win32)
		<< std::endl;
	return 0;
}

