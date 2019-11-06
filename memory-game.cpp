#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <SDL.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <chrono>
#include <time.h> 

using namespace std;

#define SCREEN_W 1305
#define SCREEN_H 768

#define CARD_H 236
#define CARD_W 200

#define CARD_GAP 15

struct Pos
{
    double x, y;
};

class Card
{
    public:
        int id;
};

struct AppearTimes
{
    Card card;
    int times;
};

class CardSection
{
    public:
        Card card;
        Pos position; 
};

SDL_Window *screen;
SDL_Renderer *renderer;

int alive = 1;

CardSection cardboard[3][6];
Card cards[10];

void load_assets()
{

    int i;
    for(i = 0; i < 10; i++)
        cards[i].id = i;

}

void render()
{
    
    int i, j;
    SDL_Rect rect;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

    for(i = 0; i < 3; i++){
        for(j = 0; j < 6; j++){
            
            rect.x = cardboard[i][j].position.x;
            rect.y = cardboard[i][j].position.y;
            rect.w = CARD_W;
            rect.h = CARD_H;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		    SDL_RenderFillRect(renderer, &rect);

        }
    }

    SDL_RenderPresent(renderer);

}

void renderer_p()
{

    chrono::high_resolution_clock::time_point tbegin, tend;
	double elapsed;

    while(alive){
        
        tbegin = chrono::high_resolution_clock::now();

        render();

        do {
			tend = chrono::high_resolution_clock::now();
			chrono::duration<double> elapsed_ = chrono::duration_cast<chrono::duration<double>>(tend - tbegin);
			elapsed = elapsed_.count();
		} while (elapsed < 0.01);

    }

}

void create_cardboard()
{
    int i, j, x, y;

    for(i = 0; i < 3; i++){
        for(j = 0; j < 6; j++){
            
            if(j == 0){
                cardboard[i][j].position.x = CARD_GAP;
            }else{
                cardboard[i][j].position.x = CARD_GAP
                     + cardboard[i][j-1].position.x + CARD_W;
            }
                
            if(i == 0){
                cardboard[i][j].position.y = CARD_GAP;
            }else{
                cardboard[i][j].position.y = CARD_GAP 
                    + cardboard[i-1][j].position.y + CARD_H;
            }

        }
    }

}

void randomize_cards()
{

    bool dpass = false;
    int i, j, count, id;
    AppearTimes apt[10];

    for(i = 0; i < 10; i++){
        apt[i].card = cards[i];
        apt[i].times = 2;
    }

    srand (time(NULL));

    for(i = 0; i < 3; i++){
        for(j = 0; j < 6; j++){
            
            do{
                id = (rand() % 9) + 1;
                if(apt[id].times > 0){
                    apt[id].times--;
                    cardboard[i][j].card = apt[id].card;
                    dpass = false;
                }else{
                    dpass = true;
                }

            }while(dpass);

        }
    }
    
}

void on_select(Card *card)
{
    printf("%d\n", card->id);
}

void on_click(Sint32 x, Sint32 y)
{

    int i, j;

    for(i = 0; i < 3; i++){
        for(j = 0; j < 6; j++){
            
            if(x >= cardboard[i][j].position.x && x <= cardboard[i][j].position.x + CARD_W &&
                y >= cardboard[i][j].position.y && y <= cardboard[i][j].position.y + CARD_H){  
                on_select(&cardboard[i][j].card);
                return;
            }

        }

    }

}

int main()
{

    SDL_Event event;
    SDL_Init(SDL_INIT_EVERYTHING);

    screen = SDL_CreateWindow("Memory Game CPP",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_W, SCREEN_H,
		SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(screen, -1, 0);

    load_assets();
    create_cardboard();
    randomize_cards();

    #pragma omp parallel
    {
        #pragma omp single
        {

            #pragma omp task
            renderer_p();

            while(alive){

                while (SDL_PollEvent(&event)) {
			        switch (event.type) {
				        case SDL_QUIT:
					        alive = 0;
					        break;
                        case SDL_MOUSEBUTTONDOWN:
                            on_click(event.motion.x, event.motion.y);
				            break;
			        }
		        }

            }

        }
    }

    SDL_Quit();
	return 0;

}