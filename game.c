/*  Game C file
 * Author: T. Hadler, S. Przychodzko
 * Date:   17 Nov 2018
 * Descr:  Main file for maze game
 */

#include <avr/io.h>
#include "ir_uart.h"
#include "system.h"
#include "button.h"
#include "pacer.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "navswitch.h"
#include <stdlib.h>

/* Define polling rates in Hz.  */
#define NAVSWITCH_TASK_RATE 100
#define DISPLAY_TASK_RATE 250
#define PACER_RATE 500

#define PLAYER_FLASH_RATE 2
#define ENEMY_FLASH_RATE 5

#define MESSAGE_RATE 10

#define HEIGHT 31
#define WIDTH 32

#define ENEMY_VISIBILITY 10
#define OBJECTS_NUMBER 20

/* Dataset of the 31 x 31 map, which is stored as an array of 
 * hexadecimal characters to reduce storage space.  */
int map[62] = {0x7fff, 0xffff, 0x4000, 0x0001, 0x577d, 0x75fd, 
0x5405, 0x5505, 0x57df, 0xdddd, 0x5050, 0x0011, 0x5fdf, 0x5ff5, 
0x4401, 0x5005, 0x75fd, 0xd577, 0x5505, 0x5551, 0x575d, 0x5f5d, 
0x4011, 0x0005, 0x5dd7, 0x7ffd, 0x5154, 0x5001, 0x5d75, 0x5dfd, 
0x4505, 0x5105, 0x75dd, 0x5f75, 0x5451, 0x4045, 0x5f57, 0xdf5d, 
0x4154, 0x1151, 0x5d77, 0xd755, 0x5400, 0x1445, 0x5777, 0xf57d, 
0x4514, 0x0541, 0x5d55, 0x755d, 0x5155, 0x5551, 0x5777, 0x5d75, 
0x5400, 0x4005, 0x577f, 0x7ffd, 0x4040, 0x0001, 0x7fff, 0xffff};

/* Define pointer that will be initialised to point to the map dataset 
 * once the game begins. */
int* nodes;
    
/* Define static variables that are used in most modules so were 
 * initialised as globals for ease of coding. */
static bool player_flash;
static bool enemy_flash;
static int change_score;
static bool finished_loading;
static bool game_over;
static int init_rand;
static bool start_game;

/* Define the Position struct which consists of the x and y coordinates 
 * of the variable of interest. */
typedef struct position_s {
    int x;
    int y;
} Position;

/* Initialise an array for the route algorithm used by the enemy and 
 * define the direction array for the algorithm looping. */
Position route[ENEMY_VISIBILITY+1] = {0};
int directions[8] = {1, 0, -1, 0, 0, 1, 0, -1};

/* Create the Position struct of a variable from the given coordinates, 
 * used to initialise all objects in the game. */
Position position_init (int x, int y)
{
    Position position;
    position.x = x;
    position.y = y;
    return position;
}

/* Define the player and enemy variables as Position struct objects. */
Position player1;
Position enemy;

/* Initialise Position array of all the objects in the game. */
Position objects[OBJECTS_NUMBER] = {0};

/* Initialise a score array. */
char score[2] = {'0', '0'};

/* Function that takes the desired coordinates and decodes map dataset 
 * into binary, taking the desired binary value and returning whether 
 * or not it is available for movement (space or a wall). */  
int move(int x, int y)
{
    int n = 0;
    n = nodes[2 * y + (x / 16)];
    int k = 0;
    if (x >= 16)
        x = x - 16;
    k = n >> (15-x);
    if (k & 1) {
        return 1;
    } else {
        return 0;
    }
}

/* Function to initialise and configure the UCFK4 to the desired 
 * settings. */
