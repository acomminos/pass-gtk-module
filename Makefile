CFLAGS=$(shell pkg-config --cflags --libs gtk+-3.0)

all: gtk-module-keepass.c
	$(CC) $(CFLAGS) -g -shared -fPIC gtk-module-keepass.c -o gtk-module-keepass.so
