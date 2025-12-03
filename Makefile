# mac make file for raylib
# Important: Put your libraylib.a in a folder called "lib"
# Important: Make sure to put your .h files form raylin in the project folder.
CFLAGS = -I. -Llib #-I is include -L is include library
LIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo
MAIN=ray_game.cpp


ray:
g++ -std=c++11 -g $(MAIN) -o ray $(CFLAGS) $(LIBS)