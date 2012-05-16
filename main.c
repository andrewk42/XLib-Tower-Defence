/* Tower Defence game for SE382 Assignment 1
 *
 * Written by Andrew Klamut; UWID 'ajklamut'
 * Last updated January 22, 2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* For usleep() */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* ---Begin custom struct definitions--- */
typedef struct {
    int x, y; //current position
    long count; //when this reaches zero, update position
    long speed; //the initial value of count
    char direction; //either 'L', 'R', 'U', 'D'
    int steps; //for keeping track of the enemy's progress along a path
    int damage; //for knowing how much to subtract from castle health
    int score; //the score gained from killing this enemy
    char path; //either 'L' or 'R'
    char type; //either 'B' for blue, 'G' for green, 'R' for red, or 'P' for purple
} Enemy;

typedef struct {
    long count; //when this reaches zero, send the next enemy
    long speed; //the initial value of count
    int enemy_total; //how many enemies are in this wave
    int enemy_count; //for counting up to the total
    int score; //what score enemies are worth for this round
    char types[100]; //the types of enemies to be spawned in this wave, in order
} Level;

typedef struct {
    int x, y; //where this node is
    char direction_mod; //what direction enemies should inherit
} PathNode;

typedef struct {
    int x, y; //where the shot is centred
    int size; //maximum size;
    int count; //for counting up to 'size'
} WaveGunShot;

typedef struct {
    int x, y; //one endpoint of the laser
    int length;
    long speed;
    char direction;
} LaserGunShot;

typedef struct {
    int x, y; //coordinates of box's top-left corner
    int last_x, last_y; //for redrawing bg
    int length; //length of each side
    long speed;
} RocketShot;
/* ---End custom struct defintions--- */


/* ---Begin global variable declarations--- */
Display* display;
int screen, screen_width, screen_height, screen_depth;
Colormap screen_colormap;
Visual* main_visual;
Window main_win;
GC main_gc;
XColor bg_screen_color, bg_exact_color;
Pixmap bg_bitmap;
unsigned int bg_bitmap_width, bg_bitmap_height;
int bg_bitmap_x, bg_bitmap_y;
Enemy* enemies = NULL;
int num_enemies = 0;
Level levels[10];
int num_levels = 0;
int level_counter = 0;
PathNode left_path[8];
PathNode right_path[8];
int health = 100; //player's castle health
int score = 0; //player's current score
int current_weapon = 1; //player's selected weapon
WaveGunShot wave; //player's current wavegun shot
LaserGunShot laser; //player's current laser shot
RocketShot rocket; //player's current rockets
int mouse_x = 0;
int mouse_y = 0; //keep track of last mouse co-ords
/* ---End global variable declarations--- */


/* ---Begin function forward declarations--- */
int init();
int splash();
int eventloop();
void gameEngine();
void writePathNodes();
void drawMaze();
void drawTopStrings();
void drawBottomStrings();
void drawSuccessPrompt();
void drawDeathPrompt();
void drawEndPrompt();
void enemyFactory(int, int, char, char, int);
void drawEnemy(int);
void enemyDestruct(int);
void drawExplosion(int, int, int);
void drawWave(int, int, int);
void drawLaser();
void drawRocket();
/* ---End function forward declarations--- */


int main(int argc, char* argv[]) {
    int status = 0;
    int show_splash = 1;
    int i;

    if (argc > 1) {
        if (!strcmp(argv[1], "-nosplash")) {
            show_splash = 0;
        }
    }

    status = init();

    if (status) return 1;

    if (show_splash) status = splash();

    if (!status && show_splash) fprintf(stderr, "splash() completed successfully!\n");
    else if (!status && !show_splash) fprintf(stderr, "splash() was skipped!\n");
    else return 2;

    /* Begin additional initializations */
    drawMaze();
    drawTopStrings();
    drawBottomStrings();
    writePathNodes();

    XSelectInput(display, main_win, ButtonPressMask | ButtonReleaseMask | ExposureMask | ButtonMotionMask | PointerMotionMask | KeyPressMask);

    level_counter = 0;

    /* Begin setting up level 1 */
    levels[0].speed = 4000;
    levels[0].count = 4000;
    levels[0].enemy_total = 10;
    levels[0].enemy_count = 0;
    levels[0].score = 10;

    for (i = 0; i < 10; i++) {
        levels[0].types[i] = 'G';
    }
    /* End setting up level 1 */

    /* Begin setting up level 2 */
    levels[1].speed = 900;
    levels[1].count = 900;
    levels[1].enemy_total = 20;
    levels[1].enemy_count = 0;
    levels[1].score = 20;

    for (i = 0; i < 20; i++) {
        if (i%3) levels[1].types[i] = 'G';
        else levels[1].types[i] = 'B';
    }
    /* End setting up level 2 */

    /* Begin setting up level 3 */
    levels[2].speed = 400;
    levels[2].count = 400;
    levels[2].enemy_total = 30;
    levels[2].enemy_count = 0;
    levels[2].score = 40;

    for (i = 0; i < 30; i++) {
        if (i%3) levels[2].types[i] = 'B';
        else levels[2].types[i] = 'G';
    }
    /* End setting up level 3 */

    /* Begin setting up level 4 */
    levels[3].speed = 400;
    levels[3].count = 400;
    levels[3].enemy_total = 40;
    levels[3].enemy_count = 0;
    levels[3].score = 50;
    
    for (i = 0; i < 40; i++) {
        if (i < 2) levels[3].types[i] = 'G';
        else if (i < 6) levels[3].types[i] = 'B';
        else {
            if (i%3) levels[3].types[i] = 'B';
            else {
                if (i%2) levels[3].types[i] = 'R';
                else levels[3].types[i] = 'G';
            }
        }
    }
    /* End setting up level 4 */

    /* Begin setting up level 5 */
    levels[4].speed = 400;
    levels[4].count = 400;
    levels[4].enemy_total = 50;
    levels[4].enemy_count = 0;
    levels[4].score = 50;
    
    for (i = 0; i < 50; i++) {
        if (i < 2) levels[4].types[i] = 'G';
        else if (i < 6) levels[4].types[i] = 'B';
        else {
            if (i%3) levels[4].types[i] = 'R';
            else {
                if (i%2) levels[4].types[i] = 'B';
                else levels[4].types[i] = 'G';
            }
        }
    }
    /* End setting up level 5 */

    /* Begin setting up level 6 */
    levels[5].speed = 350;
    levels[5].count = 350;
    levels[5].enemy_total = 60;
    levels[5].enemy_count = 0;
    levels[5].score = 100;
    
    for (i = 0; i < 60; i++) {
        if (i < 2) levels[5].types[i] = 'G';
        else if (i < 6) levels[5].types[i] = 'R';
        else {
            if (i%3) levels[5].types[i] = 'R';
            else {
                if (i%2) levels[5].types[i] = 'B';
                else levels[5].types[i] = 'R';
            }
        }
    }
    /* End setting up level 6 */

    /* Begin setting up level 7 */
    levels[6].speed = 350;
    levels[6].count = 350;
    levels[6].enemy_total = 70;
    levels[6].enemy_count = 0;
    levels[6].score = 100;
    
    for (i = 0; i < 70; i++) {
        if (i < 2) levels[6].types[i] = 'G';
        else if (i < 6) levels[6].types[i] = 'B';
        else {
            if (i%3) levels[6].types[i] = 'G';
            else {
                if (i%2) levels[6].types[i] = 'R';
                else levels[6].types[i] = 'B';
            }
        }
    }
    /* End setting up level 7 */

    /* Begin setting up level 8 */
    levels[7].speed = 300;
    levels[7].count = 300;
    levels[7].enemy_total = 70;
    levels[7].enemy_count = 0;
    levels[7].score = 100;
    
    for (i = 0; i < 70; i++) {
        if (i < 2) levels[7].types[i] = 'G';
        else if (i < 6) levels[7].types[i] = 'R';
        else {
            if (i%3) levels[7].types[i] = 'R';
            else {
                if (i%2) levels[7].types[i] = 'B';
                else levels[7].types[i] = 'R';
            }
        }
    }
    /* End setting up level 8 */

    /* Begin setting up level 9 */
    levels[8].speed = 300;
    levels[8].count = 300;
    levels[8].enemy_total = 80;
    levels[8].enemy_count = 0;
    levels[8].score = 120;
    
    for (i = 0; i < 80; i++) {
        if (i < 2) levels[8].types[i] = 'G';
        else if (i < 6) levels[8].types[i] = 'R';
        else {
            if (i%3) levels[8].types[i] = 'R';
            else {
                if (i%2) levels[8].types[i] = 'B';
                else levels[8].types[i] = 'R';
            }
        }
    }
    /* End setting up level 9 */

    /* Begin setting up level 10 */
    levels[9].speed = 300;
    levels[9].count = 300;
    levels[9].enemy_total = 90;
    levels[9].enemy_count = 0;
    levels[9].score = 150;
    
    for (i = 0; i < 90; i++) {
        if (i < 2) levels[9].types[i] = 'G';
        else if (i < 6) levels[9].types[i] = 'B';
        else {
            if (i%3) levels[9].types[i] = 'R';
            else {
                if (i%2) levels[9].types[i] = 'G';
                else levels[9].types[i] = 'B';
            }
        }
    }
    /* End setting up level 10 */

    wave.x = wave.y = wave.count = wave.size = 0;

    laser.x = laser.y = laser.length = laser.speed = 0;
    laser.direction = 'R';

    rocket.x = rocket.y = rocket.speed = rocket.length = 0;
    /* End additional initializations */

    /* Begin game */
    if (!status) status = eventloop(); else return 3;
    /* End game */

    free(enemies);

    return 0;
}


