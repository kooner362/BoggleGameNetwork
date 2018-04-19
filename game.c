#include "game.h"
#include "game_server.h"



/*
 * Create a new player with the given name.  Insert it at the tail of the list 
 * of players whose head is pointed to by *player_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a player by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int add_player(const char *name, Player **player_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    Player *new_player = malloc(sizeof(Player));
    if (new_player == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_player->name, name, MAX_NAME); // name has max length MAX_NAME - 1
    new_player->total_games =0;
    new_player->total_score =0;
    new_player->max_score =0;
    new_player->game_started = 0;
    new_player->score_submitted = 1;
    new_player->next = NULL;    

    // Add player to the end of the list
    Player *prev = NULL;
    Player *curr = *player_ptr_add;
     if (curr == NULL) {
        *player_ptr_add = new_player;
        return 0;
    }
    else{
	while (curr != NULL && strcmp(curr->name, name) != 0) {
	    prev = curr;
	    curr = curr->next;
	}

	if (curr != NULL) { //already exists
	    free(new_player);
	    return 1;
	} else {
	    prev->next = new_player;
	    return 0;
	}
    }
}


/* 
 * Return a pointer to the player with this name in
 * the list starting with head. Return NULL if no such player exists.
 *
 * NOTE: You'll likely need to cast a (const Player *) to a (Player *)
 * to satisfy the prototype without warnings.
 */
Player *find_player(const char *name, const Player *head) {
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (Player *)head;
}

/*
 * Print the playernames of all players in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
void list_players(const Player *curr, int fd) {
    char output[INPUT_BUFFER_SIZE];
    sprintf(output,"Player List\n");
    write_to_socket(output, strlen(output), fd);
    memset(output, 0, strlen(output));
    sprintf(output,"\tPlayerName\tTotalGames\tTotalScore\tMaxScore\n");
    write_to_socket(output, strlen(output), fd);
    memset(output, 0, strlen(output));
    while (curr != NULL) {
        sprintf(output, "\t     %s    \t    %d     \t    %d     \t    %d     \n", curr->name, curr->total_games, curr->total_score, curr->max_score);
        write_to_socket(output, strlen(output), fd);
        memset(output, 0, strlen(output));
        curr = curr->next;
    }
    
}

/* 
 * Print player stats * 
 *   - 0 on success.
 *   - 1 if the player is NULL.
 */
int print_player ( Player *p, int fd) {
    char output[INPUT_BUFFER_SIZE];
    if (p == NULL) {
        return 1;
    }
    // Print name
    sprintf(output, "Name: %s\n\n", p->name);
    write_to_socket(output, strlen(output), fd);
    memset(output, 0, strlen(output));
    sprintf(output, "------------------------------------------\n");
    write_to_socket(output, strlen(output), fd);
    memset(output, 0, strlen(output));
    // Print player stats.
    sprintf(output, "total games:%d, total points:%d best score:%d\n", p->total_games, p->total_score, p->max_score );
    write_to_socket(output, strlen(output), fd);
    memset(output, 0, strlen(output));
    sprintf(output, "------------------------------------------\n");
    write_to_socket(output, strlen(output), fd);

    return 0;
}



/*
 * Finds the player and updates player score
 *
 * Return:
 *   - 0 on success
 *   - 1 if player is not in the list
 */
int add_score(char *name, int score, const Player *player_list){
    Player *player = find_player (name, player_list );
    if (player == NULL){
        return 1;
    } 
    player->total_games ++;
    player->total_score += score;
    if (player->max_score < score)
        player->max_score = score;
    return 0;
}

/*
*Print top 3 all-time player names and score
*
*/

void top_3(Player **player_ptr_add, int fd){
    int high = 0, high1 = 0, high2 = 0;
    char *name, *name1, *name2;
    char output[INPUT_BUFFER_SIZE];
    int total_players = 0;
    Player *curr = *player_ptr_add;
    while(curr != NULL){
        total_players++;
        if(curr->max_score >= high){
            high2 = high1;
            name2 = name1;
            high1 = high;
            name1 = name;
            high = curr->max_score;
            name = curr->name;
        }
        else if(curr->max_score >= high1){
            high2 = high1;
            name2 = name1;
            high1 = curr->max_score;
            name1 = curr->name;
        }
        else if(curr->max_score >= high2){
            high2 = curr->max_score;
            name2 = curr->name;
        }
        curr = curr->next;
    }
    sprintf(output, "----- Top 3 -----\n");
    write_to_socket(output, strlen(output), fd);
    memset(output, 0, strlen(output));
    if (total_players == 0){
        sprintf(output, "No player data\n");
        write_to_socket(output, strlen(output), fd);
        memset(output, 0, strlen(output));
    }
    else if (total_players == 1){
        sprintf(output, "%s %d\n", name, high);
        write_to_socket(output, strlen(output), fd);
        memset(output, 0, strlen(output));
    }
    else if (total_players == 2){
        sprintf(output, "%s %d\n%s %d\n", name, high, name1, high1);
        write_to_socket(output, strlen(output), fd);
        memset(output, 0, strlen(output));
    }
    else{
        sprintf(output, "%s %d\n%s %d\n%s %d\n", name, high, name1, high1, name2, high2);
        write_to_socket(output, strlen(output), fd);
        memset(output, 0, strlen(output));
    }
}
