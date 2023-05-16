# BE RESEAU

## Membre du groupe
    - Toumany Doumbouya
    
## Contenu du dépôt important
Ce dépôt inclu notre code source final pour mictcp se limitant à la V3 fonctionnelle  
  - src/mictcp.c : code source développé
  - README.md
  - tsock_texte et tsock_video : applications de test fournies 

## Compilation du protocole mictcp et lancement des applications de test fournies

Pour compiler mictcp et générer les exécutables des applications de test depuis le code source fourni, taper :

    make

Deux applications de test sont fournies, tsock_texte et tsock_video, elles peuvent être lancées soit en mode puits, soit en mode source selon la syntaxe suivante:

    Usage: ./tsock_texte [-p|-s]
    Usage: ./tsock_video [[-p|-s] [-t (tcp|mictcp)]

Seul tsock_video permet d'utiliser, au choix, votre protocole mictcp ou une émulation du comportement de tcp sur un réseau avec pertes.

## Etat d'avancement et infos

Les différentes versions implementées sont disponibles via les tags du depot. 

Je n'ai pas pu avancer **plus loin que la V3 de MICTCP** par manque de temps, d'autant plus que je travaillais seul sur le projet depuis le debut.

Les premières versions sont toutes fonctionnelles et pour la V3 vous pouvez jouer sur le taux de pertes admissibles en modifiant la valeur de la variable globale **LOSS_RATE_VALUE**.

## Choix d'implémentation Fiabilité partielle statique

La garantie de fiabilité partielle « statique » via un mécanisme de reprise des pertes de type « Stop and Wait » a été implémenté suivant l'idée de fenetre glissante de taille N fixée par le developpeur histoire de connaitre l'état des differents paquets transmis jusque là. On calcule alors apres chaque perte de paquet le taux de perte réellement obtenu des paquets de la fenetre glissante par rapport à celui fixé de base. Ce qui nous determine si une prochaine  perte est acceptable ou non. Les paquets de la fenetre glissante sont alors mis à jour pour compter les N derniersd paquets envoyés.     

