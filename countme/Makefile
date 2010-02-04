CPP  = g++.exe
CXX  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
RM = rm
UPX = upx -9 -q --compress-icons=0
SRC = countme.cpp
OBJ = $(SRC:.cpp=.o)
INCS =  -I"../../include"
BIN  = /home/ppiastucki/Desktop/miranda/miranda/Plugins/countme.dll
CFLAGS = $(INCS) -DBUILDING_DLL=1 -fno-exceptions -fexpensive-optimizations -O3 -Os -Wno-error -Wall
CXXFLAGS = $(INCS) -DBUILDING_DLL=1 -fno-exceptions -fexpensive-optimizations -O3 -Os -Wno-error -Wall
LIBS =  -lwsock32
LFLAGS = -s -mdll -mwindows --no-export-all-symbols --add-stdcall-alias
RCFLAGS = -J rc -O coff

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after
ifdef PACK
	$(UPX) $(BIN)
endif

clean: clean-custom
	$(RM) -f $(OBJ) $(BIN)

depend :
	$(CPP) -MM $(CFLAGS) $(SRC)>Makefile.dep

$(BIN): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) $(LIBS) $(LFLAGS) -o $(BIN) -Wl

-include Makefile.dep
