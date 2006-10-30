# Project: scriver

CPP  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
RM = rm
UPX = upx -9 -q --compress-icons=0
SRC = cmdlist.c globals.c msgdialog.c msglog.c msgoptions.c msgs.c msgtimedout.c msgwindow.c srmm.c statusicon.c utils.c chat/clist.c chat/colorchooser.c chat/log.c chat/main.c chat/manager.c chat/message.c chat/options.c chat/services.c chat/tools.c chat/window.c
RES  = resource.res
OBJ = $(SRC:.c=.o) $(RES)
INCS =  -I"../../include"
BIN  = scriver_unicode.dll
CFLAGS = $(INCS) -DNDEBUG -DBUILDING_DLL=1 -DUNICODE -fexpensive-optimizations -O3 -Os -fno-exceptions -Wno-error -Wall
CXXFLAGS = $(INCS) -DNDEBUG -DBUILDING_DLL=1 -DUNICODE -fexpensive-optimizations -O3 -Os -fno-exceptions -Wno-error -Wall
# -save-temps
LIBS =  -lshlwapi -lversion -lwsock32 -lole32 -lcomctl32 -luuid -loleaut32
LFLAGS = -s -mdll -mwindows --add-stdcall-alias
#LFLAGS = -mdll -mwindows -Wl,-Map,ieview.map --no-export-all-symbols --add-stdcall-alias -g3
RCFLAGS = -I rc -O coff
#  --add-stdcall-alias -s  --no-export-all-symbols

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
