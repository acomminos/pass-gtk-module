CFLAGS=$(shell pkg-config --cflags --libs gtk+-3.0)

all: pass.c gtk-module-pass.c
	$(CC) $(CFLAGS) -g -shared -fPIC pass.c gtk-module-pass.c -o gtk-module-pass.so
