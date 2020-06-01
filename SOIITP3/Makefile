# Directorios
SOURCE=sources
BINARY_DIR=bin
#Binarios
SERVERS=servers
USER=user
# Opt de compilacion 
CC=gcc 
CFLAGS=-std=gnu11  -Wall -Werror -pedantic -Wextra -Wno-unused-parameter -Wconversion -g  
LIB=-lulfius -ljansson -lcrypt

all : build
build : $(SOURCE)/servers.c $(SOURCE)/user.c
	mkdir -p $(BINARY_DIR)
	cppcheck ./ --enable=all 
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(SERVERS).o -c $(SOURCE)/servers.c   $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(SERVERS)   $(BINARY_DIR)/$(SERVERS).o $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(USER).o -c $(SOURCE)/user.c   $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(USER)   $(BINARY_DIR)/$(USER).o $(LIB)
.PHONY: clean
clean :
	rm  -Rf $(BINARY_DIR)
docs:
	doxygen doxyfile
 