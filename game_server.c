#include "game_server.h"
#define INPUT_ARG_MAX_NUM 12
#define DELIM "> \r\n"
#define MAX_CLIENTS 30
#ifndef PORT
  #define PORT 30000
#endif
char *board; //global variable for game board.
int sfd; //global variable for socket file descriptor
int game_in_progress = 0; //0 if new_game has not been started, 1 otherwise.
struct itimerval game_t; //timer struct
/*
 * Client struct
 * Code by Alan Rosenthal
 */
struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;
}*top = NULL; //intialize head of clients

/* 
 * Write stdout of all connected clients.
 * 
 */
void broadcast(char *s, int size){
    struct client *p;
    for (p = top; p; p = p->next){
	if(write(p->fd, s, size) == -1){
            perror("write error");
        }
    }
}

/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 * Code by Alan Rosenthal
 */
void addclient(int fd, struct in_addr addr){
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
	fprintf(stderr, "out of memory!\n");  /* highly unlikely to happen */
	exit(1);
    }
    printf("Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);
    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    top = p;
}

/* 
 * Removes client from client list.
 * Code by Alan Rosenthal.
 */
void removeclient(int fd){
    struct client **p;
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
	;
    if (*p) {
	struct client *t = (*p)->next;
	printf("Removing client %s\n", inet_ntoa((*p)->ipaddr));
	fflush(stdout);
	free(*p);
	*p = t;
    } 
    else {
	fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
	fflush(stderr);
    }
}

/* 
 * Write to stdout of the current client.
 * 
 */
void write_to_socket(char *s, int size, int fd){
    if(write(fd, s, size) == -1){
        perror("write error");
    }
}

/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
int process_args(int cmd_argc, char **cmd_argv, Player **player_list_ptr, char* username, int fd) {
    char output[INPUT_BUFFER_SIZE];
    Player *player_list = *player_list_ptr;
    Player *current_player;
    current_player = find_player(username, player_list);
    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        sprintf(output, "Thanks for playing. Goodbye!\n");
        write_to_socket(output, strlen(output), fd);
        return -1;
    } else if (strcmp(cmd_argv[0], "add_player") == 0 && cmd_argc == 2) {
        switch (add_player(cmd_argv[1], player_list_ptr)) {
            case 1:
                sprintf(output, "player by this name already exists\n");
		        write_to_socket(output, strlen(output), fd);
                break;
            case 2:
                sprintf(output, "playername is too long\n");
		        write_to_socket(output, strlen(output), fd);
                break;
        }

    } else if (strcmp(cmd_argv[0], "all_players") == 0 && cmd_argc == 1) {
        list_players(player_list, fd);
    } else if (strcmp(cmd_argv[0], "print_player") == 0 && cmd_argc == 2) {
        Player * player = find_player (cmd_argv[1], player_list );
        if (print_player(player, fd) == 1) {
            sprintf(output, "player not found\n");
	    write_to_socket(output, strlen(output), fd);
        }
    } else if (strcmp(cmd_argv[0], "add_score") == 0 && cmd_argc == 2 && current_player->score_submitted == 0) {
        if (add_score(current_player->name, atoi(cmd_argv[1]),player_list) == 1) {
            sprintf(output, "player not found\n");
	    write_to_socket(output, strlen(output), fd);
        }
        current_player->game_started = 0;
        current_player->score_submitted = 1;
    } else if (strcmp(cmd_argv[0], "top_3") == 0 && cmd_argc == 1) {
        top_3(player_list_ptr, fd);
    } else if (strcmp(cmd_argv[0], "new_game") == 0 && cmd_argc == 1 && current_player->game_started == 0){
        current_player->game_started = 1;
        current_player->score_submitted = 0;
        if (game_in_progress == 0){
            if(setitimer(ITIMER_REAL, &game_t, NULL) != 0){
	        perror("setitimer");
	    }
            game_in_progress = 1;
        }
        int i;
        for(i = 0; i < 16; i+=4){
            sprintf(output, "%c%c%c%c\n",board[i], board[i+1], board[i+2], board[i+3]);
            write_to_socket(output, strlen(output), fd);
        }
    } else {
        if(strcmp(cmd_argv[0], "add_score") == 0 && current_player->score_submitted == 1){
            sprintf(output, "Run new_game to add your score\n");
        }
        else if(strcmp(cmd_argv[0], "new_game") == 0 && current_player->game_started == 1){
            sprintf(output, "Submit a score before starting a new game.\n");
        }
        else{
            sprintf(output, "Incorrect syntax\n");
        }
        write_to_socket(output, strlen(output), fd);
    }
    return 0;
}

/* 
 * Print a formatted error message to stderr.
 */
void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

/* 
 * Handles SIGALRM. Generates new game board.
 * Writes game over message to all connected clients 
 */
void sig_handler(int signo){
    char output[INPUT_BUFFER_SIZE];
    if (signo == SIGALRM){
        board_generator(&board);
        game_in_progress = 0;
        sprintf(output, "Game Over!\n>");
        broadcast(output, strlen(output));
    }
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!");
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}