void game_init(void)
{
    system_init();
    navswitch_init ();
    ir_uart_init ();
    pacer_init(PACER_RATE);
    
    // Set display up for scrolling message of intro
    tinygl_init (DISPLAY_TASK_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
}

/* Function to display a scrolling intro message which disappears when 
 * either player pushs the navswitch to start the game. */
void intro(void)
{
    tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    
    char* intro_message = "MAZE!";
    tinygl_text(intro_message);
    
    init_rand = 0;
    start_game = 0;
    while ((!navswitch_push_event_p(NAVSWITCH_PUSH)) && (!start_game)) {
        pacer_wait ();
        
        /* looping variable that gives random integer when game starts 
         * for random object placement. */
        if (init_rand > 1000) {
            init_rand = 0;
        } else {
            init_rand++;
        }
        // checks IR receiver if other player has pushed navswitch
        if (ir_uart_read_ready_p ()) {
            if (ir_uart_getc() == '1') {
                start_game = 1;
            }
        }
        tinygl_update ();
        navswitch_update ();
    }
    // sends IR message to other UCFK4 to start game
    ir_uart_putc('1');
}

/* Function that sets the initial position for all variables, using the 
 * loop value at the time the navswitch was pushed to randomise the 
 * result, checking if position attempts are valid. */
void set_init_pos(void)
{
    int max = OBJECTS_NUMBER+2;
    int index = -2;
    for (int i = 0; i < max; i++) {
        srand(init_rand+i);
        int x = (rand() % 29) + 2;
        srand((2*init_rand)+i);
        int y = (rand() % 30) + 1;
        if (!move(x,y)) {
            if (index == -2) {
                player1 = position_init(x,y); // set player position first
            } else if (index == -1) {
                enemy = position_init(x,y); // set enemy position second
            } else {
                objects[index].x = x; // set object positions last
                objects[index].y = y;
            }
            index++;
        } else {
            max++; // if position is unavailable try again with next i value 
        }
    }
}

/* Function to display a scrolling outro message which appears once 
 * one player has won the game, displaying a different message for each 
 * player. */
void outro(void)
{
    tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_clear();
    
    char* ending_message;
    
    if (score[0] > score[1]) {
        ending_message = "WINNER!";
    } else {
        ending_message = "LOSER! :(";
    }
    
    tinygl_text(ending_message);
    
    while (1) {
    pacer_wait ();
        
    tinygl_update ();
    navswitch_update ();
    }
}
    
/* Function for receiving IR communications, and if the transmittion is 
 * valid sets the score to the new values received and indicates to 
 * display the new score. */
void receive(void)
{
    if (ir_uart_read_ready_p ()) {
        if (ir_uart_getc() == 'w') {
            ir_uart_putc('r'); // if receiving message send confirmation
            char p1 = ir_uart_getc();
            char p2 = ir_uart_getc();
            // check if given values are valid
            if ((p1 >= '0') && (p1 <= '4') && (p2 >= '0') && (p2 <= '4')) {
                if ((p1 != score[0]) || (p2 != score[1])) {
                    score[1] = p1;
                    score[0] = p2; // update score and display
                    change_score = 250;
                    if ((score[0] > '3') || (score[1] > '3')) {
                        game_over = 1; // check end condition
                    }
                }
            }
        }
    }
}

/* Function for sending IR communications of the player score change. */
void send(void)
{
    char p1_score[3] = {score[0], score[1], '\0'};
    int read = 0;
    ir_uart_putc('w');
    while (!read) { // wait until confirmation
        if (ir_uart_getc() == 'r') { // if receive confirmation send message
            ir_uart_puts(p1_score);
            read = 1;
        }
    }
}
    

/* Function for finding out if player is on an object, and if true moves 
 * the object to a new random location. */
bool is_on_object(Position position, int loop)
{
    int max = 1;
    bool taken = 0;
    bool on_object = 0;
    
    for (int i = 0; i < OBJECTS_NUMBER; i++) {
        if (position.x == objects[i].x && position.y == objects[i].y) {
            on_object = 1; // check all objects

            /* change object using random loop value and continue testing 
             * until valid position is found. */
            for (int j = 0; j < max; j++) {
                srand(loop+j);
                int x = (rand() % 29) + 2;
                srand((2*loop)+j);
                int y = (rand() % 30) + 1;
                if (!move(x,y)) {
                    taken = 0;
                    for (int k = 0; k < OBJECTS_NUMBER; k++) {
                        if ((objects[k].x == x) && (objects[k].x == x)) {
                            taken = 1; // make sure two objects not on same spot
                        }
                    }
                    if (!taken) {
                        objects[i].x = x;
                        objects[i].y = y;
                    } else {
                        max++;
                    }
                } else {
                    max++;
                }
            }
        }
    }
    return on_object;
}  

/* Function for navswitch operations, checks if the direction or pick up 
 * object command is valid and if true updates the game accordingly. */
void navswitch_task(int loop)
{
    navswitch_update ();
    int x = player1.x;
    int y = player1.y;
    if (navswitch_push_event_p (NAVSWITCH_NORTH)) {
        if ((y > 0) & !(move(x,(y-1)))) {
            player1.y--;
        }
    }

    if (navswitch_push_event_p (NAVSWITCH_SOUTH)) {
        if ((y < HEIGHT) & !(move(x,(y+1)))) {
            player1.y++;
        }
    }
        
    if (navswitch_push_event_p (NAVSWITCH_EAST)) {
        if ((x < WIDTH) & !(move((x+1),y))) {
            player1.x++;
        }
    }
        
    if (navswitch_push_event_p (NAVSWITCH_WEST)) {
        if ((x > 1) & !(move((x-1),y))) {
            player1.x--;
        }
    }
    
    // picking up object event, update score, send IR and display
    if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
        if (is_on_object(player1, loop)) {
            tinygl_clear();
            score[0]++;
            send();
            change_score = 250;
            if (score[0] > '3') {
                game_over = 1; // check end condition
            }
        }
    }
}

/* Function that is activated when the enemy lands on the player, 
 * gives the opponent a point, activates IR communications and sends the 
 * player to a random location. */
void player_killed(int loop)
{
    tinygl_clear();
    score[1]++;
    send();
    change_score = 250;
    int max = 1;
    // random placement of player using loop value, repeating until valid
    for (int i = 0; i < max; i++) { 
        srand(loop+i);
        int x = (rand() % 29) + 2;
        srand((2*loop)+i);
        int y = (rand() % 30) + 1;
        if (!move(x,y)) {
            player1 = position_init(x,y);
        } else {
            max++;
        }
    }
    if (score[1] > '3') {
        game_over = 1; // check end condition
    }
}

