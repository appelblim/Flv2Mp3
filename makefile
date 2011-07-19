Flv2Mp3: spawn_process.c main.c
	gcc -o Flv2Mp3 spawn_process.c main.c `pkg-config --cflags --libs gtk+-3.0`
