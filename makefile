CC = g++ -std=c++11
CFLAGS = -g -Wall
SRCS = main.cpp
PROG = hair

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV) -lm

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

.PHONY: clean

clean:
	rm -f hair_* hair