int init() {
    /* Begin connection to display */
    display = XOpenDisplay(":0");

    if (display == NULL) {
        fprintf(stderr, "CRASH: Can't connect to display\n");
        return 1;
    }
    /* End connection to display */

    /* Begin collection of screen properties */
    screen = DefaultScreen(display);
    screen_width = DisplayWidth(display, screen);
    screen_height = DisplayHeight(display, screen);
    screen_depth = DefaultDepth(display, screen);
    main_visual = DefaultVisual(display, screen);
    screen_colormap = DefaultColormap(display, DefaultScreen(display));

    if (screen_width < 640 || screen_height < 480) {
        fprintf(stderr, "CRASH: Your display must be at least 640x480!\nI've detected a screen of %d by %d. Please resize.\n", screen_width, screen_height);
        return 1;
    }
    /* End collection of screen properties */

    /* Begin creation of main window w/ graphics context */
    Status rc = XAllocNamedColor(display, screen_colormap, "dark slate gray", &bg_screen_color, &bg_exact_color);

    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'dark slate gray' color failed.\n");
        return 1;
    }

    XSetWindowAttributes attributes;
    attributes.background_pixel = bg_screen_color.pixel;

    main_win = XCreateWindow(display, RootWindow(display, screen), 0, 0, 640, 480, 10, screen_depth, InputOutput, main_visual, CWBackPixel, &attributes);

    main_gc = XCreateGC(display, main_win, 0, 0);
    XSetForeground(display, main_gc, WhitePixel(display, screen));

    if (main_gc < 0) {
        fprintf(stderr, "CRASH: GC init failed!\n");
        return 1;
    }
    XStoreName(display, main_win, "ajklamut_a1");
    /* End creation of main window w/ graphics context */

    XMapWindow(display, main_win);
    XSync(display, False);

    fprintf(stderr, "init() completed successfully!\n");
    fprintf(stderr, "    display = %d, screen = %d, screen_width = %d, screen_height = %d\n", display, screen, screen_width, screen_height);
    fprintf(stderr, "    screen_depth = %d, root_win = %d, main_gc = %d\n", screen_depth,  DefaultRootWindow(display), main_gc);
    fprintf(stderr, "    main_win = %d, main_win width = 640, main_win height = 480\n", main_win);

    return 0;
}


