# Makefile
CC = gcc
CFLAGS = -g -Wall -O0
#LDFLAGS = 
#LDLIBS = 


EXE1 = blastee
EXE2 = blaster

OBJS1 = blastee.o
OBJS2 = blaster.o
OBJS  = $(OBJS1) $(OBJS2)
SHARE = 

.PHONY: all
all: $(EXE1) $(EXE2)

$(EXE1): $(OBJS1) $(SHARE)
	$(CC) -o $@ $(LDFLAGS) $(OBJS1) $(LDLIBS)
$(EXE2): $(OBJS2) $(SHARE)
	$(CC) -o $@ $(LDFLAGS) $(OBJS2) $(LDLIBS)

.PHONY: clean
clean: 
	rm -f $(EXE1) $(EXE2) $(OBJS) $(SHARE)
