INC= -Iinclude/
SRC=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRC))
LDFLAGS= -ljpeg -lpthread -lsqlite3
CFLAGS= $(INC) -g -DJPEG -DM0 -DDEBUG

ifeq ($(ARCH),arm)
BIN=server_arm
CC=arm-linux-gcc
else
BIN=server_x86
CC=gcc
endif
$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm $(OBJS) $(BIN)