int eventloop() {
    XEvent an_event;
    int startx, starty;
    int x, y;

    while (1) {
        if (XCheckWindowEvent(display, main_win, ButtonPressMask | ButtonReleaseMask | ExposureMask | ButtonMotionMask | PointerMotionMask | KeyPressMask, &an_event)) {
            //XNextEvent(display, &an_event);
            switch (an_event.type) {
                case KeyPress:
                    /*
                    XCloseDisplay(display);
                    exit(0);*/
                    x = an_event.xkey.x;
                    y = an_event.xkey.y;
                    Window the_win = an_event.xkey.window;
                    KeySym key_symbol = XKeycodeToKeysym(display, an_event.xkey.keycode, 0);
                    fprintf(stderr, "event.xkey.x - %d, event.xkey.y - %d on window %d\n", x, y, the_win);
                    if (key_symbol >= XK_A && key_symbol <= XK_Z) {
                        int ascii_key = key_symbol - XK_A + 'A';
	                    fprintf(stderr, "Key pressed - '%c'\n", ascii_key);
                    }
                    if (key_symbol >= XK_a && key_symbol <= XK_z) {
		                int ascii_key = key_symbol - XK_a + 'a';
		                fprintf(stderr, "Key pressed - '%c'\n", ascii_key);
		            }
                    if (key_symbol == XK_q) {
                        return 0;
                    }
                    if (key_symbol == XK_1) {
                        current_weapon = 1;
                        //drawMaze();
                        //drawTopStrings();
                        drawBottomStrings();
                    }
                    if (key_symbol == XK_2) {
                        current_weapon = 2;
                        //drawMaze();
                        //drawTopStrings();
                        drawBottomStrings();
                    }
                    if (key_symbol == XK_3) {
                        current_weapon = 3;
                        //drawTopStrings();
                        drawBottomStrings();
                    }
                    if (((key_symbol >= XK_Left) && (key_symbol <= XK_Down)) || key_symbol == XK_Return) {
                        switch (key_symbol) {
                            case XK_Left:
                                fprintf(stderr, "Key pressed - LEFTARROW\n");
                                enemyFactory(75, 10, 'L', 'R', 0);
                                break;
                            case XK_Up:
                                fprintf(stderr, "Key pressed - UPARROW\n");
                                break;
                            case XK_Right:
                                fprintf(stderr, "Key pressed - RIGHTARROW\n");
                                enemyFactory(75, 10, 'R', 'R', 0);
                                break;
                            case XK_Down:
                                fprintf(stderr, "Key pressed - DOWNARROW\n");
                                break;
                            default:
                                fprintf(stderr, "Key pressed - ENTER\n");
                                drawSuccessPrompt();
                        }
                    }
                    break;
                case ButtonPress:
                    startx = an_event.xbutton.x;
                    starty = an_event.xbutton.y;
                    
                    if (startx < bg_bitmap_x+20 || startx > bg_bitmap_x+bg_bitmap_width-20) break; //prevent shooting on edges
                    if (starty > bg_bitmap_y+bg_bitmap_height-125) break; //prevent spawn camping
                    if (starty < bg_bitmap_y+75) break; //prevent shooting above castle

                    switch (current_weapon) {
                        case 3:
                            if (startx <= bg_bitmap_x+bg_bitmap_width/2 && starty <= bg_bitmap_y+bg_bitmap_height/2) { //top-left corner
                                rocket.x = bg_bitmap_x+50; //initialize new rocket
                                rocket.y = bg_bitmap_y+50;
                            }
                            else if (startx <= bg_bitmap_x+bg_bitmap_width/2 && starty > bg_bitmap_y+bg_bitmap_height/2) { //bottom-left corner
                                rocket.x = bg_bitmap_x+50;
                                rocket.y = bg_bitmap_y+bg_bitmap_height-50;
                            }
                            else if (startx > bg_bitmap_x+bg_bitmap_width/2 && starty <= bg_bitmap_y+bg_bitmap_height/2) { //top-right corner
                                rocket.x = bg_bitmap_x+bg_bitmap_width-50;
                                rocket.y = bg_bitmap_y+50;
                            }
                            else { //bottom-right corner
                                rocket.x = bg_bitmap_x+bg_bitmap_width-50;
                                rocket.y = bg_bitmap_y+bg_bitmap_height-50;
                            }
                            rocket.length = 5;
                            rocket.speed = 1;
                            break;
                        case 2:
                            wave.x = startx; //initialize new wavegun shot
                            wave.y = starty;
                            wave.size = 60;
                            wave.count = 0;
                            break;
                        default:
                            laser.x = startx; //initialize new laser shot
                            laser.y = starty;
                            laser.length = 20;
                            laser.speed = 5;
                            if (startx <= bg_bitmap_width/2 + 10) laser.direction = 'R';
                            else laser.direction = 'L';
                    }
                    break;
                case MotionNotify:
                    mouse_x = an_event.xmotion.x;
                    mouse_y = an_event.xmotion.y;
                    //fprintf(stderr, "motion notify detected, setting mouse_x=%d and mouse_y=%d\n", mouse_x, mouse_y);
                    /*
                    x = an_event.xmotion.x;
                    y = an_event.xmotion.y;
                    XDrawLine(display, main_win, main_gc, startx, starty, x, y);
                    startx = x;
                    starty = y;
                    */
                    break;
            }
        }
        gameEngine();
        if (health <= 0) {
            drawDeathPrompt();
            return 0;
        }

        switch (level_counter) {
            case 10:
                return 0;
            case 9:
            case 8:
            case 7:
            case 6:
            case 5:
            case 4:
            case 3:
            case 2:
                usleep(700);
            case 1:
            default:
                usleep(500);
        }
    }

    return 0;
}


