PORT=57738
CFLAGS = -DPORT=$(PORT) -D_XOPEN_SOURCE=500 -Wall -Werror -std=c99 -o


main: game_server.c game.c read_line.c generator.c game_server.h game.h read_line.h generator.h
	gcc $(CFLAGS) game_server game_server.c game.c read_line.c generator.c -I.
