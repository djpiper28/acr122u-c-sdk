all: 
	gcc sdk.c -o testsc `pkg-config libpcsclite --libs` -Wpedantic -Wall