void gameEngine() {
    int i;
    int j;
    int radius; //for wave gun collision detection
    int enemy_speed;
    int enemy_damage;

    /* Begin wave-gun processing */
    if (wave.size > 0) {
        drawWave(wave.x, wave.y, wave.count++);
        if (wave.count == wave.size) wave.count = wave.size = 0;
        XSync(display, 1);
    }
    /* End wave-gun processing */


    /* Begin laser gun processing */
    if (laser.length > 0) {
        drawLaser();
        if (laser.x-laser.length <= bg_bitmap_x) { //if laser gets to left side of screen
            XCopyPlane(display, bg_bitmap, main_win, main_gc, 0, laser.y-3, laser.length+1, 4, laser.x-laser.length+4, laser.y-3, 1);
            laser.x = laser.y = laser.length = laser.speed = 0;
        }
        if (laser.x+laser.length >= bg_bitmap_x+bg_bitmap_width-laser.length) { //if laser gets to right side of screen
            XCopyPlane(display, bg_bitmap, main_win, main_gc, bg_bitmap_x+bg_bitmap_width-laser.length*3, laser.y-3, laser.length*2, 4, laser.x-laser.length, laser.y-3, 1);
            laser.x = laser.y = laser.length = laser.speed = 0;
        }
        
        XSync(display, 1);
    }
    /* End laser gun processing */


    /* Begin rocket processing */
    if (rocket.x == bg_bitmap_x || rocket.x == bg_bitmap_x+bg_bitmap_width || rocket.y == bg_bitmap_y || rocket.y == bg_bitmap_y+bg_bitmap_height) {
        rocket.x = rocket.y = rocket.length = rocket.speed = 0;
    }

    if (rocket.length > 0 && rocket.x == mouse_x && rocket.y == mouse_y) {
        rocket.x = rocket.y = rocket.length = rocket.speed = 0;
        drawMaze();
        drawTopStrings();
        drawBottomStrings();
    }

    if (rocket.length > 0) {
        rocket.last_x = rocket.x;
        rocket.last_y = rocket.y;

        if (rocket.x < mouse_x) rocket.x += rocket.speed;
        else rocket.x -= rocket.speed;
        
        if (rocket.y < mouse_y) rocket.y += rocket.speed;
        else rocket.y -= rocket.speed;

        drawRocket();
        XSync(display, 1);
    }
    /* End rocket processing */
    
    /* Begin processing on-screen or "current" enemies */
    for (i = 0; i < num_enemies; i++) {

        /* Begin check for enemy + wave gun collisions */
        if (wave.size > 0) {
            radius = wave.count/2;
            if (enemies[i].type == 'B' && (enemies[i].x <= wave.x+radius && enemies[i].x >= wave.x-radius) && (enemies[i].y <= wave.y+radius && enemies[i].y >= wave.y-radius)) {
                drawExplosion(enemies[i].x, enemies[i].y, 15);
                score += enemies[i].score;
                enemyDestruct(i);
                //drawMaze();
                drawTopStrings();
                break; //To avoid segfault
            }
        }
        /* End check for each wave gun collisions */

        /* Begin check for enemy + laser gun collisions */
        if (laser.length > 0) {
            if (enemies[i].type == 'G' && (laser.x <= enemies[i].x+8 && laser.x >= enemies[i].x-8) && (laser.y <= enemies[i].y+8 && laser.y >= enemies[i].y-8)) {
                drawExplosion(enemies[i].x, enemies[i].y, 26);
                score += enemies[i].score;
                enemyDestruct(i);
                //drawMaze();
                drawTopStrings();
                laser.x = laser.y = laser.length = laser.speed = 0; //only let laser kill 1 at a time
                break; //To avoid segfault
            }
        }
        /* End check for enemy + laser gun collisions */

        /* Begin check for enemy + rocket collisions */
        if (rocket.length > 0) {
            if (enemies[i].type == 'R' && (rocket.x <= enemies[i].x+8 && rocket.x >= enemies[i].x-8) && (rocket.y <= enemies[i].y+8 && rocket.y >= enemies[i].y-8)) {
                drawExplosion(enemies[i].x, enemies[i].y, 26);
                score += enemies[i].score;
                enemyDestruct(i);
                drawMaze();
                drawTopStrings();
                drawBottomStrings();
                rocket.x = rocket.y = rocket.length = rocket.speed = 0; //only let laser kill 1 at a time
                break; //To avoid segfault
            }
        }
        /* End check for enemy + rocket collisions */

        if (enemies[i].count) {
            (enemies[i].count)--;
            continue;
        }

        enemies[i].count = enemies[i].speed;

        /* Begin check for whether each enemy has reached the castle */
        for (j = enemies[i].steps+1; j < 8; j++) {
            if (enemies[i].path == 'L') {
                if (enemies[i].x == left_path[j].x && enemies[i].y == left_path[j].y) {
                    if (left_path[j].direction_mod == 'E') {
                        health -= enemies[i].damage;
                        drawExplosion(enemies[i].x, enemies[i].y - 10, 35);
                        enemyDestruct(i);
                        //drawMaze();
                        drawTopStrings();
                        return; //Because now our enemies[] array is smaller, and the outer for-loop would segfault
                    }
                    else {
                        enemies[i].direction = left_path[j].direction_mod;
                        enemies[i].steps++;
                    }
                }
            }
            else {
                if (enemies[i].x == right_path[j].x && enemies[i].y == right_path[j].y) {
                    if (right_path[j].direction_mod == 'E') {
                        health -= enemies[i].damage;
                        drawExplosion(enemies[i].x, enemies[i].y - 10, 35);
                        enemyDestruct(i);
                        //drawMaze();
                        drawTopStrings();
                        return; //Because now our enemies[] array is smaller, and the outer for-loop would segfault
                    }
                    else {
                        enemies[i].direction = right_path[j].direction_mod;
                        enemies[i].steps++;
                    }
                }
            }
        }
        /* End check for each enemy reaching castle */

        /* Begin increment of each enemy's position */    
        switch (enemies[i].direction) {
            case 'L':
                (enemies[i].x)--;
                break;
            case 'R':
                (enemies[i].x)++;
                break;
            case 'U':
                (enemies[i].y)--;
                break;
            case 'D':
                (enemies[i].y)++;
                break;
        }
        /* End increment of each enemy's position */

        drawEnemy(i);
    }
    /* End current enemy processing */

    /* Begin level processing */
    switch (level_counter) {
        case 9:
            enemy_speed = 8;
            enemy_damage = 30;
        case 8:
            enemy_speed = 9;
            enemy_damage = 30;
        case 7:
            enemy_speed = 9;
            enemy_damage = 30;
        case 6:
            enemy_speed = 10;
            enemy_damage = 30;
        case 5:
            enemy_speed = 10;
            enemy_damage = 20;
        case 4:
            enemy_speed = 10;
            enemy_damage = 20;
        case 3:
            enemy_speed = 12;
            enemy_damage = 20;
        case 2:
            enemy_speed = 12;
            enemy_damage = 10;
            break;
        case 1:
            enemy_speed = 18;
            enemy_damage = 10;
            break;
        default:
            enemy_speed = 50;
            enemy_damage = 10;
    }

    if (levels[level_counter].enemy_count == levels[level_counter].enemy_total && !num_enemies) {  //stuff to do if this level is complete
        drawSuccessPrompt();
        level_counter++;
        drawMaze();
        drawTopStrings();
        drawBottomStrings();
        return;
    }
    else if (levels[level_counter].count > 0) levels[level_counter].count--;
    else {
        levels[level_counter].count = levels[level_counter].speed;
        if (levels[level_counter].enemy_count < levels[level_counter].enemy_total) {
            int index = levels[level_counter].enemy_count;
            if (index%2) enemyFactory(enemy_speed, enemy_damage, 'L', levels[level_counter].types[index], levels[level_counter].score);
            else enemyFactory(enemy_speed, enemy_damage, 'R', levels[level_counter].types[index], levels[level_counter].score);
            levels[level_counter].enemy_count++;
        }
    }
    /* End level processing */
    //fprintf(stderr, "num_enemies: %d, level_counter %d's enemy_count: %d\n", num_enemies, level_counter, levels[level_counter].enemy_count);

    return;
}


void drawMaze() {
    /* Begin drawing of background maze */
    int hotspot_x, hotspot_y;
    int rc = XReadBitmapFile(display, main_win, "bg_full.xbm", &bg_bitmap_width, &bg_bitmap_height, &bg_bitmap, &hotspot_x, &hotspot_y);

    bg_bitmap_x = 10;
    bg_bitmap_y = 10;

    switch (rc) {
        case BitmapOpenFailed:
            fprintf(stderr, "CRASH: XReadBitmapFile - could not open file 'bg.xbm'.\n");
            return;
        case BitmapFileInvalid:
            fprintf(stderr, "CRASH: XReadBitmapFile - file '%s' doesn't contain a valid bitmap.\n", "bg.xbm");
            return;
        case BitmapNoMemory:
            fprintf(stderr, "CRASH: XReadBitmapFile - not enough memory.\n");
            return;
        case BitmapSuccess:
            XCopyPlane(display, bg_bitmap, main_win, main_gc, 0, 0, bg_bitmap_width, bg_bitmap_height, bg_bitmap_x, bg_bitmap_y, 1);
            break;
    }
    /* End of maze drawing */

    fprintf(stderr, "drawMaze() completed successfully!\n");
    fprintf(stderr, "    bg_bitmap_width = %d, bg_bitmap_height = %d\n", bg_bitmap_width, bg_bitmap_height);
    fprintf(stderr, "    bg_bitmap_x = %d, bg_bitmap_y = %d, current_weapon = %d\n", bg_bitmap_x, bg_bitmap_y, current_weapon);
    
    return;
}


void drawTopStrings() {
    GC health_gc; /* for tomato-coloured font */
    GC score_gc; /* for green-coloured font */
    XColor health_gc_screen, health_gc_exact, score_gc_screen, score_gc_exact;
    Status rc;

    health_gc = XCreateGC(display, main_win, 0, 0);
    score_gc = XCreateGC(display, main_win, 0, 0);

    rc = XAllocNamedColor(display, screen_colormap, "tomato", &health_gc_screen, &health_gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'tomato' color failed.\n");
        return;
    }
    XSetForeground(display, health_gc, health_gc_screen.pixel);

    rc = XAllocNamedColor(display, screen_colormap, "green", &score_gc_screen, &score_gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'green' color failed.\n");
        return;
    }
    XSetForeground(display, score_gc, score_gc_screen.pixel);

    XFontStruct* font_info;
    char* font_name = "lucidasans-10";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "CRASH: XLoadQueryFont: failed loading font '%s'\n", font_name);
        return;
    }
    XSetFont(display, health_gc, font_info->fid);
    XSetFont(display, score_gc, font_info->fid);

    XCopyPlane(display, bg_bitmap, main_win, main_gc, 0, 0, 180, 50, bg_bitmap_x, bg_bitmap_y, 1); //cover what used to be here to avoid overlap

    XDrawString(display, main_win, health_gc, bg_bitmap_x+10, bg_bitmap_y+20, "CASTLE HEALTH:", strlen("CASTLE HEALTH:"));
    char health_str[3];
    sprintf(health_str, "%d", health);
    XDrawString(display, main_win, health_gc, bg_bitmap_x+130, bg_bitmap_y+20, health_str, strlen(health_str));

    XDrawString(display, main_win, score_gc, bg_bitmap_x+10, bg_bitmap_y+40, "SCORE:", strlen("SCORE:"));
    char score_str[50];
    sprintf(score_str, "%d", score);
    XDrawString(display, main_win, score_gc, bg_bitmap_x+65, bg_bitmap_y+40, score_str, strlen(score_str));

    XDrawString(display, main_win, score_gc, bg_bitmap_x+bg_bitmap_width-70, bg_bitmap_y+20, "LEVEL", strlen("LEVEL"));
    char level_str[3];
    sprintf(level_str, "%d", level_counter+1);
    XDrawString(display, main_win, score_gc, bg_bitmap_x+bg_bitmap_width-25, bg_bitmap_y+20, level_str, strlen(level_str));
    
    fprintf(stderr, "drawTopStrings() completed successfully!\n");
    fprintf(stderr, "    health = %d, score = %d\n", health, score);

    return;
}


