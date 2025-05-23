/**
 * @file server.c
 * @date 2025-04-03
 * @author Daniel FERREIRA LARA
 *
 * @brief Implementation du serveur de jeu
 * @details
 * 	- Le serveur de jeu est responsable de la gestion des joueurs et de la logique du jeu.
 * 	- Il doit gérer les connexions des clients, distribuer les cartes et gérer le tour de jeu.
 * 	- Il doit également gérer les demandes des clients et envoyer les réponses appropriées.
 * 	- Le serveur doit être capable de gérer plusieurs clients en même temps.
 * 	- Il doit également gérer les erreurs et les exceptions.
 * 	- Le serveur doit être capable de gérer les demandes de connexion et de déconnexion des clients.
 * 	- Il doit également gérer les demandes de jeu et de fin de jeu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "infos.h"

typedef struct _c{
	char ipAdresse[16];
	int port;
	char nom[40];
} Client;
typedef struct _s{
	int socket;
	int port;
	int nbClients;

} Server;
typedef struct _p{
	Client client;
	bool eliminated;
	int score;
} Player;

Player clients[4];
Server server;


int deck[13]={0,1,2,3,4,5,6,7,8,9,10,11,12};
int tableCartes[4][8];


void melangerDeck()
{
		int i;
		int index1,index2,tmp;

		for (i=0;i<1000;i++)
		{
				index1=rand()%13;
				index2=rand()%13;

				tmp=deck[index1];
				deck[index1]=deck[index2];
				deck[index2]=tmp;
		}
}

void createTable()
{
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5 
	// Le joueur 2 possede les cartes d'indice 6,7,8 
	// Le joueur 3 possede les cartes d'indice 9,10,11 
	// Le coupable est la carte d'indice 12
	int i,j,c;

	for (i=0;i<4;i++)
		for (j=0;j<8;j++)
			tableCartes[i][j]=0;

	for (i=0;i<4;i++)
	{
		for (j=0;j<3;j++)
		{
			c=deck[i*3+j];
			switch (c)
			{
				case 0: // Sebastian Moran
					tableCartes[i][7]++;
					tableCartes[i][2]++;
					break;
				case 1: // Irene Adler
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					tableCartes[i][5]++;
					break;
				case 2: // Inspector Lestrade
					tableCartes[i][3]++;
					tableCartes[i][6]++;
					tableCartes[i][4]++;
					break;
				case 3: // Inspector Gregson 
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					tableCartes[i][4]++;
					break;
				case 4: // Inspector Baynes 
					tableCartes[i][3]++;
					tableCartes[i][1]++;
					break;
				case 5: // Inspector Bradstreet 
					tableCartes[i][3]++;
					tableCartes[i][2]++;
					break;
				case 6: // Inspector Hopkins 
					tableCartes[i][3]++;
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					break;
				case 7: // Sherlock Holmes 
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][2]++;
					break;
				case 8: // John Watson 
					tableCartes[i][0]++;
					tableCartes[i][6]++;
					tableCartes[i][2]++;
					break;
				case 9: // Mycroft Holmes 
					tableCartes[i][0]++;
					tableCartes[i][1]++;
					tableCartes[i][4]++;
					break;
				case 10: // Mrs. Hudson 
					tableCartes[i][0]++;
					tableCartes[i][5]++;
					break;
				case 11: // Mary Morstan 
					tableCartes[i][4]++;
					tableCartes[i][5]++;
					break;
				case 12: // James Moriarty 
					tableCartes[i][7]++;
					tableCartes[i][1]++;
					break;
			}
		}
	}
} 

void printDeck()
{
		int i,j;

		for (i=0;i<13;i++)
				printf("%d:%d\n",i,deck[i]);

	for (i=0;i<4;i++)
	{
		for (j=0;j<8;j++)
			printf("%2.2d ",tableCartes[i][j]);
		puts("");
	}
}

int main(int argc, char *argv[])
{
	printDeck();
	melangerDeck();
	createTable();
	printDeck();

	return 0; 
}
