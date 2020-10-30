CC		=  gcc
CCFLAGS	        += -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -pedantic -g
INCLUDES	= -I.
LDFLAGS 	= -L.
LIBS		= -l
OPTFLAGS	= -O3
AR=ar
ARFLAGS = rvs
LIBS = -pthread

TARGETS =	objserver \
					client

OBJECTS_SERVER = gestConnessi.o \
									server_op.o \
									util.o

OBJECTS_CLIENT = access.o \
									util.o 

INCLUDE_FILE_SERVER = gestConnessi.h \
												server_op.h \
												util.h

INCLUDE_FILE_CLIENT = access.h \
											util.h



.PHONY: all clean test
.SUFFIXES: .c .h

%: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -c -o $@ $<


all	: $(TARGETS)

objserver: objserver.o libserver.a $(INCLUDE_FILE_SERVER)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

client: client.o libclient.a $(INCLUDE_FILE_CLIENT)
	$(CC) $(CFLAGS) $(INCLUDES)  $(OPTFLAGS) $(LDFLAGS) -o $@ $^

libserver.a: $(OBJECTS_SERVER)
			$(AR) $(ARFLAGS) $@ $^

libclient.a: $(OBJECTS_CLIENT)
			$(AR) $(ARFLAGS) $@ $^


clean		:
	rm -rf data/
	\rm -rf objstore.sock
	\rm -rf *.[ao]
	\rm -rf $(OBJECTS_CLIENT)
	\rm -rf $(OBJECTS_SERVER)
	\rm -rf $(TARGETS)

test : clean all
	\./objserver &
	\bash ./test.sh > testout.log 2>&1
	killall -SIGINT objserver
	\bash ./testsum.sh
