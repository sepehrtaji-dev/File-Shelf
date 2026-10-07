# MSYS2 / MinGW-w64 (ucrt64) build
CC      = gcc
CXX     = g++
WINDRES = windres
TARGET  = build/shelf.exe

C_SRCS   = src/main.c src/shelf_window.c src/shelf_items.c \
           src/drop_in.c src/clipboard.c \
           src/util.c src/glass.c
CXX_SRCS = src/ui_paint.cpp src/drag_out.cpp
RC_SRC   = res/shelf.rc

C_OBJS   = $(C_SRCS:src/%.c=build/%.o)
CXX_OBJS = $(CXX_SRCS:src/%.cpp=build/%.o)
RC_OBJ   = build/shelf.res.o

CFLAGS   = -O2 -Wall -Wextra -DUNICODE -D_UNICODE -Isrc
CXXFLAGS = -O2 -Wall -Wextra -DUNICODE -D_UNICODE -Isrc
LDFLAGS  = -mwindows -municode
LIBS     = -luser32 -lgdi32 -lgdiplus -lshell32 -lole32 -loleaut32 \
           -ldwmapi -lshlwapi -lcomctl32 -ladvapi32 -luuid

all: $(TARGET)

$(TARGET): $(C_OBJS) $(CXX_OBJS) $(RC_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RC_OBJ): $(RC_SRC) res/manifest.xml
	@mkdir -p build
	$(WINDRES) -I res -i $(RC_SRC) -o $@

clean:
	rm -rf build

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run