CC = g++
# INCLUDEPATH = C:\boost\include\boost-1_69
# LINKERPATH = C:\boost\lib

CFLAGS = -g -Wall -lboost_system -pthread -std=c++14
TARGET_FILE = Console.cpp
EXECUTE_FILE = console.cgi

HTTP_TARGET_FILE = HttpServer.cpp
HTTP_EXECUTE_FILE = http_server

all: build

build:
	$(CC) $(TARGET_FILE) -o $(EXECUTE_FILE) $(CFLAGS)
	$(CC) $(HTTP_TARGET_FILE) -o $(HTTP_EXECUTE_FILE) $(CFLAGS)
# $(CC) -I $(INCLUDEPATH) -L $(LINKERPATH) main.cpp -o main.exe $(CFLAGS)
clean:
	rm $(EXECUTE_FILE) $(HTTP_EXECUTE_FILE)
# del main.exe

