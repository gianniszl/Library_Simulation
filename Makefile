CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2
TARGET  = library
SRCS    = main.c
HDRS    = Library.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) < test1.txt

clean:
	rm -f $(TARGET)