void drawBottomStrings() {

    XCopyPlane(display, bg_bitmap, main_win, main_gc, 0, bg_bitmap_height-65, bg_bitmap_width, 65, bg_bitmap_x, bg_bitmap_y+bg_bitmap_height-65, 1);

    /* Begin drawing of bottom toolbar colours */
    GC weap_gc;

    weap_gc = XCreateGC(display, main_win, 0, 0);
    XColor gc_screen, gc_exact;
    Status rc;

    XFontStruct* font_info;
    char* font_name = "lucidasans-bold-12";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "CRASH: XLoadQueryFont: failed loading font '%s'\n", font_name);
        return;
    }

    switch (current_weapon) {
        case 3: //Rockets
            rc = XAllocNamedColor(display, screen_colormap, "red", &gc_screen, &gc_exact);
            if (rc == 0) {
                fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'red' color failed.\n");
                return;
            }
            XSetForeground(display, weap_gc, gc_screen.pixel);
            XSetFont(display, weap_gc, font_info->fid);
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+242, bg_bitmap_y+bg_bitmap_height-35, "3", strlen("3"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+269, bg_bitmap_y+bg_bitmap_height-15, "Rocket", strlen("Rocket"));
            XSetForeground(display, weap_gc, WhitePixel(display, screen));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+25, bg_bitmap_y+bg_bitmap_height-35, "1", strlen("1"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+55, bg_bitmap_y+bg_bitmap_height-15, "Laser", strlen("Laser"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+134, bg_bitmap_y+bg_bitmap_height-35, "2", strlen("2"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+165, bg_bitmap_y+bg_bitmap_height-15, "Wave", strlen("Wave"));
            break;
        case 2: //Wave gun
            rc = XAllocNamedColor(display, screen_colormap, "medium blue", &gc_screen, &gc_exact);
            if (rc == 0) {
                fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'medium blue' color failed.\n");
                return;
            }
            XSetForeground(display, weap_gc, gc_screen.pixel);
            XSetFont(display, weap_gc, font_info->fid);
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+134, bg_bitmap_y+bg_bitmap_height-35, "2", strlen("2"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+165, bg_bitmap_y+bg_bitmap_height-15, "Wave", strlen("Wave"));
            XSetForeground(display, weap_gc, WhitePixel(display, screen));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+25, bg_bitmap_y+bg_bitmap_height-35, "1", strlen("1"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+55, bg_bitmap_y+bg_bitmap_height-15, "Laser", strlen("Laser"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+242, bg_bitmap_y+bg_bitmap_height-35, "3", strlen("3"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+269, bg_bitmap_y+bg_bitmap_height-15, "Rocket", strlen("Rocket"));
            break;
        default: //Laser gun a.k.a. '1'
            rc = XAllocNamedColor(display, screen_colormap, "green", &gc_screen, &gc_exact);
            if (rc == 0) {
                fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'green' color failed.\n");
                return;
            }
            XSetForeground(display, weap_gc, gc_screen.pixel);
            XSetFont(display, weap_gc, font_info->fid);
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+25, bg_bitmap_y+bg_bitmap_height-35, "1", strlen("1"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+55, bg_bitmap_y+bg_bitmap_height-15, "Laser", strlen("Laser"));
            XSetForeground(display, weap_gc, WhitePixel(display, screen));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+134, bg_bitmap_y+bg_bitmap_height-35, "2", strlen("2"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+165, bg_bitmap_y+bg_bitmap_height-15, "Wave", strlen("Wave"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+242, bg_bitmap_y+bg_bitmap_height-35, "3", strlen("3"));
            XDrawString(display, main_win, weap_gc, bg_bitmap_x+269, bg_bitmap_y+bg_bitmap_height-15, "Rocket", strlen("Rocket"));
    }
    /* End drawing of bottom toolbar colours */

    return;
}


void drawSuccessPrompt() {
    XSetWindowAttributes attributes;
    attributes.background_pixel = BlackPixel(display, screen);
    attributes.border_pixel = WhitePixel(display, screen);

    Window w = XCreateWindow(display, main_win, bg_bitmap_x+bg_bitmap_width/2-125, bg_bitmap_y+bg_bitmap_height/2-30, 250, 60, 2, screen_depth, InputOutput, main_visual, CWBackPixel | CWBorderPixel, &attributes);

    XMapWindow(display, w);

    GC success_gc = XCreateGC(display, w, 0, 0);
    XColor gc_screen, gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "red", &gc_screen, &gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'red' color failed.\n");
        return;
    }
    XSetForeground(display, success_gc, gc_screen.pixel);

    XFontStruct* font_info;
    char* font_name = "lucidasans-bolditalic-14";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "CRASH: XLoadQueryFont: failed loading font '%s'\n", font_name);
        return;
    }
    XSetFont(display, success_gc, font_info->fid);

    XDrawString(display, w, success_gc, 30, 25, "SURVIVED LEVEL", strlen("SURVIVED LEVEL"));
    char success_str[3];
    sprintf(success_str, "%d", level_counter+1);
    XDrawString(display, w, success_gc, 120, 50, success_str, strlen(success_str));

    XSync(display, False);
    sleep(3);
    XDestroyWindow(display, w);
    drawMaze();
    drawTopStrings();
    drawBottomStrings();
    XSync(display, False);
    return;
}


void drawDeathPrompt() {
    XSetWindowAttributes attributes;
    attributes.background_pixel = BlackPixel(display, screen);
    attributes.border_pixel = WhitePixel(display, screen);

    Window w = XCreateWindow(display, main_win, bg_bitmap_x+bg_bitmap_width/2-125, bg_bitmap_y+bg_bitmap_height/2-30, 250, 60, 2, screen_depth, InputOutput, main_visual, CWBackPixel | CWBorderPixel, &attributes);

    XMapWindow(display, w);

    GC success_gc = XCreateGC(display, w, 0, 0);
    XColor gc_screen, gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "red", &gc_screen, &gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'red' color failed.\n");
        return;
    }
    XSetForeground(display, success_gc, gc_screen.pixel);

    XFontStruct* font_info;
    char* font_name = "lucidasans-bolditalic-14";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "CRASH: XLoadQueryFont: failed loading font '%s'\n", font_name);
        return;
    }
    XSetFont(display, success_gc, font_info->fid);

    XDrawString(display, w, success_gc, 30, 25, "GAME OVER!", strlen("GAME OVER!"));
    XDrawString(display, w, success_gc, 120, 50, "NOW EXITING...", strlen("NOW EXITING..."));

    XSync(display, False);
    sleep(3);
    XDestroyWindow(display, w);
    return;
}


