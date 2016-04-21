### Configuration.  ####################################################


UNAME := $(shell uname)

# Binaries to build
STATIC_LIB = ../lib/libbbobb.a
ifeq ($(UNAME), Linux)
	RANLIB_OPTIONS =  
	DYNAMIC_LIB  = ../lib/libbbobb.so
	JNI_LIB = ../lib/phonyLibWhichIsHereBecauseWeNeedAJNI_LINOnMacOS.phonyLib
endif
ifeq ($(UNAME), Darwin)
	# On MacOS, we need -c option (http://lists.gnu.org/archive/html/libtool/2002-07/msg00025.html) and this option does not exist on Linux
	RANLIB_OPTIONS = -c
	DYNAMIC_LIB  = ../lib/libbbobb.dylib
	JNI_LIB  = ../lib/libbbobb.jnilib
endif
BIN = $(STATIC_LIB) $(DYNAMIC_LIB) $(JNI_LIB)
# C source files
SOURCES  = $(wildcard *.c)
# Object files of static library
OBJECTS_STATIC  = $(SOURCES:.c=.o)
# Object files of dynamic library
OBJECTS_DYNAMIC  = $(SOURCES:.c=_dyn.o)
# Include directory
INCLUDEDIRS = -I../include -I../TrainsProtocol/include -I$(JAVA_HOME)/include
# Common compilation flags
COMMON_CFLAGS = -g -Wall $(INCLUDEDIRS) $(GLOBALFLAGS) -pthread
# Common link flags
COMMON_LDFLAGS = -g

### Rules.  ############################################################

.PHONY:         all allForJNI allWithInstrumentation clean

all: depend $(BIN)

allForJNI: GLOBALFLAGS = -DJNI
allForJNI: depend $(BIN)

allWithInstrumentation: GLOBALFLAGS = -DINSERTION_TEST
allWithInstrumentation: depend $(BIN)

$(STATIC_LIB): CFLAGS = $(COMMON_CFLAGS)
$(STATIC_LIB): $(OBJECTS_STATIC)
	mkdir -p ../lib
	ar rc $@ $^
	ranlib $(RANLIB_OPTIONS) $@

$(DYNAMIC_LIB): CFLAGS = $(COMMON_CFLAGS) -fPIC
$(DYNAMIC_LIB): $(OBJECTS_DYNAMIC)
	mkdir -p ../lib
	$(CC) $(COMMON_LDFLAGS) -shared $^ -o $@ 

$(JNI_LIB): $(DYNAMIC_LIB)
	ln -s $(DYNAMIC_LIB) $(JNI_LIB)

bbError_dyn.o: bbError.c
	$(CC) $(CFLAGS) $^  -c -o $@

bbStateMachine_dyn.o: bbStateMachine.c
	$(CC) $(CFLAGS) $^  -c -o $@	

clean:
	rm -f *~ *.bak *.BAK 
	rm -f $(OBJECTS_STATIC) $(OBJECTS_DYNAMIC) $(BIN) depend

depend: CFLAGS = $(COMMON_CFLAGS)
depend: $(SOURCES)
	gcc -M $(CFLAGS) $(SOURCES) >depend 2>/dev/null

-include depend
