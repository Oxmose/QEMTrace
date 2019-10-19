################################################################################
# File: Makefile
#
# Author: Alexy Torres Aurora Dugo
#
# Date: 09/08/2018
#
# Version: 1.0
#
# Makefile to compile the SMI library. 
################################################################################

LIBS = -lpthread -lrt
LIB_DIR = lib
EXPORT_LIB_DIR = ./lib
SRC_DIR = src
BUILD_DIR = build
INC_DIR = include
BIN_DIR = bin

# C++ Flags
EXTRAFLAGS = -std=c11 -Wall -Wextra -mno-ms-bitfields -Werror -D_POSIX_C_SOURCE=199309L

ifeq ($(DEBUG), true)
  EXTRAFLAGS += -g -O0 
else 
  EXTRAFLAGS += -O2
endif


.PHONY: all clean

all: libexport
	rm -rf $(BIN_DIR) $(BUILD_DIR)

clean:
	$(RM) -rf $(BUILD_DIR) *~ $(BIN_DIR) $(COV_DIR) $(DOXY_DIR)


####################### Project code section #######################
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

#compiles source files to build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(EXTRAFLAGS) -c $< -o $@ -I $(INC_DIR)

init_build:
	@mkdir -p $(BUILD_DIR)

#compiles all source files
build: init_build $(OBJS)

########################## Library export section ##########################
LIB_EXPORT_NAME = libsmi.a
libexport: build
	mkdir -p $(EXPORT_LIB_DIR)	
	ar rcs $(EXPORT_LIB_DIR)/$(LIB_EXPORT_NAME) $(OBJS)
	mkdir -p $(EXPORT_LIB_DIR)/include
	cp -rf $(INC_DIR)/* $(EXPORT_LIB_DIR)/include/
	
cleanlib: clean
	$(RM) -rf $(EXPORT_LIB_DIR)/*
