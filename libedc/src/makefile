###############################################################################
##      S i Z i O U S    R i P P E D    L i B E D C    M A K E F i L E       ##
###############################################################################

# proggys
UNAME_S := $(shell uname -s)
CC = gcc
AR = ar
RM = rm
STRIP = strip
ifeq ($(UNAME_S),Darwin)
	LIBTOOL = libtool
endif

# flags
CFLAGS = -I"../inc" -O3 -fexpensive-optimizations
ARFLAGS = rcs

# files
TARGET = ../lib/libedc.a
OBJSDIR = obj/
OBJS = $(OBJSDIR)edc_ecc.o

all : $(TARGET)

# make lib
$(TARGET) : $(OBJS)
ifeq ($(UNAME_S),Darwin)
	$(LIBTOOL) -static -o $@ $(OBJS)
else
	$(AR) $(ARFLAGS) $(TARGET) $(OBJS)
endif

# Build rules : Compile all !
$(OBJSDIR)%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJSDIR)%.o: %.cc
	$(CPP) $(CFLAGS) -c $< -o $@

$(OBJSDIR)%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

$(OBJSDIR)%.o: %.s
	$(AS) $< -o $@
	
clean:
	$(RM) $(OBJS)
	
clean-all:
	$(RM) $(OBJS) $(TARGET)