CFLAGS ?= -Wall -O2 -levdev
VPATH = src

SRCS = xiaoxind.c
OBJS = $(SRCS:.c=.o)
TARGET = xiaoxind

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: all
	install -Dm755 $(TARGET) /usr/local/bin/$(TARGET)
	install -Dm644 $(TARGET).service /etc/systemd/system/$(TARGET).service
