# Makefile
SHELL = /bin/sh
scrdir = .

MAIN_FLAGS = -std=c99 -pedantic -O2 -mtune=native -fPIC
W_FLAGS = -Wall -Wextra -Wundef -Winit-self -Wswitch-default -Wshadow
D_FLAGS = -fstack-protector -ggdb
VPI_CFLAGS := $(shell iverilog-vpi --cflags)
CFLAGS = $(VPI_CFLAGS) $(MAIN_FLAGS) $(W_FLAGS) $(D_FLAGS)
LDFLAGS = -lvpi
#CC = g++

EXEC1 = hello_vpi
EXEC2 = list_net_vpi
EXEC3 = regfile_test_vpi
EXEC4 = alu_test_vpi
ALL = $(EXEC1) $(EXEC2) $(EXEC3) $(EXEC4)
COMMON = common

# Define the target "all"
.PHONY: all
all: $(ALL)

.PHONY: $(EXEC1)
$(EXEC1): $(EXEC1).o
	$(CC) $(LDFLAGS) $(EXEC1).o -o $@.vpi -shared

$(EXEC1).o: $(EXEC1).c
	$(CC) $(CFLAGS) -c $(EXEC1).c

.PHONY: $(EXEC2)
$(EXEC2): $(EXEC2).o
	$(CC) $(LDFLAGS) $(EXEC2).o -o $@.vpi -shared

$(EXEC2).o: $(EXEC2).c
	$(CC) $(CFLAGS) -c $(EXEC2).c

.PHONY: $(EXEC3)
$(EXEC3): $(EXEC3).o $(COMMON).o
	$(CC) $(LDFLAGS) $(EXEC3).o $(COMMON).o -o $@.vpi -shared

$(EXEC3).o: $(EXEC3).c
	$(CC) $(CFLAGS) -c $(EXEC3).c

.PHONY: $(EXEC4)
$(EXEC4): $(EXEC4).o $(COMMON).o
	$(CC) $(LDFLAGS) $(EXEC4).o $(COMMON).o -o $@.vpi -shared

$(EXEC4).o: $(EXEC4).c
	$(CC) $(CFLAGS) -c $(EXEC4).c


$(COMMON).o: $(COMMON).c $(COMMON).h
	$(CC) $(CFLAGS) -c $(COMMON).c

.PHONY: clean
clean:
	-rm -f *.o $(ALL) core