AC_INIT([redismodule], [0.0.0], [], [], [https://gerrit.oran-osc.org/r/#/admin/projects/ric-plt/dbaas])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([run-tests.sh], [chmod +x run-tests.sh])


AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects tar-pax])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([disable-static])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_ARG_ENABLE([unit-test],
[  --enable-unit-test    Check for UT dependencies],
[  PKG_CHECK_MODULES([LIBCPPUTEST],[cpputest])
   UNIT_TEST_ENABLED=true
],
[  UNIT_TEST_ENABLED=false  ])
AM_CONDITIONAL([UNIT_TEST_ENABLED], [$UNIT_TEST_ENABLED])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
