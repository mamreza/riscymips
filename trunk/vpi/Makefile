# Makefile
SHELL = /bin/sh
scrdir = .

MAIN_FLAGS = -std=c99 -pedantic -O2 -mtune=native -fPIC
W_FLAGS = -Wall -Wextra -Wundef -Winit-self -Wswitch-default -Wshadow
D_FLAGS = -fstack-protector -ggdb
CFLAGS = $(MAIN_FLAGS) $(W_FLAGS) $(D_FLAGS)
LDFLAGS = -lvpi
#CC = g++

EXEC1 = hello_vpi
EXEC2 = list_net_vpi
EXEC3 = regfile_test_vpi

# Define the target "all"
.PHONY: all
all: $(EXEC1) $(EXEC2) $(EXEC3)

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
$(EXEC3): $(EXEC3).o
	$(CC) $(LDFLAGS) $(EXEC3).o -o $@.vpi -shared

$(EXEC3).o: $(EXEC3).c
	$(CC) $(CFLAGS) -c $(EXEC3).c

.PHONY: clean
clean:
	-rm -f *.o $(EXEC) core