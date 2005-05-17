# Project: ieview

CPP  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
RM = rm
UPX = upx -9 -q --compress-icons=0
SRC = main.c menuitem.c options.c popup.c
RES  = resource.res
OBJ = $(SRC:.c=.o) $(RES)
BIN  = ngnotify.dll
INCS =  -I"../../include"
CFLAGS = $(INCS) -DBUILDING_DLL=1 -fexpensive-optimizations -O3 -Os -fno-exceptions -Wno-error -Wall
CXXFLAGS = $(INCS) -DBUILDING_DLL=1 -fexpensive-optimizations -O3 -Os -fno-exceptions -Wno-error -Wall
LIBS =  -lversion -lwsock32 -lole32 -lcomctl32 -luuid -loleaut32
LFLAGS = -s -mdll -mwindows --add-stdcall-alias
#LFLAGS = -mdll -mwindows -Wl,-Map,ieview.map --no-export-all-symbols --add-stdcall-alias -g3
RCFLAGS = -I rc -O coff

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after
ifdef PACK
	$(UPX) $(BIN)
endif

clean: clean-custom
	$(RM) -f $(OBJ) $(BIN)

depend :
	$(CPP) -MM $(CFLAGS) $(SRC)>Makefile.dep

$(RES): $(RES:.res=.rc) resource.h
	$(WINDRES) $(RCFLAGS) -i $(RES:.res=.rc) -o $(RES)

$(BIN): $(OBJ)
	$(CPP) $(CFLAGS) $(OBJ) $(LIBS) $(LFLAGS) -o $(BIN) -Wl

-include Makefile.dep
