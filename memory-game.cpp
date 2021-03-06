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
#include <unistd.h>
#include <SDL_image.h>

using namespace std;

#define SCREEN_W 1305
#define SCREEN_H 768

#define CARD_W 200
#define CARD_H 236
#define CARD_GAP 15

#define STATUS_DONE 0
#define STATUS_DOWN -1
#define STATUS_UP 1

struct Pos
{
    double x, y;
};

class Card
{
    public:
        int id;
        int status;
        SDL_Surface *surface;
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
int points = 0;

CardSection cardboard[3][6];
Card cards[10];

CardSection *selected_card;

void load_assets()
{

    int i;
    for(i = 0; i < 10; i++){
        cards[i].id = i;
        cards[i].status = STATUS_DOWN;
    }

    cards[0].surface = IMG_Load("img/back.png");
    cards[1].surface = IMG_Load("img/card_1.png");
    cards[2].surface = IMG_Load("img/card_2.png");
    cards[3].surface = IMG_Load("img/card_3.png");
    cards[4].surface = IMG_Load("img/card_4.png");
    cards[5].surface = IMG_Load("img/card_5.png");
    cards[6].surface = IMG_Load("img/card_6.png");
    cards[7].surface = IMG_Load("img/card_7.png");
    cards[8].surface = IMG_Load("img/card_8.png");
    cards[9].surface = IMG_Load("img/card_9.png");

}

void render()
{
    
    int i, j;
    SDL_Rect rect;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

    for(i = 0; i < 3; i++){
        for(j = 0; j < 6; j++){
            
            SDL_Texture *texture;

            Card *card = &cardboard[i][j].card;
            rect.x = cardboard[i][j].position.x;
            rect.y = cardboard[i][j].position.y;
            rect.w = CARD_W;
            rect.h = CARD_H;
            
            if(card->status != STATUS_DONE){
                
                if(card->status == STATUS_DOWN){
                    texture = SDL_CreateTextureFromSurface(renderer, cards[0].surface);
                }else{
                    texture = SDL_CreateTextureFromSurface(renderer, card->surface);
                }

                SDL_RenderCopy(renderer, texture, NULL, &rect);
            
            }

            SDL_DestroyTexture(texture);

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

    srand(time(NULL));

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

void on_select(CardSection *card_section)
{
    
    Card *card = &card_section->card;

    if(card_section->card.status == STATUS_DONE)
        return;

    if(selected_card == NULL){
        card->status = STATUS_UP;
        selected_card = card_section;
    }else{

        if(selected_card->position.x == card_section->position.x &&
            selected_card->position.y == card_section->position.y)
            return;

        card->status = STATUS_UP;

        usleep(400000);

        if(selected_card->card.id == card->id){
            selected_card->card.status = STATUS_DONE;
            card->status = STATUS_DONE;
            selected_card = NULL;
            points++;
        }else {
            selected_card->card.status = STATUS_DOWN;
            card->status = STATUS_DOWN;
            selected_card = NULL;   
        }

    }

}

void on_click(Sint32 x, Sint32 y)
{

    int i, j;

    for(i = 0; i < 3; i++){
        for(j = 0; j < 6; j++){
            
            if(x >= cardboard[i][j].position.x && x <= cardboard[i][j].position.x + CARD_W &&
                y >= cardboard[i][j].position.y && y <= cardboard[i][j].position.y + CARD_H){  
                on_select(&cardboard[i][j]);
                return;
            }

        }

    }

}

int main()
{

    SDL_Event event;
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    screen = SDL_CreateWindow("Memory Game CPP",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_W, SCREEN_H,
		SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(screen, -1, 0);

    selected_card = NULL;

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

                if(points == 9){
                    randomize_cards();
                    points = 0;
                }

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

    IMG_Quit();
    SDL_Quit();
	return 0;

}