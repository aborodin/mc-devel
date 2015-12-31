dnl @synopsis mc_ENCODING
dnl
dnl Clarify encoding names in mc.charset
dnl
dnl @author Slava Zanko <slavazanko@gmail.com>
dnl @modified Yury V. Zaytsev <yury@shurup.com>
dnl @modified Andrew Borodin <aborodin@vmail.ru>
dnl @version 2025-03-22
dnl @license GPL
dnl @copyright Free Software Foundation, Inc.

AC_DEFUN([mc_ENCODING],[
    AC_CHECK_HEADERS([gnu/libc-version.h])

    dnl Solaris has different name of Windows 1251 encoding
    case $host_os in
        solaris*)
            ENCODING_CP1251="ANSI-1251"
            ;;
        *)
            ENCODING_CP1251="CP1251"
            ;;
    esac

    if test "x$ac_cv_header_gnu_libc_version_h" != "xno"; then
        ENCODING_CP866="IBM866"
        ENCODING_ISO8859="ISO-8859"
    else
        ENCODING_CP866="CP866"
        ENCODING_ISO8859="ISO8859"
    fi

    AC_SUBST(ENCODING_CP1251)
    AC_SUBST(ENCODING_CP866)
    AC_SUBST(ENCODING_ISO8859)
])

AC_DEFUN([mc_ENCODING_PRINTF_GROUPING],[
    gt_GLIBC2

    AC_CACHE_CHECK([whether printf() can print localized thousand separators.],
        [mc_cv_encoding_printf_grouping],
        [
            grouping=no

            dnl Since runtime detection doesn't work when cross-compiling, we first
            dnl test for the existence of a decent glibc library.

            AS_IF([test x"$GLIBC2" = xyes], [grouping=yes], [
                AC_MSG_NOTICE([** TRYING RUNTIME DETECTION])
                AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <string.h>
int main ()
{
  /* We merely check that the "'" doesn't break anything. If it
   * doesn't, we assume we have encoding support. */
  char buf[100];
  sprintf (buf, "%'d", 123);
  return (strcmp (buf, "123") != 0);
}
                ]])], [grouping=yes], [:], [:])
            ])

            mc_cv_encoding_printf_grouping=$grouping
        ])

    if test x"$mc_cv_encoding_printf_grouping" = xyes; then
        AC_DEFINE(HAVE_ENCODING_PRINTF_GROUPING, [1], [Define if printf() and family can print localized thousands separators for numbers.])
    fi

])
