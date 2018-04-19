#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "generator.h"

//Returns a random face on a die
void get_random_face(int* random_face_ptr){
    *random_face_ptr = rand() % 6;
}

//Returns a random dice
void get_random_dice(int* random_dice_ptr){
    random_dice_ptr = malloc(sizeof(int));
    *random_dice_ptr = rand() % 16;
}

//Generates board 
void board_generator(char **tiles) {
	const char *dice[16] = {"RIFOBX", "IFEHEY", "DENOWS", "UTOKND", "HMSRAO", "LUPETS", "ACITOA", 
		"YLGKUE","QBMJOA", "EHISPN", "VETIGN", "BALIYT", "EZAVND", "RALESC", "UWILRG", "PACEMD"};
	int random_face;
	int random_dice;
	int used_dice[16];
    *tiles = malloc(sizeof(char) * 17);
    char* tiles_pt = *tiles;
    int i;
	
	for (i=0; i<16; i++) {	
		used_dice[i] = 0;
	}
	srand(time(NULL));
    
	for (i=0; i<16; i++) {
		random_face = rand() % 6;	//assign a random face 0-5		
		random_dice = rand() % 16;	//assigns a random dice 0-15
		
		//if dice is unused, checks it as used and puts value in tiles
		if (used_dice[random_dice] == 0) { 
			used_dice[random_dice] = 1;
			tiles_pt[random_dice] = dice[random_dice][random_face];
		}
		else {	
		     //if dice is already used, looks for another unused dice
			while (used_dice[random_dice] == 1 && i < 16) {
				if (random_dice + 1 != 16){
				    random_dice++;
				}
				else{
				    random_dice = 0;
				}
			}
			used_dice[random_dice] = 1;
			tiles_pt[random_dice] = dice[random_dice][random_face];
		}
	}	
}