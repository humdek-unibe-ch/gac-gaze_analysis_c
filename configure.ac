AC_INIT([gac], [0.1.0], [simon.maurer@unibe.ch])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/])

AC_PROG_CC
AM_PROG_AR

AC_ENABLE_SHARED
AC_ENABLE_STATIC

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([sqrt strdup])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
