# As LDAhid is missing in git and not under open source
# license you will need to download it from LabView website

# TODO: add dependecies checks for libus, gtk and the other
# 	stuff needed for this Program to run

CC=gcc
CFLAGS=-lm -lpthread -lusb -lrt

all: clean_up clean console lab_brick clean

lab_brick: ldahid.o control.o input.o

ldahid.o: LDAhid.c LDAhid.h
	$(CC) -o ldahid.o -c LDAhid.c

control.o: control.c control.h
	$(CC) -o control.o -c control.c

input.o: input.c input.h
	$(CC) -o input.o -c input.c

console: ldahid.o input.o control.o
	$(CC) -o attenuator_lab_brick  ldahid.o input.o control.o $(CFLAGS)

gui: clean console lab_brick gui.o

gui.o: control.o input.o lab_brick_gui.c
	$(CC) -o gui.o $(pkg-config --cflags --libs gtk+-3.0) -c lab_brick_gui.c

clean:
	rm -rf control.o
	rm -rf input.o
	rm -rf ldahid.o

clean_up:
	rm -rf attenuator_lab_brick
