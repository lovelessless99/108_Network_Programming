INCLUDEPATH = include
OBJPATH = obj
SRCPATH = src
EXECPATH = ..

CC = gcc
CFLAGS = -g -Wall -c -D _GNU_SOURCE

SRCS = $(wildcard $(SRCPATH)/*.c)
OBJS = $(patsubst $(SRCPATH)/%.c, $(OBJPATH)/%.o, $(SRCS))
EXEC = $(EXECPATH)/np_multi_proc

all: dir build

dir:
	mkdir -p $(OBJPATH)

build: $(OBJS) $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -g -o $@ $(OBJS) -lrt

$(OBJPATH)/%.o: src/%.c
	$(CC) $(CFLAGS) $< -I $(INCLUDEPATH) -o $@

clean:
	rm -r $(OBJPATH)
	rm $(EXEC)
