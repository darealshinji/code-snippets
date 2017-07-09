/* BinReloc - a library for creating relocatable executables
 * Written by: Hongli Lai <h.lai@chello.nl>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                    Version 2, December 2004
 *
 * Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#ifndef BINRELOC_H
#define BINRELOC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** These error codes can be returned by br_init(), br_init_lib(), gbr_init() or gbr_init_lib(). */
typedef enum {
	/** Cannot allocate memory. */
	BR_INIT_ERROR_NOMEM,
	/** Unable to open /proc/self/maps; see errno for details. */
	BR_INIT_ERROR_OPEN_MAPS,
	/** Unable to read from /proc/self/maps; see errno for details. */
	BR_INIT_ERROR_READ_MAPS,
	/** The file format of /proc/self/maps is invalid; kernel bug? */
	BR_INIT_ERROR_INVALID_MAPS,
	/** BinReloc is disabled (the ENABLE_BINRELOC macro is not defined). */
	BR_INIT_ERROR_DISABLED
} BrInitError;


#ifndef BINRELOC_RUNNING_DOXYGEN
/* Mangle symbol names to avoid symbol
 * collisions with other ELF objects.
 */
#ifndef BINRELOC_MANGLE
#define BINRELOC_MANGLE(x)  x
//#define BINRELOC_MANGLE(x)  x##_abcdefg123456
#endif
#define br_init             BINRELOC_MANGLE(br_init)
#define br_init_lib         BINRELOC_MANGLE(br_init_lib)
#define br_find_exe         BINRELOC_MANGLE(br_find_exe)
#define br_find_exe_dir     BINRELOC_MANGLE(br_find_exe_dir)
#define br_find_prefix      BINRELOC_MANGLE(br_find_prefix)
#define br_find_bin_dir     BINRELOC_MANGLE(br_find_bin_dir)
#define br_find_sbin_dir    BINRELOC_MANGLE(br_find_sbin_dir)
#define br_find_data_dir    BINRELOC_MANGLE(br_find_data_dir)
#define br_find_locale_dir  BINRELOC_MANGLE(br_find_locale_dir)
#define br_find_lib_dir     BINRELOC_MANGLE(br_find_lib_dir)
#define br_find_libexec_dir BINRELOC_MANGLE(br_find_libexec_dir)
#define br_find_etc_dir     BINRELOC_MANGLE(br_find_etc_dir)
#define br_strcat           BINRELOC_MANGLE(br_strcat)
#define br_build_path       BINRELOC_MANGLE(br_build_path)
#define br_dirname          BINRELOC_MANGLE(br_dirname)
#endif /* BINRELOC_RUNNING_DOXYGEN */

int   br_init             (BrInitError *error);
int   br_init_lib         (BrInitError *error);

char *br_find_exe         (const char *default_exe);
char *br_find_exe_dir     (const char *default_dir);
char *br_find_prefix      (const char *default_prefix);
char *br_find_bin_dir     (const char *default_bin_dir);
char *br_find_sbin_dir    (const char *default_sbin_dir);
char *br_find_data_dir    (const char *default_data_dir);
char *br_find_locale_dir  (const char *default_locale_dir);
char *br_find_lib_dir     (const char *default_lib_dir);
char *br_find_libexec_dir (const char *default_libexec_dir);
char *br_find_etc_dir     (const char *default_etc_dir);

/* Utility functions */
char *br_strcat  (const char *str1, const char *str2);
char *br_build_path (const char *dir, const char *file);
char *br_dirname (const char *path);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BINRELOC_H */
