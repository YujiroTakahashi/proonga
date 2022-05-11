dnl $Id$
dnl config.m4 for extension proonga

PHP_ARG_ENABLE(groonga, whether to enable groonga support,
dnl Make sure that the comment is aligned:
[  --enable-proonga           Enable proonga support])

if test "$PHP_PROONGA" != "no"; then
  # --with-proonga -> check with-path
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/groonga/groonga.h"
  if test -r $PHP_PROONGA/$SEARCH_FOR; then
    GROONGA_DIR=$PHP_PROONGA
  else
    AC_MSG_CHECKING([for groonga files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        GROONGA_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$GROONGA_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the groonga distribution])
  fi

  # --with-proonga -> add include path
  PHP_ADD_INCLUDE($GROONGA_DIR/include/groonga)

  # --with-proonga -> check for lib and symbol presence
  LIBNAME="groonga"
  LIBSYMBOL="grn_get_version"

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $GROONGA_DIR/lib, PROONGA_SHARED_LIBADD)
    AC_DEFINE(HAVE_GROONGALIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong groonga lib version or lib not found])
  ],[
    -L$GROONGA_DIR/lib -lm
  ])

  PHP_SUBST(PROONGA_SHARED_LIBADD)

  CFLAGS="-Wimplicit-function-declaration"

  PHP_NEW_EXTENSION(proonga, proonga.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