void drawEndPrompt() {
    XSetWindowAttributes attributes;
    attributes.background_pixel = BlackPixel(display, screen);
    attributes.border_pixel = WhitePixel(display, screen);

    Window w = XCreateWindow(display, main_win, bg_bitmap_x+bg_bitmap_width/2-125, bg_bitmap_y+bg_bitmap_height/2-30, 250, 60, 2, screen_depth, InputOutput, main_visual, CWBackPixel | CWBorderPixel, &attributes);

    XMapWindow(display, w);

    GC success_gc = XCreateGC(display, w, 0, 0);
    XColor gc_screen, gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "red", &gc_screen, &gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'red' color failed.\n");
        return;
    }
    XSetForeground(display, success_gc, gc_screen.pixel);

    XFontStruct* font_info;
    char* font_name = "lucidasans-bolditalic-14";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "CRASH: XLoadQueryFont: failed loading font '%s'\n", font_name);
        return;
    }
    XSetFont(display, success_gc, font_info->fid);

    XDrawString(display, w, success_gc, 30, 25, "YOU WIN!", strlen("YOU WIN!"));
    XDrawString(display, w, success_gc, 120, 50, "NOW EXITING...", strlen("NOW EXITING..."));

    XSync(display, False);
    sleep(3);
    XDestroyWindow(display, w);
    return;
}


void enemyFactory(int speed, int damage, char path, char type, int new_score) {
    int i;
    Enemy* temp = NULL;

    if (!num_enemies) {
        num_enemies = 1;
        enemies = (Enemy*)malloc(num_enemies*sizeof(Enemy));
        enemies[0].steps = 0;
        if (path == 'L') {
            enemies[0].x = left_path[0].x;
            enemies[0].y = left_path[0].y;
            enemies[0].direction = left_path[0].direction_mod;
        }
        else {
            enemies[0].x = right_path[0].x;
            enemies[0].y = right_path[0].y;
            enemies[0].direction = right_path[0].direction_mod;
        }
        enemies[0].speed = speed;
        enemies[0].count = speed;
        enemies[0].damage = damage;
        enemies[0].path = path;
        enemies[0].type = type;
        enemies[0].score = new_score;
        return; //End here if first Enemy
    }

    temp = (Enemy*)malloc((num_enemies+1)*sizeof(Enemy));

    for (i = 0; i < num_enemies; i++) {
        temp[i].steps = enemies[i].steps;
        temp[i].x = enemies[i].x;
        temp[i].y = enemies[i].y;
        temp[i].speed = enemies[i].speed;
        temp[i].count = enemies[i].count;
        temp[i].direction = enemies[i].direction;
        temp[i].damage = enemies[i].damage;
        temp[i].path = enemies[i].path;
        temp[i].type = enemies[i].type;
        temp[i].score = enemies[i].score;
    }

    temp[num_enemies].steps = 0;
    if (path == 'L') {
        temp[num_enemies].x = left_path[0].x;
        temp[num_enemies].y = left_path[0].y;
        temp[num_enemies].direction = left_path[0].direction_mod;
    }
    else {
        temp[num_enemies].x = right_path[0].x;
        temp[num_enemies].y = right_path[0].y;
        temp[num_enemies].direction = right_path[0].direction_mod;
    }
    temp[num_enemies].speed = speed;
    temp[num_enemies].count = speed;
    temp[num_enemies].damage = damage;
    temp[num_enemies].path = path;
    temp[num_enemies].type = type;
    temp[num_enemies].score = new_score;
    num_enemies++;

    free(enemies);
    enemies = temp;

    return;
}


void drawEnemy(int index) {
    GC enemy_gc;
    enemy_gc = XCreateGC(display, main_win, 0, 0);
    XColor gc_screen, gc_exact;
    Status rc;

    switch (enemies[index].type) {
        case 'B': //drawing blue x's
            rc = XAllocNamedColor(display, screen_colormap, "royal blue", &gc_screen, &gc_exact);
            if (rc == 0) {
                fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'royal blue' color failed.\n");
                return;
            }
            XSetForeground(display, enemy_gc, gc_screen.pixel);

            XCopyPlane(display, bg_bitmap, main_win, main_gc, enemies[index].x-10-3, enemies[index].y-10-3, 7, 7, enemies[index].x-3, enemies[index].y-3, 1);

            XDrawPoint(display, main_win, enemy_gc, enemies[index].x, enemies[index].y); //draw the enemy onto the next position
            XDrawLine(display, main_win, enemy_gc, enemies[index].x-2, enemies[index].y-2, enemies[index].x+2, enemies[index].y+2);
            XDrawLine(display, main_win, enemy_gc, enemies[index].x-2, enemies[index].y+2, enemies[index].x+2, enemies[index].y-2);
            break;
        case 'G': //drawing green triangles
            rc = XAllocNamedColor(display, screen_colormap, "green", &gc_screen, &gc_exact);
            if (rc == 0) {
                fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'green' color failed.\n");
                return;
            }
            XSetForeground(display, enemy_gc, gc_screen.pixel);

            XCopyPlane(display, bg_bitmap, main_win, main_gc, enemies[index].x-10-9, enemies[index].y-10-8, 20, 20, enemies[index].x-9, enemies[index].y-8, 1);

            XPoint points[] = {
                {enemies[index].x+8, enemies[index].y+8},
                {enemies[index].x-8, enemies[index].y+8},
                {enemies[index].x, enemies[index].y-8},
                {enemies[index].x+8, enemies[index].y+8}
            };

            int npoints = sizeof(points)/sizeof(XPoint);

            XDrawLines(display, main_win, enemy_gc, points, npoints, CoordModeOrigin);
            break;
        case 'R': //drawing red rockets
            rc = XAllocNamedColor(display, screen_colormap, "red", &gc_screen, &gc_exact);
            if (rc == 0) {
                fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'red' color failed.\n");
                return;
            }
            XSetForeground(display, enemy_gc, gc_screen.pixel);
            XSetLineAttributes(display, enemy_gc, 2, LineSolid, CapRound, JoinBevel);

            XCopyPlane(display, bg_bitmap, main_win, main_gc, enemies[index].x-10-9, enemies[index].y-10-8, 20, 20, enemies[index].x-9, enemies[index].y-8, 1);

            XPoint points2[] = {
                {enemies[index].x+6, enemies[index].y+6},
                {enemies[index].x-6, enemies[index].y+6},
                {enemies[index].x-6, enemies[index].y-6},
                {enemies[index].x+6, enemies[index].y-6},
                {enemies[index].x+6, enemies[index].y+6}
            };

            int npoints2 = sizeof(points2)/sizeof(XPoint);

            XDrawLines(display, main_win, enemy_gc, points2, npoints2, CoordModeOrigin);
            break;
    }
    
    return;
}


