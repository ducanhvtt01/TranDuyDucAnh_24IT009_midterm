CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu99
TARGET = lsducanh
OBJS = main.o ls_core.o utils.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c ls.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
