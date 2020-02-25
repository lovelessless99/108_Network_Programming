CC = gcc
CFLAGS = -g -Wall -c

INCLUDEPATH = include
SRCPATH = src
OBJPATH = obj
EXEPATH = .

SRCS = $(wildcard $(SRCPATH)/*.c)
OBJS = $(patsubst $(SRCPATH)/%.c, $(OBJPATH)/%.o, $(SRCS))
EXEC = npshell

all: dir build

dir:
	mkdir -p $(OBJPATH)

build: $(OBJS) $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS)
	cp $(EXEC) test/$(EXEC)

$(OBJPATH)/%.o: $(SRCPATH)/%.c
	$(CC) $(CFLAGS) $< -I $(INCLUDEPATH) -o $@

clean:
	rm -r $(OBJPATH)
	rm $(EXEC) test/$(EXEC)