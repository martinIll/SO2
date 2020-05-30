# Directorios
SOURCE=sources
BINARY_DIR=bin
#Binarios
SERVERS=servers
USER=user
# Opt de compilacion 
CC=gcc 
CFLAGS=-std=gnu11  -Wall -Werror -pedantic -Wextra -Wconversion -g -O3  
LIB=-lulfius -ljansson

all : build
build : $(SOURCE)/servers.c  	
	mkdir -p $(BINARY_DIR)
	cppcheck ./ --enable=all 
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(SERVERS).o -c $(SOURCE)/servers.c   $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(MAIN)   $(BINARY_DIR)/$(SERVERS).o $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(USER).o -c $(SOURCE)/user.c   $(LIB)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINARY_DIR)/$(USER)   $(BINARY_DIR)/$(USER).o $(LIB)

.PHONY: clean
clean :
	rm  -Rf $(BINARY_DIR)
docs:
	doxygen doxyfile
 