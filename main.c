#include <gb/gb.h>
#include <stdio.h>
#include "mazesprites.c"
#include "mazemap1.c"
#include "mazemap2.c"
#include "Snail.c"
#include "logo_data.c"
#include "logo_map.c"

const char blankmap[1] = {0x00};
UINT8 playerlocation[2];
UBYTE debug, haskey, gamerunning;
UINT8 currentmap = 0;
UINT8 currentmapprev = 0;
UINT8 i;



void performantdelay(UINT8 numloops){
    UINT8 i;
    for(i = 0; i < numloops; i++){
        wait_vbl_done();
    }     
}
void playsound(int a, int b, int c, int d, int e){
    NR10_REG = a;
    NR11_REG = b;
    NR12_REG = c;
    NR13_REG = d;
    NR14_REG = e;
}
UBYTE canplayermove(UINT8 newplayerx, UINT8 newplayery){
    UINT16 indexTLx, indexTLy, tileindexTL;
    UBYTE result;

    indexTLx = (newplayerx - 8) / 8;
    indexTLy = (newplayery - 16) / 8;
    tileindexTL = 20 * indexTLy + indexTLx;

    if(debug){
        printf("%u %u\n",(UINT16)(newplayerx),(UINT16)(newplayery));
        printf("%u %u %u\n",(UINT16)indexTLx,(UINT16)indexTLy,(UINT16)tileindexTL);  
        if(currentmap==0) 
            printf("%u\n",(UINT16)MazeMap1[tileindexTL]);
        else if(currentmap==1)
            printf("%u\n",(UINT16)MazeMap2[tileindexTL]);
    }    
    if(currentmap==0)
        result = MazeMap1[tileindexTL] == blankmap[0];
    else if(currentmap==1)
        result = MazeMap2[tileindexTL] == blankmap[0];
    if(tileindexTL==keypos1 && currentmap == 0){
        // collect key
        set_bkg_tiles(1,16,1,1,blankmap);
        playsound(0x16,0x40,0x73,0x00,0xC3);
        haskey = 1;
        result = 1;
    }
    else if(tileindexTL==keypos2 && currentmap == 1){
        set_bkg_tiles(1,11,1,1,blankmap);
        playsound(0x16,0x40,0x73,0x00,0xC3);
        haskey = 1;
        result = 1;
    }
    else if(tileindexTL==doorpos1 && haskey && currentmap == 0){
        // open door
        set_bkg_tiles(3,13,1,1,blankmap);
        playsound(0x78,0xC1,0x4B,0x73,0x86);
        result = 1;
    }
    else if(tileindexTL==doorpos2 && haskey && currentmap == 1){
        set_bkg_tiles(4,14,1,1,blankmap);
        playsound(0x78,0xC1,0x4B,0x73,0x86);
        result = 1;
    }
    else if(tileindexTL==340){
        // finish game
        if(currentmap==0){
            HIDE_SPRITES;
            currentmap = 1;
            result = 1;   
        }
        else if(currentmap==1){
            HIDE_SPRITES;
            printf("Game Over");
            gamerunning=0;
            result = 1;
        }
             
    }

    return result;
}

void animatesprite(UINT8 spriteindex, INT8 movex, INT8 movey){
    while(movex!=0){
        scroll_sprite(spriteindex, movex < 0 ? -1 : 1, 0);
        movex += (movex < 0 ? 1 : -1);
        wait_vbl_done();
    }
    while(movey!=0){
        scroll_sprite(spriteindex, 0, movey < 0 ? -1 : 1);
        movey += movey < 0 ? 1 : -1;
        wait_vbl_done();
    } 
    playsound(0x70,0x40,0x53,0x72,0x86);
}
void fadeout(){
    for(i = 0; i < 4; i++){
        switch(i){
            case 0:
                BGP_REG = 0xE4;
                break;
            case 1:
                BGP_REG = 0xF9;
                break;
            case 2:
                BGP_REG = 0xFE;
                break;
            case 3:
                BGP_REG = 0xFF;
                break;

        }
        performantdelay(10);
    }
}

void fadein(){
    for(i = 0; i < 3; i++){
        switch(i){
            case 0:
                BGP_REG = 0xFE;
                break;
            case 1:
                BGP_REG = 0xF9;
                break;
            case 2:
                BGP_REG = 0xE4;
                break;

        }
        performantdelay(10);
    }
}
void animatebackground(){
    
}

void main(){
    NR52_REG = 0x80;
    NR50_REG = 0x77;
    NR51_REG = 0xFF;
    set_bkg_data(0,114,logo_data);
    set_bkg_tiles(0,0,20,18,logo_map);

    SHOW_BKG;
    DISPLAY_ON;

    waitpad(J_START);

    fadeout();

    set_bkg_data(0, 4, mazesprites);
    set_bkg_tiles(0, 0, 20, 18, MazeMap1);

	set_sprite_data(0, 1, Snail);   /* defines the sprite data */
	set_sprite_tile(0,0);            /* defines the tiles numbers */

    fadein();
    playerlocation[0] = 16;
    playerlocation[1] = 24;

    move_sprite(0,playerlocation[0],playerlocation[1]);

    

    gamerunning = 1;

    SHOW_SPRITES;
    SHOW_BKG;
    DISPLAY_ON;

    while(gamerunning){
        if(currentmapprev != currentmap){
            fadeout();
            haskey = 0;
            set_bkg_data(0,4,mazesprites);
            set_bkg_tiles(0,0,20,18,MazeMap2);
            set_sprite_data(0,1, Snail);
            set_sprite_tile(0,0);
            fadein();
            playerlocation[0] = 16;
            playerlocation[1] = 24;
            move_sprite(0,playerlocation[0],playerlocation[1]);
            SHOW_SPRITES;
            SHOW_BKG;
            DISPLAY_ON;
            currentmapprev = 1;
        }      
        if(joypad() & J_A){
            debug = 1;
        }
        if(joypad() & J_B){
            currentmap = 1;
        }
        if(joypad() & J_LEFT){
            if(canplayermove(playerlocation[0]-8,playerlocation[1])){
                playerlocation[0] -= 8;
                animatesprite(0,-8,0);
            }
        }
        else if(joypad() & J_RIGHT){
            if(canplayermove(playerlocation[0]+8,playerlocation[1])){            
                playerlocation[0] += 8;
                animatesprite(0,8,0);
            }
        }
        else if(joypad() & J_UP){
            if(canplayermove(playerlocation[0],playerlocation[1]-8)){
                playerlocation[1] -= 8;
                animatesprite(0,0,-8);
            }
        }
        else if(joypad() & J_DOWN){
            if(canplayermove(playerlocation[0],playerlocation[1]+8)){
                playerlocation[1] += 8;
                animatesprite(0,0,8);
            }
        }
        
        performantdelay(6);
    }
}