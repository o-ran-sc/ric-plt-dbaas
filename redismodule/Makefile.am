ACLOCAL_AMFLAGS = -I m4

redismoduledir = @libexecdir@/$(PACKAGE)
redismodule_LTLIBRARIES = libredismodule.la
#lib_LTLIBRARIES = libredismodule.la

BASE_LDFLAGS =

libredismodule_la_SOURCES = \
	include/redismodule.h\
	src/exstrings.c 

libredismodule_la_CFLAGS = \
	-std=c11 -fPIC -g \
	-I${includedir} -I${top_srcdir}/include 


libredismodule_la_LDFLAGS = $(BASE_LDFLAGS) -avoid-version -module -shared

#pkgincludedir = ${includedir}
#pkginclude_HEADERS = include/redismodule.h 

clean-local: 
	rm -rf ${builddir}/libredismodule.pc

# UT
CPP_U_TEST=$(CPP_U_TEST_LATEST)
check_PROGRAMS = redismodule_ut
#TESTS = ${check_PROGRAMS} 
redismodule_ut_SOURCES = \
	tst/mock/include/redismodule.h  \
	tst/mock/include/exstringsStub.h \
	src/exstrings.c \
	tst/mock/src/redismoduleStub.cpp \
	tst/src/main.cpp \
	tst/src/exstrings_test.cpp 


redismodule_ut_CFLAGS = \
        -std=c11 -g -Wall \
        -fprofile-arcs -ftest-coverage \
        -D__UT__ \
		-I${top_srcdir}/tst/mock/include \
		-I${includedir} \
		-I${top_srcdir}/include \
		-I${CPP_U_TEST_LATEST}/include 

redismodule_ut_CXXFLAGS = \
	-std=c++11 -g -Wall \
	-fprofile-arcs -ftest-coverage \
	-D__UT__ \
	-I${top_srcdir}/tst/mock/include \
	-I${includedir} \
	-I${top_srcdir}/include \
	-I${CPP_U_TEST_LATEST}/include 


redismodule_ut_LDFLAGS = -Wl,-rpath=${libdir} ${UT_COVERAGE_LDFLAGS}
redismodule_ut_LDADD = -L${libdir} -L${CPP_U_TEST_LATEST}/lib -lCppUTest -lCppUTestExt -lgcov 

test: redismodule_ut
	./run-tests.sh

TESTS = run-tests.sh
