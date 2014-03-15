CC     = g++
CFLAGS = -g -Wall --std=c++11
EXT    = cpp
TARGET = main
DRIVER = Classifier

all: start

debug: CFLAGS += -DDEBUG
debug: start

start: $(TARGET)

$(TARGET): % : %.o $(patsubst %, %.o, $(DRIVER))
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.$(EXT)
	$(CC) $(CFLAGS) -c $<

help:
	@echo "Help"

clean:
	rm -f *.o $(TARGET)
