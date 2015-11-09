dnl $Id$
dnl config.m4 for extension zyxss

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(zyxss, for zyxss support,
	Make sure that the comment is aligned:
	[  --with-zyxss             Include zyxss support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(zyxss, whether to enable zyxss support,
Make sure that the comment is aligned:
[  --enable-zyxss           Enable zyxss support])

if test "$PHP_ZYXSS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-zyxss -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/zyxss.h"  # you most likely want to change this
  dnl if test -r $PHP_ZYXSS/$SEARCH_FOR; then # path given as parameter
  dnl   ZYXSS_DIR=$PHP_ZYXSS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for zyxss files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       ZYXSS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$ZYXSS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the zyxss distribution])
  dnl fi

  dnl # --with-zyxss -> add include path
  dnl PHP_ADD_INCLUDE($ZYXSS_DIR/include)

  dnl # --with-zyxss -> check for lib and symbol presence
  dnl LIBNAME=zyxss # you may want to change this
  dnl LIBSYMBOL=zyxss # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ZYXSS_DIR/lib, ZYXSS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ZYXSSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong zyxss lib version or lib not found])
  dnl ],[
  dnl   -L$ZYXSS_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(ZYXSS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(zyxss, zyxss.c, $ext_shared)
fi

if test -z "$PHP_DEBUG"; then
        AC_ARG_ENABLE(debug,
                [--enable-debg  compile with debugging system],
                [PHP_DEBUG=$enableval], [PHP_DEBUG=no]
        )
fi
