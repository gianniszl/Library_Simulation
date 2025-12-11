CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2
TARGET  = project_phase2
SRCS    = main.c
HDRS    = Library.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) < test2.txt

clean:
	rm -f $(TARGET)