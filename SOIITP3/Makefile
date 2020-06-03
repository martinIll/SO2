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
install:
	#!/bin/bash
	sudo useradd -p contraseña tp3user
	echo 'tp3user ALL=(ALL) NOPASSWD:ALL' | sudo EDITOR='tee -a' visudo
	sudo cp -f ./bin/user /usr/bin/tp3_user
	sudo cp -f ./bin/servers /usr/bin/tp3_servers
	sudo cp -f ./resources/tp3_servers.service /lib/systemd/system/tp3_servers.service
	sudo cp -f ./resources/tp3_users.service /lib/systemd/system/tp3_users.service
	sudo systemctl start tp3_servers.service
	sudo systemctl start tp3_users.service
	sudo systemctl enable tp3_users.service
	sudo systemctl enable tp3_users.service
	sudo cp -f ./resources/tp3 /etc/nginx/sites-available/tp3
	sudo ln -s /etc/nginx/sites-available/tp3 /etc/nginx/sites-enabled/tp3
	sudo mkdir -p /etc/tp3
	sudo htpasswd   -b -c /etc/tp3/.htpasswd USER CONTRASENA 
	sudo systemctl restart nginx.service