void enemyDestruct(int index) {
    int i;
    Enemy* temp = NULL;

    if (!num_enemies) return;

    if (num_enemies == 1) {
        num_enemies = 0;
        free(enemies);
        enemies = NULL;
        return; //End here if last remaining Enemy
    }

    temp = (Enemy*)malloc((num_enemies-1)*sizeof(Enemy));

    for (i = 0; i < index; i++) {
        temp[i].steps = enemies[i].steps;
        temp[i].x = enemies[i].x;
        temp[i].y = enemies[i].y;
        temp[i].speed = enemies[i].speed;
        temp[i].count = enemies[i].count;
        temp[i].direction = enemies[i].direction;
        temp[i].damage = enemies[i].damage;
        temp[i].path = enemies[i].path;
        temp[i].type = enemies[i].type;
        temp[i].score = enemies[i].score;
    }

    for (i = index+1; i < num_enemies; i++) {
        temp[i-1].steps = enemies[i].steps;
        temp[i-1].x = enemies[i].x;
        temp[i-1].y = enemies[i].y;
        temp[i-1].speed = enemies[i].speed;
        temp[i-1].count = enemies[i].count;
        temp[i-1].direction = enemies[i].direction;
        temp[i-1].damage = enemies[i].damage;
        temp[i-1].path = enemies[i].path;
        temp[i-1].type = enemies[i].type;
        temp[i-1].score = enemies[i].score;
    }

    num_enemies--;

    free(enemies);
    enemies = temp;

    return;
}


void drawExplosion(int x, int y, int size) {
    int i;

    GC ex_gc = XCreateGC(display, main_win, 0, 0);
    XColor ex_gc_screen, ex_gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "dark red", &ex_gc_screen, &ex_gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'dark red' color failed.\n");
        return;
    }
    XSetForeground(display, ex_gc, ex_gc_screen.pixel);

    for (i = 1; i < size; i++) {
        XFillArc(display, main_win, ex_gc, x-(i/2), y-(i/2), i, i, 0, 360*64);
        XFlush(display);
        usleep(3000);
    }

    rc = XAllocNamedColor(display, screen_colormap, "orange", &ex_gc_screen, &ex_gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'orange' color failed.\n");
        return;
    }
    XSetForeground(display, ex_gc, ex_gc_screen.pixel);

    x += size/4;
    y -= size/4;
    size *= 0.85;

    for (i = 1; i < size; i++) {
        XFillArc(display, main_win, ex_gc, x-(i/2), y-(i/2), i, i, 0, 360*64);
        XFlush(display);
        usleep(3000);
    }

    XCopyPlane(display, bg_bitmap, main_win, main_gc, x-bg_bitmap_x-size, y-bg_bitmap_y-size, size*2, size*2, x-size, y-size, 1);
    
    return;
}


void drawWave(int x, int y, int size) {
    GC wave_gc = XCreateGC(display, main_win, 0, 0);
    XColor wave_gc_screen, wave_gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "medium blue", &wave_gc_screen, &wave_gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'medium blue' color failed.\n");
        return;
    }
    XSetForeground(display, wave_gc, wave_gc_screen.pixel);
    XSetLineAttributes(display, wave_gc, 2, LineSolid, CapRound, JoinBevel);

    XDrawArc(display, main_win, wave_gc, x-(size/2), y-(size/2), size, size, 0, 360*64);

    if (wave.count == wave.size) {
        XCopyPlane(display, bg_bitmap, main_win, main_gc, x-bg_bitmap_x-size/2-2, y-bg_bitmap_y-size/2-2, size+4, size+4, x-size/2-2, y-size/2-2, 1);
    }
    else {
        XCopyPlane(display, bg_bitmap, main_win, main_gc, x-bg_bitmap_x-size/2+4, y-bg_bitmap_y-size/2+4, size-8, size-8, x-size/2+4, y-size/2+4, 1);
    }
   
    return;
}


void drawLaser() {
    GC laser_gc = XCreateGC(display, main_win, 0, 0);
    XColor laser_gc_screen, laser_gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "green", &laser_gc_screen, &laser_gc_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'green' color failed.\n");
        return;
    }
    XSetForeground(display, laser_gc, laser_gc_screen.pixel);
    XSetLineAttributes(display, laser_gc, 2, LineSolid, CapRound, JoinBevel);

    switch (laser.direction) {
        case 'L':
            XDrawLine(display, main_win, laser_gc, laser.x-laser.length, laser.y, laser.x, laser.y);
            XSetLineAttributes(display, main_gc, 2, LineSolid, CapRound, JoinBevel);
            XCopyPlane(display, bg_bitmap, main_win, main_gc, laser.x-bg_bitmap_x, laser.y-bg_bitmap_y-1, laser.length, 2, laser.x, laser.y-1, 1);
            laser.x -= laser.speed;
            break;
        case 'R':
            XDrawLine(display, main_win, laser_gc, laser.x, laser.y, laser.x+laser.length, laser.y);
            XSetLineAttributes(display, main_gc, 2, LineSolid, CapRound, JoinBevel);
            XCopyPlane(display, bg_bitmap, main_win, main_gc, laser.x-bg_bitmap_x-laser.length, laser.y-bg_bitmap_y-1, laser.length, 2, laser.x-laser.length, laser.y-1, 1);
            laser.x += laser.speed;
            break;
    }

    return;
}


void drawRocket() {
    GC rocket_gc = XCreateGC(display, main_win, 0, 0);
    XColor gc_screen, gc_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "red", &gc_screen, &gc_exact);
    int diff_x = 0;
    int diff_y = 0;

    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'red' color failed.\n");
        return;
    }
    XSetForeground(display, rocket_gc, gc_screen.pixel);
    XSetLineAttributes(display, rocket_gc, 2, LineSolid, CapRound, JoinBevel);

    //XFillRectangle(display, main_win, rocket_gc, rocket.x, rocket.y, rocket.length, rocket.length);
    
    if (rocket.x < rocket.last_x) diff_x = rocket.length*2+5; //moving left
    if (rocket.x > rocket.last_x) diff_x = rocket.length*(-2)-5; //moving right
    if (rocket.y < rocket.last_y) diff_y = rocket.length*2+5; //moving up
    if (rocket.y > rocket.last_y) diff_y = rocket.length*(-2)-5; //moving down

    XCopyPlane(display, bg_bitmap, main_win, main_gc, rocket.x-bg_bitmap_x+diff_x, rocket.y-bg_bitmap_y+diff_y, rocket.length*4, rocket.length*4, rocket.x+diff_x, rocket.y+diff_y, 1);

    XDrawArc(display, main_win, rocket_gc, rocket.x-(rocket.length/2), rocket.y-(rocket.length/2), rocket.length, rocket.length, 0, 360*64);

    //rocket.y -= rocket.speed;
    
    return;
}


