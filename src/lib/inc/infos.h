#ifndef INFOS_H
#define INFOS_H

#define NB_CARTES 13
#define NB_JOUEURS 4
#define NB_CARTES_PAR_JOUEUR 3
#define NB_SYMBOLES 8
#define BUF_SIZE 256
#define BUF_NOM 40
#define IPADDR_SIZE 16
#include <stdio.h>

typedef struct _c{
	char ipAdresse[IPADDR_SIZE]; // IPv4
	int port; 					// Port de la socket
	char nom[BUF_NOM]; 		// Nom du client
	int _socket; 				// Socket du client
} Client;

typedef struct _s{
	int socket; 		// Socket du serveur
	int port; 			// Port du serveur
	int nbClients; 		// Nombre de clients connectes
} Server;

typedef struct _p{
	Client client; 		// Client
	bool eliminated; 	// Indique si le joueur est elimine
	int score; 			// Score du joueur
} Player;

static char *nbobjets[NB_SYMBOLES] = {"5", "5", "5", "5", "4", "3", "3", "3"};
static char *Cards[NB_CARTES] = {
    "Sebastian Moran",
	"Irene Adler",
	"Inspector Lestrade",
	"Inspector Gregson",
	"Inspector Baynes",
	"Inspector Bradstreet",
	"Inspector Hopkins",
	"Sherlock Holmes",
	"John Watson",
	"Mycroft Holmes",
	"Mrs. Hudson",
	"Mary Morstan",
	"James Moriarty"
};

#endif