/* Function that updates the map around the player that is located at 
 * the middle of the LED display and checks if for player killed and 
 * change in score events. */
void update (int loop)
{
    if ((player1.x == enemy.x) && (player1.y == enemy.y)) {
        player_killed(loop); // check if enemy on player
    }
    if (change_score) {
        tinygl_text(score); // check if display score event is active
        change_score--;
    } else {
        tinygl_clear();
        tinygl_draw_point(tinygl_point (2, 3), player_flash);
        int k = 0;
        // iterate through 5 x 7 LED grid with player offset
        for ( int i = 0; i < 5; i++ ) {
            for ( int j = 0; j < 7; j++ ) {
                if (((i+player1.x-2) >= 0) & ((i+player1.x-2) < WIDTH)) {
                    if (((j+player1.y-3) >= 0) & ((j+player1.y-3) < HEIGHT)) {
                        for (k = 0; k < OBJECTS_NUMBER; k++) {
                            if (((i+player1.x-2) == objects[k].x) & ((j+player1.y-3) == objects[k].y)) {
                                tinygl_draw_point(tinygl_point (i, j), !enemy_flash);
                                }
                            }
                        if (((i+player1.x-2) == enemy.x) & ((j+player1.y-3) == enemy.y)) {
                            tinygl_draw_point(tinygl_point (i, j), enemy_flash);
                        } else if (move((i+player1.x-2),(j+player1.y-3))) {
                            tinygl_draw_point(tinygl_point (i, j), 1);
                        }
                    }
                }
            }
        }
    }
    tinygl_update ();
}

/* Recursive function used by the enemy to iterate through all viable 
 * routes within the enemy visibility range until it finds a route to 
 * the player */
int algorithm(int i, int found)
{
    i++;
    if (i >= ENEMY_VISIBILITY) {
        return 0;
    }
    int x = route[i].x;
    int y = route[i].y;
    for (int d = 0; d < 8; d++) {
        if (!move(x+directions[d],y+directions[d+1])) {
            if (((x+directions[d]) != route[i-1].x) || ((y+directions[d+1]) != route[i-1].y)) {
                route[i+1].x = x + directions[d]; // put new coordinates into route array
                route[i+1].y = y + directions[d+1];
                if ((route[i+1].x == player1.x) && (route[i+1].y == player1.y)) {
                    return 1; // if found player return 1
                }
                found = algorithm(i, found); // if direction valid take next step
                if (found) {
                    return 1; // if found player return 1
                }
            }
        }
    d++;
    }
    i--;
    return 0;
}
    
/* Function for the enemy movement that uses the algorithm function 
 * and sets the next movement of the enemy accordingly. */
void update_enemy(void)
{
    int found = 0;
    route[0].x = enemy.x;
    route[0].y = enemy.y; // initialise route array
    int i = 0;
    int x = route[i].x;
    int y = route[i].y;
    for (int d = 0; d < 8; d++) { // check all 4 potential directions
        if (!move(x+directions[d],y+directions[d+1])) {
            route[i+1].x = x + directions[d];
            route[i+1].y = y + directions[d+1]; // if valid put new coordinates into array
            if ((route[i+1].x == player1.x) && (route[i+1].y == player1.y)) {
                found = 1;
                enemy.x = route[1].x;
                enemy.y = route[1].y; // if found player confirm enemy movement step
                break;
            }
            found = algorithm(i, found);
            if (found) {
                enemy.x = route[1].x;
                enemy.y = route[1].y; // if found player confirm enemy movement step
                break;
            }
        }
        d++;
    }
}

/* Main function of the program that sets initial conditions using the 
 * initialisation functions, and loops loops through the various tasks 
 * using a pacer procedure method until the game over event is 
 * triggered then calls the outro function to finish the game. */
int main ( void )
{
    game_over = 0;
    game_init();
    
    enemy = position_init(30,1);
    finished_loading = 0;

    intro();
    // change message configuration to suit score display in game
    tinygl_text_speed_set (10);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    while(!game_over)
    {
        for (int loop = 1; loop < 1001; loop++) {
            pacer_wait();
            if (navswitch_push_event_p(NAVSWITCH_PUSH) || start_game) {
                if (!finished_loading) { // initialise variables positions
                    set_init_pos();
                    nodes = map;
                    finished_loading = 1;
                    change_score = 0;
                }
            }
            if (finished_loading) {
                receive();
                if (loop % (PACER_RATE/DISPLAY_TASK_RATE) == 0) {
                    update(loop);
                }
                if (loop % (PACER_RATE/NAVSWITCH_TASK_RATE) == 0) {
                    if (!change_score) {
                        navswitch_task(loop);
                    }
                }
                if (loop % (PACER_RATE/PLAYER_FLASH_RATE) == 0) {
                    update_enemy();
                    player_flash = !player_flash;
                }
                if (loop % (PACER_RATE/ENEMY_FLASH_RATE) == 0) {
                    enemy_flash = !enemy_flash;
                }
            }
        }
    }
    outro();
}
