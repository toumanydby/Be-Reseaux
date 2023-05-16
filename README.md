# BE RESEAU

## Membre du groupe
    - Toumany Doumbouya
    
## Compilation du protocole mictcp et lancement des applications de test fournies

Pour compiler mictcp et générer les exécutables des applications de test depuis le code source fourni, taper :

    make

Deux applications de test sont fournies, tsock_texte et tsock_video, elles peuvent être lancées soit en mode puits, soit en mode source selon la syntaxe suivante:

    Usage: ./tsock_texte [-p|-s]
    Usage: ./tsock_video [[-p|-s] [-t (tcp|mictcp)]

Seul tsock_video permet d'utiliser, au choix, votre protocole mictcp ou une émulation du comportement de tcp sur un réseau avec pertes.

Les differentes versions implementees sont disponible via les tags du depot. 

Je n'ai pas pu avancer **plus loin que la V3 de MICTCP** par manque de temps, d'autant plus que je travaillais seul sur le projet depuis le debut.

Les premières versions sont toutes fonctionnelles et pour la V3 vous pouvez jouer sur le taux de pertes admissibles en modifiant la valeur de la variable globale **LOSS_RATE_VALUE**.