int main(int argc, char* argv[]) {
    int op = 1;
    int master_socket , addrlen , new_socket , client_socket[30], activity, i;
    int max_sfd;
    char *message ="Welcome to Game server! (Network version)\n Enter name to play:\n>";
    int status[MAX_CLIENTS];
    char input[INPUT_BUFFER_SIZE];
    char output[INPUT_BUFFER_SIZE];
    char player_name[MAX_CLIENTS][MAX_NAME]; //2D array of connected player names
    Player *player_list = NULL; //Linked list of all players to play on the server
    struct sigaction time_signal;
    time_signal.sa_handler = sig_handler;
    time_signal.sa_flags = SA_RESTART;
    struct timeval to;
    to.tv_sec = 0;
    to.tv_usec = 0;
    game_t.it_value.tv_sec = 120;
    game_t.it_value.tv_usec = 0;
    //set of socket descriptors
    fd_set readfds;
    struct sockaddr_in address;
    board_generator(&board); //Create new game board
    /* Following code from Alan Rosenthal
     * also used http://www.binarytides.com/multiple-socket-connections-fdset-select-linux/
     * to supplement code provided by Rosenthal
     */
    //initialize all client_socket[] to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket");
        exit(1);
    }
    //set master socket to allow multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op)) < 0 ){
        perror("setsockopt");
        exit(1);
    }
  
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
      
    //bind the socket to localhost port 56361
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind");
        exit(1);
    }
    printf("Listening on port %d \n", PORT);
     
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0){
        perror("listen");
        exit(1);
    }
      
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for client connections ...");
    //SIGALRM Handler
    sigaction(SIGALRM, &time_signal, NULL);
	
    while(1){
        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sfd = master_socket;
        //add child sockets to set
        for (i = 0 ; i < MAX_CLIENTS ; i++) {
            //socket descriptor
            sfd = client_socket[i];
            //if valid socket file descriptor then add to read list
            if(sfd > 0)
                FD_SET( sfd , &readfds);
            //highest file descriptor number, need it for the select function
            if(sfd > max_sfd)
                max_sfd = sfd;
        }
        //Setup for signal mask for SIGALRM
	sigset_t signal_block;
	sigemptyset(&signal_block);
	sigaddset(&signal_block, SIGALRM);
        //Block SIGALRM signal
	sigprocmask(SIG_BLOCK, &signal_block,NULL);
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely	
        activity = select( max_sfd + 1 , &readfds , NULL , NULL , &to);
        if ((activity < 0) && (errno!=EINTR)){
            printf("select error");
        }
	sigprocmask(SIG_UNBLOCK, &signal_block,NULL);
		
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)){
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
                perror("accept");
                exit(EXIT_FAILURE);
            }
        
            //send new connection greeting message
            if(send(new_socket, message, strlen(message), 0) != strlen(message)) {
                perror("send");
            }
              
            //add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                //if position is empty
                if( client_socket[i] == 0 ){
                    status[i] = 100;
                    sfd = (client_socket[i] = new_socket);
                    addclient(sfd, address.sin_addr);
                    break;
                }
            }
        }
        //loop through all connected clients if fd is set then read user input
        for (i = 0; i < MAX_CLIENTS; i++){
            sfd = client_socket[i];
            if (FD_ISSET(sfd, &readfds)){		    
                if(readLine(sfd, input, INPUT_BUFFER_SIZE - 1) != 0 && status[i] != -1){
		    char *cmd_argv[INPUT_ARG_MAX_NUM];
		    int cmd_argc = tokenize(input, cmd_argv);
                    //Login player if user does not exist create new player
		    if(status[i] == 100){
		        if(strlen(cmd_argv[0]) > MAX_NAME){
		       	    sprintf(output, "playername is too long\n");
			    write_to_socket(output, strlen(output), sfd);
			    memset(output, 0, strlen(output));
			}
			add_player(cmd_argv[0], &player_list);
			memset(player_name[i], '\0' , MAX_NAME);
			strncpy(player_name[i], cmd_argv[0], MAX_NAME-1);
			sprintf(output, "Welcome %s!\n", player_name[i]);
			write_to_socket(output, strlen(output), sfd);
			memset(output, 0, strlen(output));
			sprintf(output, "Please type a command: <all_players>, <add_score score>, <top_3>, <new_game> or <quit>\n>");
			write_to_socket(output, strlen(output), sfd);
			memset(output, 0, strlen(output));
			status[i] = 200;
		    }     
                    //If user enters "quit" removed user from client list and player_name array 
		    else if (cmd_argc > 0 && (status[i] = process_args(cmd_argc, cmd_argv, &player_list, player_name[i], sfd)) == -1) { 
                        memset(player_name[i], '\0', strlen(player_name[i]));
                        //Close the socket and mark as 0 in list for reuse
		        removeclient(sfd);
		        close(sfd);
		        client_socket[i] = 0;
		    }	
                    else{
                        sprintf(output, "> ");
		        write_to_socket(output, strlen(output), sfd);
		        memset(output, 0, strlen(output));
                    }
		    
                }
                else{
                    //Somebody disconnected remove them from client list and close socket file descriptor
                    memset(player_name[i], '\0', strlen(player_name[i]));
                    //Close the socket and mark as 0 in list for reuse
                    removeclient(sfd);
                    close(sfd);
                    client_socket[i] = 0;
                }		
            }    
        }       
    }
  return 0;
}
