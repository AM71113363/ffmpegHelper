# Project: AM71113363
# Makefile for FFMPEG CMD helper


CC   = gcc.exe -s
WINDRES = windres.exe
RES  = main.res
OBJ  = main.o $(RES)
LINKOBJ  = main.o $(RES)
LIBS =  -L"C:/Dev-Cpp/lib" -mwindows  
INCS =  -I"C:/Dev-Cpp/include" 
BIN  = ffmpegHelper.exe
CFLAGS = $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before ffmpegHelper.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o "ffmpegHelper.exe" $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

main.res: main.rc 
	$(WINDRES) -i main.rc --input-format=rc -o main.res -O coff 
