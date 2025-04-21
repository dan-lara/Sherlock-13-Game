#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "lib/inc/Logger.h"
#include "lib/src/Logger.c"

#define BLACK (SDL_Color){0, 0, 0}
#define RED (SDL_Color){204, 78, 60}
#define GREEN (SDL_Color){47, 182, 113}
#define BLUE (SDL_Color){0, 0, 255}

#define WIDTH 1024
#define HEIGHT 768

#define DEBUG 1
#define DEBUG_RANDOM 0

pthread_t thread_serveur_tcp_id;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char gbuffer[256];
char gServerIpAddress[256];
int gServerPort;
char gClientIpAddress[256];
int gClientPort;
char gName[256];
char gNames[4][256];
int gId;
int joueurSel;
int objetSel;
int guiltSel;
int guiltGuess[13];
int tableCartes[4][8];
int b[3];
int goEnabled;
int connectEnabled;

int eliminated[4] = {0, 0, 0, 0};
int gWinner = -1;
int replayEnabled = 0;
int nbReplay = 0;
char msg[256];

char *nbobjets[] = {"5", "5", "5", "5", "4", "3", "3", "3"};
char *nbnoms[] = {"Sebastian Moran", "irene Adler", "inspector Lestrade",
				  "inspector Gregson", "inspector Baynes", "inspector Bradstreet",
				  "inspector Hopkins", "Sherlock Holmes", "John Watson", "Mycroft Holmes",
				  "Mrs. Hudson", "Mary Morstan", "James Moriarty"};

volatile int synchro;

void rst_click()
{
	joueurSel = -1;
	objetSel = -1;
	guiltSel = -1;
}

void end_game(int i, int j)
{
	gWinner = i;
	int k;
	for (k = 0; k < 4; k++)
		if (k != gWinner)
			eliminated[k] = 1;
	if (gWinner == gId)
		sprintf(msg, "Dear %s, You win!     The guilty person was %s!", gName, nbnoms[j]);
	else
		sprintf(msg, "Dear %s, You lose!    The guilty person was %s!     Winner : %s", gName, nbnoms[j], gNames[gWinner]);
	log_message(LOG_INFO, "%s", msg);
	log_message(LOG_INFO, "=============End of the game=============");

	nbReplay = 0;
	replayEnabled = 1;

	rst_click();
}

void eliminate(int i, int j)
{
	if (i == gId)
	{
		sprintf(msg, "Dear %s, You are eliminated! %s wasn't guilty!", gName, nbnoms[j]);
		log_message(LOG_INFO, "%s", msg);
	}
	else{
		sprintf(msg, "%s is eliminated! %s wasn't guilty!", gNames[i], nbnoms[j]);
		log_message(LOG_INFO, "%s is eliminated! %s wasn't guilty!", gNames[i], nbnoms[j]);
	}
}

void *fn_serveur_tcp(void *arg)
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("sockfd error\n");
		exit(1);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = gClientPort;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("bind error\n");
		exit(1);
	}

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
		{
			printf("accept error\n");
			exit(1);
		}

		bzero(gbuffer, 256);
		n = read(newsockfd, gbuffer, 255);
		if (n < 0)
		{
			printf("read error\n");
			exit(1);
		}
		// printf("%s",gbuffer);

		synchro = 1;

		while (synchro)
			;
	}
}

void sendMessageToServer(char *ipAddress, int portno, char *mess)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char sendbuffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(ipAddress);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR connecting\n");
		exit(1);
	}

	sprintf(sendbuffer, "%s\n", mess);
	n = write(sockfd, sendbuffer, strlen(sendbuffer));

	close(sockfd);
}
void init_game()
{
	int i, j;


	for (i = 0; i < 4; i++)
		for (j = 0; j < 8; j++)
			tableCartes[i][j] = -1;

	for (i = 0; i < 13; i++)
		guiltGuess[i] = 0;

	for (i = 0; i < 3; i++)
		b[i] = -1;

	joueurSel = -1;
	objetSel = -1;
	guiltSel = -1;

	for (i = 0; i < 4; i++)
		eliminated[i] = 0;
	gWinner = -1;
}

