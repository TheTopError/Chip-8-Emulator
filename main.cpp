#include "Chip8.cpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <memory>

int screenWidth = 640;
int screenHeight = 320;

uint8_t counter = 0;

int main(int argc, char* args[]) {

    std::unique_ptr<Chip8> ch8(new Chip8);
    ch8->LoadRom("rom//Pong.ch8");

    //IBM:  Nur ein Logo
    //Pong: Pong, funktioniert nicht ganz richtig
    //Cave: Funktioniert

    //Pong: Down: Move up (nice), Y: Move down
    //Cave: R:Start, A Y C and Left movement 


#pragma region Window/SDL initialization


    SDL_Window* window = NULL;
    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if (SDL_Init(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create window
        window = SDL_CreateWindow("Chip8", 100, 100, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            screenSurface = SDL_GetWindowSurface(window);
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 155));
            SDL_UpdateWindowSurface(window);

            //Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event event;

            //Sounds
            Mix_AllocateChannels(2);
            std::unique_ptr<Mix_Chunk, void (*)(Mix_Chunk*)> chunk(Mix_LoadWAV("sine.wav"), Mix_FreeChunk);
            Mix_VolumeChunk(chunk.get(), 50);

            // rectangle for drawing
            SDL_Rect rect;
            rect.w = screenWidth / 64;
            rect.h = screenWidth / 32;
            

#pragma endregion
            
            while (!quit)
            {
                //Eventhandling
                while (SDL_PollEvent(&event) != 0)
                {
                    if (event.type == SDL_QUIT) {
                        printf("\nShut down\n");
                        quit = true;
                        goto END; // Goes to end of Game loop to quit SDL etc.
                    }
                    else {
                        switch (event.type) {
                        case SDL_KEYDOWN: //Keys[0-F] = true if certain buttons are pressed down
                            switch (event.key.keysym.sym) {
                            case SDLK_UP:
                                ch8->keys[0] = true;
                                break;

                            case SDLK_DOWN:
                                ch8->keys[1] = true;
                                break;

                            case SDLK_LEFT:
                                ch8->keys[2] = true;
                                break;

                            case SDLK_RIGHT:
                                ch8->keys[3] = true;
                                break;

                            case SDLK_y:
                                ch8->keys[4] = true;
                                break;

                            case SDLK_z:
                                ch8->keys[4] = true;
                                break;

                            case SDLK_x:
                                ch8->keys[5] = true;
                                break;

                            case SDLK_c:
                                ch8->keys[6] = true;
                                break;

                            case SDLK_v:
                                ch8->keys[7] = true;
                                break;

                            case SDLK_a:
                                ch8->keys[8] = true;
                                break;

                            case SDLK_s:
                                ch8->keys[9] = true;
                                break;

                            case SDLK_d:
                                ch8->keys[0xA] = true;
                                break;

                            case SDLK_f:
                                ch8->keys[0xb] = true;
                                break;

                            case SDLK_q:
                                ch8->keys[0xc] = true;
                                break;

                            case SDLK_w:
                                ch8->keys[0xd] = true;
                                break;

                            case SDLK_e:
                                ch8->keys[0xe] = true;
                                break;

                            case SDLK_r:
                                ch8->keys[0xf] = true;
                                break;

                            default:
                                break;
                            }
                            break;

                        case SDL_KEYUP://Keys[0-F] = false if buttons are released
                            switch (event.key.keysym.sym) {
                            case SDLK_UP:
                                ch8->keys[0] = false;
                                break;

                            case SDLK_DOWN:
                                ch8->keys[1] = false;
                                break;

                            case SDLK_LEFT:
                                ch8->keys[2] = false;
                                break;

                            case SDLK_RIGHT:
                                ch8->keys[3] = false;
                                break;

                            case SDLK_y:
                                ch8->keys[4] = false;
                                break;

                            case SDLK_z:
                                ch8->keys[4] = false;
                                break;

                            case SDLK_x:
                                ch8->keys[5] = false;
                                break;

                            case SDLK_c:
                                ch8->keys[6] = false;
                                break;

                            case SDLK_v:
                                ch8->keys[7] = false;
                                break;

                            case SDLK_a:
                                ch8->keys[8] = false;
                                break;

                            case SDLK_s:
                                ch8->keys[9] = false;
                                break;

                            case SDLK_d:
                                ch8->keys[0xA] = false;
                                break;

                            case SDLK_f:
                                ch8->keys[0xb] = false;
                                break;

                            case SDLK_q:
                                ch8->keys[0xc] = false;
                                break;

                            case SDLK_w:
                                ch8->keys[0xd] = false;
                                break;

                            case SDLK_e:
                                ch8->keys[0xe] = false;
                                break;

                            case SDLK_r:
                                ch8->keys[0xf] = false;
                                break;

                            default:
                                break;
                            }
                            break;
                        }
                    }
                }

                ch8->Cycle();

                if (ch8->needToDraw) {

                    ch8->needToDraw = false;

                    for (int i = 0; i < (sizeof(ch8->display) / sizeof(ch8->display[0])); i++) {

                        rect.x = (i % 64) * (screenWidth / 64);
                        rect.y = (i / 64) * (screenWidth / 64);

                        if (ch8->display[i] == 1) {
                            SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 255, 255, 255));
                        }
                        else {
                            SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 0, 0, 0));
                        }
                    }
                }

                if (ch8->clear) { //Could also loop through display[] but this is faster
                    ch8->clear = false;
                    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
                }

                if (counter == 16) { //decrementing sound and delay timer in ~60hz
                    counter = 0;

                    if (ch8->soundTimer > 0) {
                        --ch8->soundTimer;
                        //play sound
                        Mix_PlayChannel(1, chunk.get(), 1); //hört sich schrecklich an
                    }

                    if (ch8->delayTimer > 0) --ch8->delayTimer;

                }

                counter += 2;
                SDL_Delay(1); //500hz clock speed
                END:
                SDL_UpdateWindowSurface(window);
            }
#pragma region Quit

            chunk.reset();
        }
        SDL_DestroyWindow(window);

        //Quit SDL subsystems
        Mix_Quit();
        SDL_Quit();
    }
#pragma endregion
    ch8.reset();
	return 0;
}