void writePathNodes() {
    /* Begin left path */
    left_path[0].x = bg_bitmap_x + 136;
    left_path[0].y = bg_bitmap_y + bg_bitmap_height - 98;
    left_path[0].direction_mod = 'U';

    left_path[1].x = bg_bitmap_x + 136;
    left_path[1].y = bg_bitmap_y + 311;
    left_path[1].direction_mod = 'R';

    left_path[2].x = bg_bitmap_x + 291;
    left_path[2].y = bg_bitmap_y + 311;
    left_path[2].direction_mod = 'U';

    left_path[3].x = bg_bitmap_x + 291;
    left_path[3].y = bg_bitmap_y + 210;
    left_path[3].direction_mod = 'L';

    left_path[4].x = bg_bitmap_x + 136;
    left_path[4].y = bg_bitmap_y + 210;
    left_path[4].direction_mod = 'U';

    left_path[5].x = bg_bitmap_x + 136;
    left_path[5].y = bg_bitmap_y + 130;
    left_path[5].direction_mod = 'R';

    left_path[6].x = bg_bitmap_x + 291;
    left_path[6].y = bg_bitmap_y + 130;
    left_path[6].direction_mod = 'U';

    left_path[7].x = bg_bitmap_x + 291;
    left_path[7].y = bg_bitmap_y + 97;
    left_path[7].direction_mod = 'E'; //END OF PATH
    /* End left path */

    /* Begin right path */
    right_path[0].x = bg_bitmap_x + 466;
    right_path[0].y = bg_bitmap_y + bg_bitmap_height - 98;
    right_path[0].direction_mod = 'U';

    right_path[1].x = bg_bitmap_x + 466;
    right_path[1].y = bg_bitmap_y + 311;
    right_path[1].direction_mod = 'L';

    right_path[2].x = bg_bitmap_x + 321;
    right_path[2].y = bg_bitmap_y + 311;
    right_path[2].direction_mod = 'U';

    right_path[3].x = bg_bitmap_x + 321;
    right_path[3].y = bg_bitmap_y + 210;
    right_path[3].direction_mod = 'R';

    right_path[4].x = bg_bitmap_x + 466;
    right_path[4].y = bg_bitmap_y + 210;
    right_path[4].direction_mod = 'U';

    right_path[5].x = bg_bitmap_x + 466;
    right_path[5].y = bg_bitmap_y + 130;
    right_path[5].direction_mod = 'L';

    right_path[6].x = bg_bitmap_x + 321;
    right_path[6].y = bg_bitmap_y + 130;
    right_path[6].direction_mod = 'U';

    right_path[7].x = bg_bitmap_x + 321;
    right_path[7].y = bg_bitmap_y + 97;
    right_path[7].direction_mod = 'E'; //END OF PATH
    /* End right path */
}


int splash() {
    GC splash_gc1; /* has main window background colour and increased line width*/
    GC splash_gc2; /* has "tan" colour */
    GC splash_gc3; /* has "black" colour and font and increased line width */

    splash_gc1 = XCreateGC(display, main_win, 0, 0);
    XSetForeground(display, splash_gc1, bg_screen_color.pixel);

    splash_gc2 = XCreateGC(display, main_win, 0, 0);
    XColor gc2_screen, gc2_exact;
    Status rc = XAllocNamedColor(display, screen_colormap, "tan", &gc2_screen, &gc2_exact);
    if (rc == 0) {
        fprintf(stderr, "CRASH: XAllocNamedColor - allocation of 'tan' color failed.\n");
        return 1;
    }
    XSetForeground(display, splash_gc2, gc2_screen.pixel);

    splash_gc3 = XCreateGC(display, main_win, 0, 0);
    XFontStruct* font_info;
    char* font_name = "lucidasans-bold-14";
    font_info = XLoadQueryFont(display, font_name);
    if (!font_info) {
        fprintf(stderr, "CRASH: XLoadQueryFont: failed loading font '%s'\n", font_name);
        return 1;
    }
    XSetFont(display, splash_gc3, font_info->fid);
    XSetForeground(display, splash_gc3, BlackPixel(display, screen));
    XSetLineAttributes(display, splash_gc3, 3, LineSolid, CapRound, JoinBevel);

    int i;
    int speed = 11000;
    int arc_width = 350;
    int arc_height = 225;
    /* Begin unnecessarily complex calculations for attempting to keep splash screen centred */
    int arc_centre_x = 640/2 - arc_width*0.55 - (640 - 608)*0.05;
    int arc_centre_y = 600/2 - arc_height*0.75 - (480 - 384)*0.15;
    /* End stupid calculations */

    XSetArcMode(display, splash_gc2, ArcPieSlice);
    XSetArcMode(display, splash_gc1, ArcPieSlice);

    for (i = 0; i <= 45; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XFlush(display);
        usleep(speed);
    }

    for (i = 46; i <= 80; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.5), arc_centre_y+(arc_height*0.4), "Assignment 1", strlen("Assignment 1"));
        XFlush(display);
        usleep(speed);
    }
    
    for (i = 81; i <= 120; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.5), arc_centre_y+(arc_height*0.4), "Assignment 1", strlen("Assignment 1"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.35), "SE382", strlen("SE382"));
        XFlush(display);
        usleep(speed);
    }

    for (i = 121; i <= 165; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.35), "SE382", strlen("SE382"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.6), "Winter", strlen("Winter"));
        XFlush(display);
        usleep(speed);
    }

    for (i = 166; i <= 181; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.35), "SE382", strlen("SE382"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.6), "Winter", strlen("Winter"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.5), arc_centre_y+(arc_height*0.7), "2011", strlen("2011"));
        XFlush(display);
        usleep(speed);
    }

    /* First rotation completed */

    for (i = 0; i <= 181; i++) {
        XFillArc(display, main_win, splash_gc1, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc1, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XFlush(display);
        usleep(speed);
    }

    for (i = 0; i <= 45; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XFlush(display);
        usleep(speed);
    }

    for (i = 46; i <= 80; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.5), arc_centre_y+(arc_height*0.4), "Andrew Klamut", strlen("Andrew Klamut"));
        XFlush(display);
        usleep(speed);
    }

    for (i = 81; i <= 120; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.5), arc_centre_y+(arc_height*0.4), "Andrew Klamut", strlen("Andrew Klamut"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.35), "by", strlen("by"));
        XFlush(display);
        usleep(speed);
    }

    for (i = 121; i <= 165; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.35), "by", strlen("by"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.6), "UWID", strlen("UWID"));
        XFlush(display);
        usleep(speed);
    }

    for (i = 166; i <= 181; i++) {
        XFillArc(display, main_win, splash_gc2, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc3, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.35), "by", strlen("by"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.19), arc_centre_y+(arc_height*0.6), "UWID:", strlen("UWID:"));
        XDrawString(display, main_win, splash_gc3, arc_centre_x+(arc_width*0.5), arc_centre_y+(arc_height*0.7), "ajklamut", strlen("ajklamut"));
        XFlush(display);
        usleep(speed);
    }

    XSetLineAttributes(display, splash_gc1, 3, LineSolid, CapRound, JoinBevel);

    for (i = 0; i <= 181; i++) {
        XFillArc(display, main_win, splash_gc1, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XDrawArc(display, main_win, splash_gc1, arc_centre_x, arc_centre_y, arc_width, arc_height, 64*i, 64*i+1);
        XFlush(display);
        usleep(speed);
    }

    return 0;
}
