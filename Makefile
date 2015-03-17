AR=$(TARGET)ar
CC=$(TARGET)gcc

CFLAGS=-Wall -Wpedantic -std=c99 -g -Og
SOURCES=src/liblicor.c src/cc2500/cc2500.c
OBJECTS=$(SOURCES:src/%.c=build/%.o)
ARTIFACT=build/liblicor.a

.PHONY: clean example install uninstall

all: pre-build $(SOURCES) $(ARTIFACT)

pre-build:
	@mkdir -p build/
	@mkdir -p build/cc2500

$(ARTIFACT): $(OBJECTS)
	$(AR) -r $@ $(OBJECTS)

build/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(ARTIFACT)
	rm -rf build/licor

example: $(ARTIFACT) build/licor

build/licor: example/main.c
	$(CC) $(CFLAGS) -c -Isrc/ example/main.c -o build/example.o
	$(CC) -Lbuild/ build/example.o -llicor -o build/licor

install: pre-build $(ARTIFACT) build/licor
	install --group=root --owner=root build/licor /usr/local/bin
	install --group=root --owner=root example/http/index.html example/http/jquery.js example/http/licor.js /srv/http
	mkdir --mode=775 /var/local/licor
	echo -en "\x00" > /var/local/licor/seqno
	chmod 666 /var/local/licor/seqno

uninstall: /usr/local/bin/licor
	rm -f /usr/local/bin/licor
	rm -f /src/http/index.html
	rm -f /src/http/jquery.js
	rm -f /src/http/licor.js
	rm -rf /var/local/licor