int main(int argc, char **argv)
{
#if !DEBUG_RANDOM
	srand(time(NULL));
#endif

	int ret;
	int i, j;

	int joueurCourant = 0;

	int quit = 0;
	SDL_Event event;
	int mx, my;
	char sendBuffer[256];
	char lname[256];
	int id;

	if (argc < 6)
	{
		printf("<app> <Main server ip address> <Main server port> <Client ip address> <Client port> <player name>\n");
		exit(1);
	}

	strcpy(gServerIpAddress, argv[1]);
	gServerPort = atoi(argv[2]);
	strcpy(gClientIpAddress, argv[3]);
	gClientPort = atoi(argv[4]);
	strcpy(gName, argv[5]);
	char log_filename[256];
	sprintf(log_filename, "client_%s.log", gName);
	init_logger(log_filename, 0);

	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();

	SDL_Window *window = SDL_CreateWindow(gName,
										  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_Surface *deck[14], *objet[8], *gobutton, *connectbutton, *replaybutton;

	deck[0] = IMG_Load("assets/SH13_0.png");
	deck[1] = IMG_Load("assets/SH13_1.png");
	deck[2] = IMG_Load("assets/SH13_2.png");
	deck[3] = IMG_Load("assets/SH13_3.png");
	deck[4] = IMG_Load("assets/SH13_4.png");
	deck[5] = IMG_Load("assets/SH13_5.png");
	deck[6] = IMG_Load("assets/SH13_6.png");
	deck[7] = IMG_Load("assets/SH13_7.png");
	deck[8] = IMG_Load("assets/SH13_8.png");
	deck[9] = IMG_Load("assets/SH13_9.png");
	deck[10] = IMG_Load("assets/SH13_10.png");
	deck[11] = IMG_Load("assets/SH13_11.png");
	deck[12] = IMG_Load("assets/SH13_12.png");
	deck[13] = IMG_Load("assets/SH13_?.png");

	objet[0] = IMG_Load("assets/SH13_pipe_120x120.png");
	objet[1] = IMG_Load("assets/SH13_ampoule_120x120.png");
	objet[2] = IMG_Load("assets/SH13_poing_120x120.png");
	objet[3] = IMG_Load("assets/SH13_couronne_120x120.png");
	objet[4] = IMG_Load("assets/SH13_carnet_120x120.png");
	objet[5] = IMG_Load("assets/SH13_collier_120x120.png");
	objet[6] = IMG_Load("assets/SH13_oeil_120x120.png");
	objet[7] = IMG_Load("assets/SH13_crane_120x120.png");

	gobutton = IMG_Load("assets/go.png");
	connectbutton = IMG_Load("assets/connect.png");
	replaybutton = IMG_Load("assets/replay.png");

	strcpy(gNames[0], "-");
	strcpy(gNames[1], "-");
	strcpy(gNames[2], "-");
	strcpy(gNames[3], "-");

	init_game();

	goEnabled = 0;
	connectEnabled = 1;
	replayEnabled = 0;

	SDL_Texture *texture_deck[14], *texture_gobutton, *texture_connectbutton, *texture_replaybutton, *texture_objet[8];

	for (i = 0; i < 14; i++)
		texture_deck[i] = SDL_CreateTextureFromSurface(renderer, deck[i]);
	for (i = 0; i < 8; i++)
		texture_objet[i] = SDL_CreateTextureFromSurface(renderer, objet[i]);

	texture_gobutton = SDL_CreateTextureFromSurface(renderer, gobutton);
	texture_connectbutton = SDL_CreateTextureFromSurface(renderer, connectbutton);
	texture_replaybutton = SDL_CreateTextureFromSurface(renderer, replaybutton);

	TTF_Font *Sans = TTF_OpenFont("assets/sans.ttf", 16);
	TTF_Font *SansTitle = TTF_OpenFont("assets/sans.ttf", 24);
	TTF_SetFontStyle(SansTitle, TTF_STYLE_BOLD);
	printf("Sans=%p\n", Sans);

	/* Creation du thread serveur tcp. */
	printf("Creation du thread serveur tcp !\n");
	log_message(LOG_INFO, "Creation du thread serveur tcp !");
	synchro = 0;
	ret = pthread_create(&thread_serveur_tcp_id, NULL, fn_serveur_tcp, NULL);

	sprintf(msg, "Welcome, %s! Can you identify the guilty party?", gName);

	while (!quit)
	{
		if (SDL_PollEvent(&event))
		{
			// printf("un event\n");
			switch (event.type)
			{
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_GetMouseState(&mx, &my);
				// printf("mx=%d my=%d\n",mx,my);
				if ((mx < 200) && (my < 50))
				{
					if (connectEnabled==1)
					{
						sprintf(sendBuffer, "C %s %d %s", gClientIpAddress, gClientPort, gName);
						sendMessageToServer(gServerIpAddress, gServerPort, sendBuffer);
						log_message(LOG_DEBUG, "|-->C %s %d %s", gClientIpAddress, gClientPort, gName);
						connectEnabled = 0;
					}
					else if (replayEnabled==1)
					{
						init_game();
						sprintf(sendBuffer, "R %d", gId);
						sendMessageToServer(gServerIpAddress, gServerPort, sendBuffer);
						log_message(LOG_DEBUG, "|-->R %d", gId);
						replayEnabled = 0;
					}
				}
				else if ((mx >= 0) && (mx < 200) && (my >= 90) && (my < 330))
				{
					joueurSel = (my - 90) / 60;
					guiltSel = -1;
				}
				else if ((mx >= 200) && (mx < 680) && (my >= 0) && (my < 90))
				{
					objetSel = (mx - 200) / 60;
					guiltSel = -1;
				}
				else if ((mx >= 100) && (mx < 250) && (my >= 350) && (my < 740))
				{
					joueurSel = -1;
					objetSel = -1;
					guiltSel = (my - 350) / 30;
				}
				else if ((mx >= 250) && (mx < 300) && (my >= 350) && (my < 740))
				{
					int ind = (my - 350) / 30;
					guiltGuess[ind] = 1 - guiltGuess[ind];
				}
				else if ((mx >= 500) && (mx < 700) && (my >= 350) && (my < 450) && (goEnabled == 1))
				{
					printf("go! joueur=%d objet=%d guilt=%d\n", joueurSel, objetSel, guiltSel);
					log_message(LOG_INFO, "Played! joueur=%d objet=%d guilt=%d", joueurSel, objetSel, guiltSel);
					if (guiltSel != -1)
					{
						sprintf(sendBuffer, "G %d %d", gId, guiltSel);
						sendMessageToServer(gServerIpAddress, gServerPort, sendBuffer);
						log_message(LOG_DEBUG, "|-->G %d %d (%s, %s)", gId, guiltSel,
									gNames[gId], nbnoms[guiltSel]);
						goEnabled = 0;
					}
					else if ((objetSel != -1) && (joueurSel == -1))
					{
						sprintf(sendBuffer, "O %d %d", gId, objetSel);

						sendMessageToServer(gServerIpAddress, gServerPort, sendBuffer);
						log_message(LOG_DEBUG, "|-->O %d %d (%s, %s)", gId, objetSel,
									gNames[gId], nbnoms[objetSel]);
						goEnabled = 0;
					}
					else if ((objetSel != -1) && (joueurSel != -1))
					{
						sprintf(sendBuffer, "S %d %d %d", gId, joueurSel, objetSel);

						sendMessageToServer(gServerIpAddress, gServerPort, sendBuffer);
						log_message(LOG_DEBUG, "|-->S %d %d %d (%s, %s)", gId, joueurSel, objetSel,
									gNames[gId], nbnoms[joueurSel]);
						goEnabled = 0;
					}
				}
				else
					rst_click();
				break;
			case SDL_MOUSEMOTION:
				SDL_GetMouseState(&mx, &my);
				break;
			}
		}

		if (synchro == 1)
		{
			printf("<--|%s\n", gbuffer);
			// log_message(LOG_DEBUG, "<--|%s", gbuffer);
			switch (gbuffer[0])
			{
			// Message 'I' : le joueur recoit son Id
			case 'I':
				sscanf(gbuffer, "I %d", &gId);
				log_message(LOG_DEBUG, "<--|I %d (%s)", gId, gName);
				log_message(LOG_INFO, "The %s user ID is :%d", gName, gId);
				break;
			// Message 'L' : le joueur recoit la liste des joueurs
			case 'L':
				sscanf(gbuffer, "L %s %s %s %s",
					   gNames[0], gNames[1], gNames[2], gNames[3]);
				log_message(LOG_DEBUG, "<--|L %s %s %s %s",
							gNames[0], gNames[1], gNames[2], gNames[3]);
				sprintf(msg, "Welcome, %s! Waiting...", gName);
				break;
			// Message 'D' : le joueur recoit ses trois cartes
			case 'D':
				sscanf(gbuffer, "D %d %d %d", &b[0], &b[1], &b[2]);
				log_message(LOG_DEBUG, "<--|D %d %d %d (%s, %s, %s)", b[0], b[1], b[2],
							nbnoms[b[0]], nbnoms[b[1]], nbnoms[b[2]]);
				guiltGuess[b[0]] = 1;
				guiltGuess[b[1]] = 1;
				guiltGuess[b[2]] = 1;
				sprintf(msg, "You have your cards!");
				break;
			// Message 'M' : le joueur recoit le n° du joueur courant
			// Cela permet d'affecter goEnabled pour autoriser l'affichage du bouton go
			case 'M':
				sscanf(gbuffer, "M %d", &joueurCourant);
				goEnabled = (joueurCourant == gId) ? 1 : 0;
				log_message(LOG_DEBUG, "<--|M %d (%s)", joueurCourant,
							gNames[joueurCourant]);
				break;
			// Message 'V' : le joueur recoit une valeur de tableCartes
			case 'V':
			{
				int i, j, n;
				sscanf(gbuffer, "V %d %d %d", &i, &j, &n);
				if (tableCartes[i][j] == -1 || tableCartes[i][j] == 0 || tableCartes[i][j] == 100)
					tableCartes[i][j] = n;
				log_message(LOG_DEBUG, "<--|V %d %d %d (%s, %s)", i, j, n,
							gNames[i], nbnoms[j]);
			}
			break;
			case 'E':
				sscanf(gbuffer, "E %d %d", &i, &j);
				eliminated[i] = 1;
				guiltGuess[j] = 1;
				log_message(LOG_DEBUG, "<--|E %d %d (%s, %s)", i, j,
							gNames[i], nbnoms[j]);
				eliminate(i, j);
				break;
			// Message 'W' : le joueur recoit le numéro du joueur victorieux et le coupable
			case 'W':
				sscanf(gbuffer, "W %d %d", &i, &j);
				joueurCourant = -1;
				rst_click();
				log_message(LOG_DEBUG, "<--|W %d %d (%s, %s)", i, j,
							gNames[i], nbnoms[j]);

				end_game(i, j);
				break;
			case 'R':
				sscanf(gbuffer, "R %d", &i);
				nbReplay++;
				sprintf(msg, "Player %s is ready! (%d/4)", gNames[i], nbReplay);
				log_message(LOG_DEBUG, "<--|R %d (%s)", i, gNames[i]);
				break;
			}
			synchro = 0;
		}

		SDL_Rect dstrect_grille = {512 - 250, 10, 500, 350};
		SDL_Rect dstrect_image = {0, 0, 500, 330};
		SDL_Rect dstrect_image1 = {0, 340, 250, 330 / 2};

		// SDL_SetRenderDrawColor(renderer, 222, 184, 135, 255); // Brun clair (beige de vieux papier)
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanc
		SDL_Rect rect = {0, 0, WIDTH, HEIGHT};
		SDL_RenderFillRect(renderer, &rect);

		if (joueurSel != -1)
		{
			SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Doré
			SDL_Rect rect1 = {0, 90 + joueurSel * 60, 200, 60};
			SDL_RenderFillRect(renderer, &rect1);
		}

		if (objetSel != -1)
		{
			SDL_SetRenderDrawColor(renderer, 0, 255, 127, 255); // Vert brillant
			SDL_Rect rect1 = {200 + objetSel * 60, 0, 60, 90};
			SDL_RenderFillRect(renderer, &rect1);
		}

		if (guiltSel != -1)
		{
			SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255); // Bleu clair
			SDL_Rect rect1 = {100, 350 + guiltSel * 30, 150, 30};
			SDL_RenderFillRect(renderer, &rect1);
		}

		{
			SDL_Rect dstrect_pipe = {210, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[0], NULL, &dstrect_pipe);
			SDL_Rect dstrect_ampoule = {270, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[1], NULL, &dstrect_ampoule);
			SDL_Rect dstrect_poing = {330, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[2], NULL, &dstrect_poing);
			SDL_Rect dstrect_couronne = {390, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[3], NULL, &dstrect_couronne);
			SDL_Rect dstrect_carnet = {450, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[4], NULL, &dstrect_carnet);
			SDL_Rect dstrect_collier = {510, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[5], NULL, &dstrect_collier);
			SDL_Rect dstrect_oeil = {570, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[6], NULL, &dstrect_oeil);
			SDL_Rect dstrect_crane = {630, 10, 40, 40};
			SDL_RenderCopy(renderer, texture_objet[7], NULL, &dstrect_crane);
		}

		SDL_Color col1 = {0, 0, 0};
		for (i = 0; i < 8; i++)
		{
			SDL_Surface *surfaceMessage = TTF_RenderText_Solid(SansTitle, nbobjets[i], col1);
			SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

			SDL_Rect Message_rect;				// create a rect
			Message_rect.x = 230 + i * 60;		// controls the rect's x coordinate
			Message_rect.y = 50;				// controls the rect's y coordinte
			Message_rect.w = surfaceMessage->w; // controls the width of the rect
			Message_rect.h = surfaceMessage->h; // controls the height of the rect

			SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
			SDL_DestroyTexture(Message);
			SDL_FreeSurface(surfaceMessage);
		}

		for (i = 0; i < 13; i++)
		{
			SDL_Surface *surfaceMessage = TTF_RenderText_Solid(Sans, nbnoms[i], col1);
			SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

			SDL_Rect Message_rect;
			Message_rect.x = 105;
			Message_rect.y = 350 + i * 30;
			Message_rect.w = surfaceMessage->w;
			Message_rect.h = surfaceMessage->h;

			SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
			SDL_DestroyTexture(Message);
			SDL_FreeSurface(surfaceMessage);
		}

		for (i = 0; i < 4; i++)
			for (j = 0; j < 8; j++)
			{
				if (tableCartes[i][j] != -1)
				{
					char mess[10];
					if (tableCartes[i][j] == 100)
						sprintf(mess, "*");
					else
						sprintf(mess, "%d", tableCartes[i][j]);
					SDL_Surface *surfaceMessage = TTF_RenderText_Solid(SansTitle, mess, col1);
					SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

					SDL_Rect Message_rect;
					Message_rect.x = 230 + j * 60;
					Message_rect.y = 110 + i * 60;
					Message_rect.w = surfaceMessage->w;
					Message_rect.h = surfaceMessage->h;

					SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
					SDL_DestroyTexture(Message);
					SDL_FreeSurface(surfaceMessage);
				}
			}

		// Sebastian Moran
		{
			SDL_Rect dstrect_crane = {0, 350, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[7], NULL, &dstrect_crane);
		}
		{
			SDL_Rect dstrect_poing = {30, 350, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[2], NULL, &dstrect_poing);
		}
		// Irene Adler
		{
			SDL_Rect dstrect_crane = {0, 380, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[7], NULL, &dstrect_crane);
		}
		{
			SDL_Rect dstrect_ampoule = {30, 380, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[1], NULL, &dstrect_ampoule);
		}
		{
			SDL_Rect dstrect_collier = {60, 380, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[5], NULL, &dstrect_collier);
		}
		// Inspector Lestrade
		{
			SDL_Rect dstrect_couronne = {0, 410, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[3], NULL, &dstrect_couronne);
		}
		{
			SDL_Rect dstrect_oeil = {30, 410, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[6], NULL, &dstrect_oeil);
		}
		{
			SDL_Rect dstrect_carnet = {60, 410, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[4], NULL, &dstrect_carnet);
		}
		// Inspector Gregson
		{
			SDL_Rect dstrect_couronne = {0, 440, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[3], NULL, &dstrect_couronne);
		}
		{
			SDL_Rect dstrect_poing = {30, 440, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[2], NULL, &dstrect_poing);
		}
		{
			SDL_Rect dstrect_carnet = {60, 440, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[4], NULL, &dstrect_carnet);
		}
		// Inspector Baynes
		{
			SDL_Rect dstrect_couronne = {0, 470, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[3], NULL, &dstrect_couronne);
		}
		{
			SDL_Rect dstrect_ampoule = {30, 470, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[1], NULL, &dstrect_ampoule);
		}
		// Inspector Bradstreet
		{
			SDL_Rect dstrect_couronne = {0, 500, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[3], NULL, &dstrect_couronne);
		}
		{
			SDL_Rect dstrect_poing = {30, 500, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[2], NULL, &dstrect_poing);
		}
		// Inspector Hopkins
		{
			SDL_Rect dstrect_couronne = {0, 530, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[3], NULL, &dstrect_couronne);
		}
		{
			SDL_Rect dstrect_pipe = {30, 530, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[0], NULL, &dstrect_pipe);
		}
		{
			SDL_Rect dstrect_oeil = {60, 530, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[6], NULL, &dstrect_oeil);
		}
		// Sherlock Holmes
		{
			SDL_Rect dstrect_pipe = {0, 560, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[0], NULL, &dstrect_pipe);
		}
		{
			SDL_Rect dstrect_ampoule = {30, 560, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[1], NULL, &dstrect_ampoule);
		}
		{
			SDL_Rect dstrect_poing = {60, 560, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[2], NULL, &dstrect_poing);
		}
		// John Watson
		{
			SDL_Rect dstrect_pipe = {0, 590, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[0], NULL, &dstrect_pipe);
		}
		{
			SDL_Rect dstrect_oeil = {30, 590, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[6], NULL, &dstrect_oeil);
		}
		{
			SDL_Rect dstrect_poing = {60, 590, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[2], NULL, &dstrect_poing);
		}
		// Mycroft Holmes
		{
			SDL_Rect dstrect_pipe = {0, 620, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[0], NULL, &dstrect_pipe);
		}
		{
			SDL_Rect dstrect_ampoule = {30, 620, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[1], NULL, &dstrect_ampoule);
		}
		{
			SDL_Rect dstrect_carnet = {60, 620, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[4], NULL, &dstrect_carnet);
		}
		// Mrs. Hudson
		{
			SDL_Rect dstrect_pipe = {0, 650, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[0], NULL, &dstrect_pipe);
		}
		{
			SDL_Rect dstrect_collier = {30, 650, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[5], NULL, &dstrect_collier);
		}
		// Mary Morstan
		{
			SDL_Rect dstrect_carnet = {0, 680, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[4], NULL, &dstrect_carnet);
		}
		{
			SDL_Rect dstrect_collier = {30, 680, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[5], NULL, &dstrect_collier);
		}
		// James Moriarty
		{
			SDL_Rect dstrect_crane = {0, 710, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[7], NULL, &dstrect_crane);
		}
		{
			SDL_Rect dstrect_ampoule = {30, 710, 30, 30};
			SDL_RenderCopy(renderer, texture_objet[1], NULL, &dstrect_ampoule);
		}

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

		// Afficher les suppositions
		for (i = 0; i < 13; i++)
			if (guiltGuess[i])
			{
				SDL_RenderDrawLine(renderer, 250, 350 + i * 30, 300, 380 + i * 30);
				SDL_RenderDrawLine(renderer, 250, 380 + i * 30, 300, 350 + i * 30);
			}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawLine(renderer, 0, 30 + 60, 680, 30 + 60);
		SDL_RenderDrawLine(renderer, 0, 30 + 120, 680, 30 + 120);
		SDL_RenderDrawLine(renderer, 0, 30 + 180, 680, 30 + 180);
		SDL_RenderDrawLine(renderer, 0, 30 + 240, 680, 30 + 240);
		SDL_RenderDrawLine(renderer, 0, 30 + 300, 680, 30 + 300);

		SDL_RenderDrawLine(renderer, 200, 0, 200, 330);
		SDL_RenderDrawLine(renderer, 260, 0, 260, 330);
		SDL_RenderDrawLine(renderer, 320, 0, 320, 330);
		SDL_RenderDrawLine(renderer, 380, 0, 380, 330);
		SDL_RenderDrawLine(renderer, 440, 0, 440, 330);
		SDL_RenderDrawLine(renderer, 500, 0, 500, 330);
		SDL_RenderDrawLine(renderer, 560, 0, 560, 330);
		SDL_RenderDrawLine(renderer, 620, 0, 620, 330);
		SDL_RenderDrawLine(renderer, 680, 0, 680, 330);

		for (i = 0; i < 14; i++)
			SDL_RenderDrawLine(renderer, 0, 350 + i * 30, 300, 350 + i * 30);
		SDL_RenderDrawLine(renderer, 100, 350, 100, 740);
		SDL_RenderDrawLine(renderer, 250, 350, 250, 740);
		SDL_RenderDrawLine(renderer, 300, 350, 300, 740);

		// SDL_RenderCopy(renderer, texture_grille, NULL, &dstrect_grille);
		if (b[0] != -1)
		{
			SDL_Rect dstrect = {750, 0, 1000 / 4, 660 / 4};
			SDL_RenderCopy(renderer, texture_deck[b[0]], NULL, &dstrect);
		}
		else
		{
			SDL_Rect dstrect = {750, 0, 1000 / 4, 660 / 4};
			SDL_RenderCopy(renderer, texture_deck[13], NULL, &dstrect);
		}
		if (b[1] != -1)
		{
			SDL_Rect dstrect = {750, 200, 1000 / 4, 660 / 4};
			SDL_RenderCopy(renderer, texture_deck[b[1]], NULL, &dstrect);
		}
		else
		{
			SDL_Rect dstrect = {750, 200, 1000 / 4, 660 / 4};
			SDL_RenderCopy(renderer, texture_deck[13], NULL, &dstrect);
		}
		if (b[2] != -1)
		{
			SDL_Rect dstrect = {750, 400, 1000 / 4, 660 / 4};
			SDL_RenderCopy(renderer, texture_deck[b[2]], NULL, &dstrect);
		}
		else
		{
			SDL_Rect dstrect = {750, 400, 1000 / 4, 660 / 4};
			SDL_RenderCopy(renderer, texture_deck[13], NULL, &dstrect);
		}

		// Le bouton go
		if (goEnabled == 1)
		{
			SDL_Rect dstrect = {500, 370, 200, 100};
			SDL_RenderCopy(renderer, texture_gobutton, NULL, &dstrect);
		}
		// Le bouton connect
		if (connectEnabled == 1)
		{
			SDL_Rect dstrect = {0, 0, 200, 50};
			SDL_RenderCopy(renderer, texture_connectbutton, NULL, &dstrect);
		}
		else if (replayEnabled == 1)
		{
			SDL_Rect dstrect = {0, 0, 200, 50};
			SDL_RenderCopy(renderer, texture_replaybutton, NULL, &dstrect);
		}

		// SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		// SDL_RenderDrawLine(renderer, 0, 0, 200, 200);

		SDL_Color col = {0, 0, 0};
		for (i = 0; i < 4; i++)
			if (strlen(gNames[i]) > 0)
			{
				SDL_Surface *surfaceMessage = NULL;
				if (gWinner != -1 && gWinner == i)
				{
					col = GREEN;
					surfaceMessage = TTF_RenderText_Solid(SansTitle, gNames[i], col);
				}
				else if (eliminated[i])
				{ // Si éliminé, en rouge
					col = RED;
					surfaceMessage = TTF_RenderText_Solid(SansTitle, gNames[i], col);
				}
				else if (i == joueurCourant)
				{
					col = BLUE;
					surfaceMessage = TTF_RenderText_Solid(SansTitle, gNames[i], col);
				}
				else
				{
					col = BLACK;
					surfaceMessage = TTF_RenderText_Solid(SansTitle, gNames[i], col);
				}

				// SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, gNames[i], col);
				SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

				SDL_Rect Message_rect;				// create a rect
				Message_rect.x = 10;				// controls the rect's x coordinate
				Message_rect.y = 110 + i * 60;		// controls the rect's y coordinte
				Message_rect.w = surfaceMessage->w; // controls the width of the rect
				Message_rect.h = surfaceMessage->h; // controls the height of the rect

				SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
				SDL_DestroyTexture(Message);
				SDL_FreeSurface(surfaceMessage);
			}

		if (gWinner != -1)
		{
			if (gId == gWinner)
				col = GREEN;
			else
				col = RED;
		}
		else
			col = BLACK;
		SDL_Surface *surfaceMessage = TTF_RenderText_Solid(SansTitle, msg, col);
		SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

		SDL_Rect Message_rect;				// create a rect
		Message_rect.x = 350;				// controls the rect's x coordinate
		Message_rect.y = 700;				// controls the rect's y coordinte
		Message_rect.w = surfaceMessage->w; // controls the width of the rect
		Message_rect.h = surfaceMessage->h; // controls the height of the rect

		SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
		SDL_DestroyTexture(Message);
		SDL_FreeSurface(surfaceMessage);

		SDL_RenderPresent(renderer);
	}
	SDL_DestroyTexture(texture_deck[0]);
	SDL_DestroyTexture(texture_deck[1]);
	SDL_FreeSurface(deck[0]);
	SDL_FreeSurface(deck[1]);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
