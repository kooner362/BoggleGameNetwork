#ifndef GAME_H
#define GAME_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#define MAX_NAME 10     // Max playername 


//Node for linked list which stores all players
typedef struct player {
    char name[MAX_NAME];  
    int max_score;
    int total_games;
    int total_score; 
    int score_submitted;
    int game_started;
    struct player *next;
} Player;


/*
 * Create a new player with the given name.  Insert it at the tail of the list
 * of players whose head is pointed to by *player_ptr_add.
 *
 * Return:
 *   0 if successful
 *   1 if a player by this name already exists in this list
 *   2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator)
 */
int add_player(const char *name, Player **user_ptr_add);

/*
 * Return a pointer to the player with this name in
 * the list starting with head. Return NULL if no such user exists.
 */
Player *find_player(const char *name, const Player *head);


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
void list_players(const Player *curr, int fd);

/*
 * Print a player profile.
 *   returns
 *   - 0 on success.
 *   - 1 if the player is NULL.
 */
int print_player( Player *player, int fd);

/*
 * Finds the player and updates player score
 *
 * Return:
 *   - 0 on success
 *   - 1 if player is not in the list
 */
int add_score(char *name, int score, const Player *head);

/*
 *Prints the names and scores of all time top 3
 *
 *
 */
void top_3(Player **player_ptr_add, int fd);

#endif
