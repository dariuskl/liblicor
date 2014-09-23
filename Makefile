AR=$(PREFIX)ar
CC=$(PREFIX)gcc

CFLAGS=-Wall -Wpedantic -std=c99 -g -Og
SOURCES=src/liblicor.c src/cc2500/cc2500.c
OBJECTS=$(SOURCES:src/%.c=build/%.o)
ARTIFACT=build/liblicor.a

.PHONY: clean example

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

example: $(ARTIFACT)
	$(CC) $(CFLAGS) -c -Isrc/ example/main.c -o build/example.o
	$(CC) -Lbuild/ build/example.o -lm -llicor -o build/licor
