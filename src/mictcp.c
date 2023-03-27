#include <mictcp.h>
#include <api/mictcp_core.h>

// Mode debug ou pas pour afficher les informations des fonctions appelles ou pas 
#define DEBUG_MODE 0
// Loss rate value
#define LOSS_RATE_VALUE 0

// Nombre de socket maximal que notre programme peut supporter
#define NB_MAX_SOCKET 10

/**
 * @brief Varible determinant si on effectue une phase d'etablissement de connexion
 * 0 = NON
 * 1 = OUI
 */
#define HANDSHAKE 0

// Notre tableau de socket
mic_tcp_sock ourSocketTab[NB_MAX_SOCKET];

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
    if( DEBUG_MODE ){
        printf("[MIC-TCP] Appel de la fonction: ");printf(__FUNCTION__);printf("\n");
    }

    int result = -1;
    result = initialize_components(sm); /* Appel obligatoire */
    char *cote;
    mic_tcp_sock ourSocket;
    ourSocket.fd = 0;

    if( result != -1 ){
        result = ourSocket.fd;
    } else{
        if( sm == 0)
            cote = "client";
        else
            cote = "serveur";
        printf("Impossible de creer la socket du cote %s\n",cote);
    }

    set_loss_rate(LOSS_RATE_VALUE);

    return result;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
    if (DEBUG_MODE) {
        printf("[MIC-TCP] Appel de la fonction: ");
        printf(__FUNCTION__);
        printf("\n");
    }

    if (socket == -1) {
        printf("Impossible d'effectuer le bind");
        return -1;
    }

    ourSocketTab[socket].state = IDLE;
    ourSocketTab[socket].addr = addr;
    return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr *addr)
{

    if (DEBUG_MODE){
        printf("[MIC-TCP] Appel de la fonction: ");
        printf(__FUNCTION__);
        printf("\n");
    }

    if( !HANDSHAKE ) {
        ourSocketTab[socket].state = ESTABLISHED;
        return 0;
    }

    // ICI ON GERE LE CAS OU IL FAUT EFFECTUER UNE PHASE D'ETABLISSEMENT DE CONNECTION 
    return -1;

}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    if(DEBUG_MODE){
        printf("[MIC-TCP] Appel de la fonction: ");
        printf(__FUNCTION__);
        printf("\n");
    }
    
    if(!HANDSHAKE){
        ourSocketTab[socket].state = ESTABLISHED;
        ourSocketTab[socket].addr = addr;
        return 0;
    }
    
    // ICI ON GERE LE CAS OU IL FAUT EFFECTUER UNE PHASE D'ETABLISSEMENT DE CONNECTION 
    return -1;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send(int mic_sock, char *mesg, int mesg_size)
{
    if(DEBUG_MODE){
        printf("[MIC-TCP] Appel de la fonction: ");
        printf(__FUNCTION__);
        printf("\n");
    }

    if(ourSocketTab[mic_sock].state != ESTABLISHED){
        printf("Impossible d'envoyer des donnees applicative\n");
        return -1;
    }

    mic_tcp_pdu pdu_to_send;
    mic_tcp_sock_addr addr_to_sent = ourSocketTab[mic_sock].addr;
    pdu_to_send.header.dest_port = addr_to_sent.port;
    pdu_to_send.payload.data = mesg;
    pdu_to_send.payload.size = mesg_size;

    int nb_octets_sent = -1;
    
    nb_octets_sent = IP_send(pdu_to_send,addr_to_sent);

    if(DEBUG_MODE){
        if(nb_octets_sent != -1)
            printf("Vous venez d'envoyer %d octets de donnees\n",nb_octets_sent);
        else
            printf("Erreur lors de l'envoi des donnees applicatives\n");
    }

    return nb_octets_sent;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv(int socket, char *mesg, int max_mesg_size)
{
    if(DEBUG_MODE){
        printf("[MIC-TCP] Appel de la fonction: ");
        printf(__FUNCTION__);
        printf("\n");
    }

    if(ourSocketTab[socket].state != ESTABLISHED){
        printf("Impossible de recevoir les donnees, vous devez etre connecte a une autre machine pour cela\n");
        return -1;
    }

    int delivered_size = -1;

    mic_tcp_payload payload;
    payload.data = mesg;
    payload.size = max_mesg_size;

    delivered_size = app_buffer_get(payload);

    return delivered_size;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close(int socket)
{
    if (DEBUG_MODE)
    {
        printf("[MIC-TCP] Appel de la fonction :  ");
        printf(__FUNCTION__);
        printf("\n");
    }
    
    ourSocketTab[socket].state = CLOSED;
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    if(DEBUG_MODE){
        printf("[MIC-TCP] Appel de la fonction: ");
        printf(__FUNCTION__);
        printf("\n");
    }

    app_buffer_put(pdu.payload);
}
