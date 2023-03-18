dnl @synopsis mc_I18N
dnl
dnl Check if environment is ready for get translations of docs from transifex
dnl
dnl @author Slava Zanko <slavazanko@gmail.com>
dnl @author Mooffie <mooffie@gmail.com>
dnl @version 2023-03-18
dnl @license GPL
dnl @copyright Free Software Foundation, Inc.

AC_DEFUN([mc_I18N],[
    dnl User visible support for charset conversion.
    AC_ARG_ENABLE([charset],
        AS_HELP_STRING([--enable-charset], [Support for charset selection and conversion @<:@yes@:>@]))
    have_charset=
    charset_msg="no"
    if test "x$enable_charset" != "xno"; then
        AC_DEFINE(HAVE_CHARSET, 1, [Define to enable charset selection and conversion])
        have_charset=yes
        charset_msg="yes"

        dnl Solaris has different name of Windows 1251 encoding
        case $host_os in
            solaris*)
                CP1251="ANSI-1251"
                ;;
            *)
                CP1251="CP1251"
                ;;
        esac

        AC_SUBST(CP1251)
    fi
])

AC_DEFUN([mc_I18N_PRINTF_GROUPING],[
    gt_GLIBC2

    AC_CACHE_CHECK([whether printf() can print localized thousand separators.],
        [mc_cv_i18n_printf_grouping],
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
   * doesn't, we assume we have i18n support. */
  char buf[100];
  sprintf (buf, "%'d", 123);
  return (strcmp (buf, "123") != 0);
}
                ]])], [grouping=yes], [:], [:])
            ])

            mc_cv_i18n_printf_grouping=$grouping
        ])

    if test x"$mc_cv_i18n_printf_grouping" = xyes; then
        AC_DEFINE(HAVE_I18N_PRINTF_GROUPING, [1], [Define if printf() and family can print localized thousands separators for numbers.])
    fi

])
