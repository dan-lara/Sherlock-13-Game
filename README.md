# Sherlock 13 - Jeu en C pour 4 Joueurs

Bienvenue dans l'implémentation du jeu **Sherlock 13** en langage C, conçu pour 4 joueurs. Ce projet recrée l'expérience du jeu de société en mettant l'accent sur la logique et la déduction.

## Fonctionnement Technique

Ce projet utilise un serveur écrit en C pour gérer les interactions entre les joueurs. Le fonctionnement est orchestré via websockets et utilise des threads, permettant une gestion simultanée des actions de chaque joueur. Pour plus de détails techniques, consultez le [rapport](doc/rapport.pdf).

---

## Installation

Pour installer et exécuter le projet, suivez les étapes ci-dessous :

1. Clonez le dépôt GitHub :

    ```bash
    git clone https://github.com/dan-lara/Sherlock-13-Game.git
    cd Sherlock-13-Game
    ```

2. Construisez le projet avec `make` :

    ```bash
    make all
    ```

3. Lancez le jeu (serveur et client) :

    ```bash
    make run_server 30000
    # ou ./build/server 
    ```

    Chaque client doit être dans un terminal distinct.

    ```bash
    ./build/client localhost 30000 localhost 40000 Abgail
    ./build/client localhost 30000 localhost 40001 Bernard
    ./build/client localhost 30000 localhost 40002 Charles
    ./build/client localhost 30000 localhost 40003 Daniel

    ```

4. Nettoyez les fichiers de construction si nécessaire :

    ```bash
    make clean
    ```

## Démonstration

Ajoutez ici une capture d'écran ou une photo montrant le jeu en action :

![Démonstration du jeu](doc/screenshots/Play_Idle.png)
![Démonstration du jeu](doc/screenshots/Replay_waiting.png)

---

## Architecture

Voici un schéma de l'architecture du projet pour mieux comprendre son organisation :

```mermaid
sequenceDiagram
    title    Messagerie
    participant Client
    participant Server

    %% Connexion
    Note over Client,Server: Étape 1 - Connexion au serveur
    Client->>+Server: C <IP> <Port> <Name> (Connexion au serveur)
    Server-->>Client: I <ID> (Identifiant du joueur)
    Server-->>-Client: L <Name> <Name> <Name> <Name> (Liste des joueurs connectés)

    %% Début du jeu
    Note over Client,Server: Étape 2 - Début du jeu (quand le quatrième joueur se connecte)
    Server-->>Client: D <Suspect1> <Suspect2> <Suspect3> (Cartes du joueur)
    Server-->>Client: V <ID> <Symbole> <Valeur> (Mise à jour de la table des symboles)
    Server-->>Client: M <ID> (Joueur actuel)

    %% Actions d'investigation
    Note over Client,Server: Étape 3 - Actions d'investigation
    Client->>Server: O <ID> <Symbole> ou S <ID> <Joueur> <Symbole> (Action d'investigation)
    Server-->>Client: V <ID> <Symbole> <Valeur> (Mise à jour de la table des symboles)
    Server-->>Client: M <ID> (Nouveau joueur actuel)

    %% Accusation
    Note over Client,Server: Étape 4 - Accusation
    Client->>Server: G <ID> <Suspect> (Accusation)
    alt Accusation correcte
        Server-->>Client: W <ID> <Suspect> (Gagnant et coupable)
    else Accusation incorrecte
        Server-->>Client: E <ID> <Suspect> (Joueur éliminé et suspect innocent)
        Server-->>Client: M <ID> (Nouveau joueur actuel si le jeu continue)
    end

    %% Rejouer
    Note over Client,Server: Étape 5 - Rejouer
    Client->>Server: R <ID> (Demande de rejouer)
    Server-->>Client: R <ID> (Diffusion de la demande de rejouer)
    Note over Client,Server: Si tous les joueurs demandent à rejouer, le jeu redémarre
    Server-->>Client: D <Suspect1> <Suspect2> <Suspect3> (Nouvelle distribution des cartes)
    Server-->>Client: V <ID> <Symbole> <Valeur> (Réinitialisation de la table des symboles)
    Server-->>Client: M <ID> (Nouveau joueur actuel)
```

## Règles du Jeu

Le jeu suit les règles classiques de **Sherlock 13**. Chaque joueur doit utiliser la logique et poser des questions pour identifier le coupable parmi les suspects. Vous pouvez consulter les règles complètes [ici](doc/regle.pdf).

---

## Contribuer

Les contributions sont les bienvenues ! N'hésitez pas à ouvrir une issue ou soumettre une pull request.
