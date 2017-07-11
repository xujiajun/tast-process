dnl $Id$
dnl config.m4 for extension tast_process

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(tast_process, for tast_process support,
dnl Make sure that the comment is aligned:
dnl [  --with-tast_process             Include tast_process support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(tast_process, whether to enable tast_process support,
Make sure that the comment is aligned:
[  --enable-tast_process           Enable tast_process support])

if test "$PHP_TAST_PROCESS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-tast_process -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/tast_process.h"  # you most likely want to change this
  dnl if test -r $PHP_TAST_PROCESS/$SEARCH_FOR; then # path given as parameter
  dnl   TAST_PROCESS_DIR=$PHP_TAST_PROCESS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for tast_process files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TAST_PROCESS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TAST_PROCESS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the tast_process distribution])
  dnl fi

  dnl # --with-tast_process -> add include path
  dnl PHP_ADD_INCLUDE($TAST_PROCESS_DIR/include)

  dnl # --with-tast_process -> check for lib and symbol presence
  dnl LIBNAME=tast_process # you may want to change this
  dnl LIBSYMBOL=tast_process # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TAST_PROCESS_DIR/$PHP_LIBDIR, TAST_PROCESS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TAST_PROCESSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong tast_process lib version or lib not found])
  dnl ],[
  dnl   -L$TAST_PROCESS_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(TAST_PROCESS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(tast_process, tast_process.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
