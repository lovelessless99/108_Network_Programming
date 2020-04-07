CC = g++
CFLAGS = -lboost_system -pthread -std=c++14

TARGET_FILE = sock_server.cpp
EXECUTE_FILE = sock_server

CONSOLE_FILE = Console.cpp
EXECUTE_CONSOLE = hw4.cgi

all: build

build:
	$(CC) $(TARGET_FILE) -o $(EXECUTE_FILE) $(CFLAGS)
	$(CC) $(CONSOLE_FILE) -o $(EXECUTE_CONSOLE) $(CFLAGS)

clean:
	rm $(EXECUTE_CONSOLE) $(EXECUTE_FILE)



