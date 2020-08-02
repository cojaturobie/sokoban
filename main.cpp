#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<windows.h>

extern "C" {
#include"./sdl-2.0.7/include/SDL.h"
#include"./sdl-2.0.7/include/SDL_main.h"
}

//rozmiar tablicy pole
#define POLEX 14
#define POLEY 10
//rozdzielczoœæ bitmaapy o wymiarach ELEMENT*ELEMENT i wyznaczenie jej œrodka
#define ELEMENT 90
#define SRODEK ELEMENT/2
//ustwawianie rozdzielczoœæ ze wzglêdu na rozmiar tablicy pole i wielkoœci elementu
#define SCREEN_WIDTH	POLEX*ELEMENT
#define SCREEN_HEIGHT	POLEY*ELEMENT
//prêdkoœæ animacji
#define ANIMACJA1	1
#define ANIMACJA2   1



enum element {
	GRACZ,
	KORYTARZ,
	SKRZYNIA,
	SCIANA,
};

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void nowa(int t[POLEY][POLEX], bool c[POLEY][POLEX]);
int wygrana(int t[POLEY][POLEX], bool c[POLEY][POLEX]);
void wczytaj(int t[POLEY][POLEX], bool c[POLEY][POLEX], FILE *plik);

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	//
	int x=1, y = 1;
	bool cele[POLEY][POLEX];
	int pole[POLEY][POLEX];
	nowa(pole,cele);
	//
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *gracz;
	SDL_Surface *gracz2;
	SDL_Surface *sciana;
	SDL_Surface *cel;
	SDL_Surface *skrzynia;
	SDL_Surface *jest;
	SDL_Surface *korytarz;
	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "SOKOBAN - Jacek Grzybek 165710");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	// wczytanie obrazka gracz.bmp
	gracz = SDL_LoadBMP("./gracz.bmp");
	if (gracz == NULL) {
		printf("SDL_LoadBMP(gracz.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka gracz2.bmp
	gracz2 = SDL_LoadBMP("./gracz2.bmp");
	if (gracz2 == NULL) {
		printf("SDL_LoadBMP(gracz2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka sciana.bmp
	sciana = SDL_LoadBMP("./sciana.bmp");
	if (sciana == NULL) {
		printf("SDL_LoadBMP(sciana.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka cel.bmp
	cel = SDL_LoadBMP("./cel.bmp");
	if (cel == NULL) {
		printf("SDL_LoadBMP(cel.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	
	// wczytanie obrazka skkrzynia.bmp
	skrzynia = SDL_LoadBMP("./skrzynia.bmp");
	if (skrzynia == NULL) {
		printf("SDL_LoadBMP(skrzynia.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka jest.bmp
	jest = SDL_LoadBMP("./jest.bmp");
	if (jest == NULL) {
		printf("SDL_LoadBMP(jest.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	// wczytanie obrazka korytarz.bmp
	korytarz = SDL_LoadBMP("./korytarz.bmp");
	if (korytarz == NULL) {
		printf("SDL_LoadBMP(korytarz.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;

	int starex = -1, starey = -1;

	while(!quit) {
		if (wygrana(pole, cele) == 1) {
			t2 = SDL_GetTicks();

			// w tym momencie t2-t1 to czas w milisekundach,
			// jaki uplyna³ od ostatniego narysowania ekranu
			// delta to ten sam czas w sekundach
			// here t2-t1 is the time in milliseconds since
			// the last screen was drawn
			// delta is the same time in seconds
			delta = (t2 - t1) * 0.001;
			t1 = t2;

			worldTime += delta;

		}

		SDL_FillRect(screen, NULL, czarny);

		
		//rysowanie pola gry
		for (int i = 0; i < POLEX; i++) {
			for (int j = 0; j < POLEY; j++) {
				if(pole[j][i]==SCIANA)DrawSurface(screen, sciana, i*ELEMENT+ SRODEK, j*ELEMENT+ SRODEK);
				else if (pole[j][i] == KORYTARZ && cele[j][i]==true)DrawSurface(screen, cel, i*ELEMENT + SRODEK, j*ELEMENT + SRODEK);
				else if (pole[j][i] == KORYTARZ && cele[j][i] == false)DrawSurface(screen, korytarz, i*ELEMENT + SRODEK, j*ELEMENT + SRODEK);
				else if (pole[j][i] == SKRZYNIA && cele[j][i] == true)DrawSurface(screen, jest, i*ELEMENT + SRODEK, j*ELEMENT + SRODEK);
				else if (pole[j][i] == SKRZYNIA && cele[j][i] == false)DrawSurface(screen, skrzynia, i*ELEMENT + SRODEK, j*ELEMENT + SRODEK);
				else if (pole[j][i] == GRACZ)DrawSurface(screen, gracz, i*ELEMENT + SRODEK, j*ELEMENT + SRODEK);
			}
		}
			
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};
		// tekst informacyjny / info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "SOKOBAN                                               Czas trwania = %.1lf s                             %.0lf klatek / s", worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		//	      "Esc - exit, \030 - faster, \031 - slower"
		if(wygrana(pole,cele)==1)sprintf(text, "Esc - wyjscie            Graj dalej!!!");
		else sprintf(text, "Esc - wyjscie            Wygra³eœ gratulacje!!!");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		
		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_RIGHT && pole[x][y+1]==KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK + i, x*ELEMENT + SRODEK);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK + i, x*ELEMENT + SRODEK);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA1);
						}
						pole[x][y] = KORYTARZ;
						y++;
						pole[x][y] = GRACZ;
					}
					else if(event.key.keysym.sym == SDLK_LEFT && pole[x][y - 1] == KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK - i, x*ELEMENT + SRODEK);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK - i, x*ELEMENT + SRODEK);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA1);
						}
						pole[x][y] = KORYTARZ;
						y--;
						pole[x][y] = GRACZ;
					}
					else if (event.key.keysym.sym == SDLK_DOWN && pole[x+1][y] == KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK +i);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK +i);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA1);
						}
						pole[x][y] = KORYTARZ;
						x++;
						pole[x][y] = GRACZ;
					}
					else if (event.key.keysym.sym == SDLK_UP && pole[x-1][y] == KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK - i);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK - i);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA1);
						}
						pole[x][y] = KORYTARZ;
						x--;
						pole[x][y] = GRACZ;
					}
					else if (event.key.keysym.sym == SDLK_RIGHT && pole[x][y + 1] == SKRZYNIA && pole[x][y+2]==KORYTARZ ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK + i, x*ELEMENT + SRODEK);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK + i, x*ELEMENT + SRODEK);
							DrawSurface(screen, skrzynia, (y+1)*ELEMENT + SRODEK + i, x*ELEMENT + SRODEK);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA2);
						}
						pole[x][y] = KORYTARZ;
						y++;
						pole[x][y] = GRACZ;
						pole[x][y + 1] = SKRZYNIA;
					}
					else if (event.key.keysym.sym == SDLK_LEFT && pole[x][y - 1] == SKRZYNIA && pole[x][y - 2] == KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK - i, x*ELEMENT + SRODEK);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK - i, x*ELEMENT + SRODEK);
							DrawSurface(screen, skrzynia, (y - 1)*ELEMENT + SRODEK - i, x*ELEMENT + SRODEK);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA2);
						}
						pole[x][y] = KORYTARZ;
						y--;
						pole[x][y] = GRACZ;
						pole[x][y - 1] = SKRZYNIA;
					}
					else if (event.key.keysym.sym == SDLK_DOWN && pole[x+1][y] == SKRZYNIA && pole[x+2][y] == KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK + i);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK + i);
							DrawSurface(screen, skrzynia, y *ELEMENT + SRODEK, (x+1)*ELEMENT + SRODEK +i);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA2);
						}
						pole[x][y] = KORYTARZ;
						x++;
						pole[x][y] = GRACZ;
						pole[x+1][y] = SKRZYNIA;
					}
					else if (event.key.keysym.sym == SDLK_UP && pole[x-1][y] == SKRZYNIA && pole[x-2][y] == KORYTARZ) {
						for (int i = 0; i < ELEMENT; i++) {
							if (i % 2 == 0)DrawSurface(screen, gracz, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK - i);
							else DrawSurface(screen, gracz2, y*ELEMENT + SRODEK, x*ELEMENT + SRODEK - i);
							DrawSurface(screen, skrzynia, y *ELEMENT + SRODEK, (x - 1)*ELEMENT + SRODEK - i);
							SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
							SDL_RenderCopy(renderer, scrtex, NULL, NULL);
							SDL_RenderPresent(renderer);
							Sleep(ANIMACJA2);
						}
						pole[x][y] = KORYTARZ;
						x--;
						pole[x][y] = GRACZ;
						pole[x-1][y] = SKRZYNIA;
					}
					else if (event.key.keysym.sym == SDLK_n) {
						nowa(pole, cele);
						x = 1;
						y = 1;
						worldTime = 0;
					}
					else if (event.key.keysym.sym == SDLK_1) {
						worldTime = 0;
						x = 1; y = 1;
						wczytaj(pole, cele, fopen("1.txt", "r"));
					}
					else if (event.key.keysym.sym == SDLK_2) {
						worldTime = 0;
						x = 2; y = 2;
						wczytaj(pole, cele, fopen("2.txt","r"));
					}
					else if (event.key.keysym.sym == SDLK_3) {
						worldTime = 0;
						x = 1; y = 1;
						wczytaj(pole, cele, fopen("3.txt", "r"));
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};
	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
