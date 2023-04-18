#include <mictcp.h>
#include <api/mictcp_core.h>

// Mode debug ou pas pour afficher les informations des fonctions appelles ou pas 
#define DEBUG_MODE 1

// Loss rate value
#define LOSS_RATE_VALUE 0

// Taux des paquets perdus et abandonnes. Si ADMITTED_LOSS_RATE = 10, 10% des paquets perdus ne seront pas renvoyes.
#define ADMITTED_LOSS_RATE 0

// Nombre de socket maximal que notre programme peut supporter
#define NB_MAX_SOCKET 10

/**
 * @brief Varible determinant si on effectue une phase d'etablissement de connexion
 * 0 = NON
 * 1 = OUI
 */
#define HANDSHAKE 0

// Temps d'attente en millisecondes
#define TIMEOUT 5

// Notre tableau de socket
mySocket ourSocketTab[NB_MAX_SOCKET];

// indice du socket sur lequel on travaille
int id_sock = 0;

int nb_rcv_paquets = 0;      /* Nombre de paquets reçus */
int nb_loss_paquets = 0;      /* Nombre de paquets perdus */
int nb_send_paquets = 0;      /* Nombre de paquets envoyés */
int nb_loss_resend = 0;      /* Nombre des pertes renvoyées */
int nb_loss_left = 0;      /* Nombre des pertes abandonnees */


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

    ourSocketTab[socket].socket.state = IDLE;
    ourSocketTab[socket].socket.addr = addr;
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
        ourSocketTab[socket].socket.state = ESTABLISHED;
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
    ourSocketTab[socket].num_seq_local = 0;
    ourSocketTab[socket].num_seq_distant = 0;
    ourSocketTab[socket].addr_distante = addr; // addresse destinataire
    
    if(!HANDSHAKE){
        ourSocketTab[socket].socket.state = ESTABLISHED;
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

    if(ourSocketTab[mic_sock].socket.state != ESTABLISHED){
        printf("Impossible d'envoyer des donnees applicative\n");
        return -1;
    }

    mic_tcp_pdu pdu_to_send, pdu_to_rcv;
    initialise_to_null_pdu(&pdu_to_send);
    mic_tcp_sock_addr addr_to_sent = ourSocketTab[mic_sock].addr_distante;
    pdu_to_send.header.source_port = ourSocketTab[mic_sock].socket.addr.port;
    pdu_to_send.header.dest_port = ourSocketTab[mic_sock].addr_distante.port;
    pdu_to_send.header.seq_num = ourSocketTab[mic_sock].num_seq_local;
    pdu_to_send.payload.data = mesg;
    pdu_to_send.payload.size = mesg_size;

    // On envoie les donnees
    int size_octets_sent;
    int size_pdu_rcv;
    ourSocketTab[mic_sock].num_seq_local = (ourSocketTab[mic_sock].num_seq_local+1) % 2;

    // Tant qu'on a pas recu le pdu qui dit que la donnee a bien ete recu, on tourne cette boucle
    while (1)
    {
        size_octets_sent = IP_send(pdu_to_send,addr_to_sent);
        if( size_octets_sent == -1) {
            printf("[ERROR] Echec de l'envoie des donnees\n");
            return -1;
        }

        initialise_to_null_pdu(&pdu_to_rcv);
        size_pdu_rcv = IP_recv(&pdu_to_rcv,&(ourSocketTab[mic_sock].addr_distante),TIMEOUT);
        
        print_mic_tcp_pdu_infos(pdu_to_rcv,"PDU RCV\n");

        // Si on est dans ce cas alors on a recu le bon ack 
        if((pdu_to_rcv.header.ack == 1)
            && (size_pdu_rcv != -1)
        ){
            nb_send_paquets++;
            break;
        }

        // Si on est toujours pas hors de la boucle alors notre paquet s'est perdu
        nb_loss_paquets++;
        
        if(accept_loss()){
            nb_loss_left++;
        }else{
            nb_loss_resend++;
        }
    }
    
    if(DEBUG_MODE){
        if(size_octets_sent != -1){
            printf("Vous venez d'envoyer %d octets de donnees\n",size_octets_sent);
            printf("[INFO] Nombre de paquets envoyes : %d\n",nb_send_paquets);
        }
        else
            printf("Erreur lors de l'envoi des donnees\n");
    }

    return size_octets_sent;
}


int accept_loss(){
    return 0;
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

    if(ourSocketTab[socket].socket.state != ESTABLISHED){
        printf("Impossible de recevoir les donnees, vous devez etre connecte a une autre machine pour cela\n");
        return -1;
    }

    int delivered_size = -1;

    mic_tcp_payload *payload = malloc(sizeof(mic_tcp_payload));

    payload->data = mesg;
    payload->size = max_mesg_size;

    delivered_size = app_buffer_get(*payload);

    if( delivered_size == -1){
        return -1;
    } else{
        nb_rcv_paquets++;
    }

    if(DEBUG_MODE){
        if(delivered_size != -1){
            printf("Vous venez de recuperer %d octets de donnees dans le buffer\n",delivered_size);
            printf("[INFO] Nombre de paquets recu : %d\n",nb_rcv_paquets);
        } else
            printf("Erreur lors de la recuperation des donnees\n");
    }

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
    
    ourSocketTab[socket].socket.state = CLOSED;
    ourSocketTab[socket].socket.fd = -1;
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

    switch (ourSocketTab[id_sock].socket.state)
    {
    case ESTABLISHED:
        // On recois un pdu de donnees
        if(ourSocketTab[id_sock].num_seq_distant == pdu.header.seq_num){
            // On cree l'acquitement du pdu en question et on l'envoie
            mic_tcp_pdu pdu_ack;
            initialise_to_null_pdu(&pdu_ack);
            pdu_ack.header.ack = 1;
            pdu_ack.header.source_port = ourSocketTab[id_sock].socket.addr.port;
            pdu_ack.header.dest_port = addr.port;
            pdu_ack.header.ack_num = ourSocketTab[id_sock].num_seq_distant;
            ourSocketTab[id_sock].num_seq_distant = (ourSocketTab[id_sock].num_seq_distant + 1) % 2;

            if(DEBUG_MODE){
                print_mic_tcp_pdu_infos(pdu_ack,"PDU ACK CREATED:");
            }

            int sent_size;
            if((sent_size = IP_send(pdu_ack,addr) == -1)){
                printf("[ERROR] Echec d'envoie du PDU ACK\n");
                return;
            }

            // On traite le pdu une fois l'ack envoye
            app_buffer_put(pdu.payload);
            if(DEBUG_MODE){
                printf("PDU placé dans le buffer\n");
            }
        }
        break;
    default:
        break;
    }
    
}

/*
 * Fonction initialisant toutes les composantes d'un PDU
 * à zéro. 
 */
void initialise_to_null_pdu(mic_tcp_pdu *pdu)
{
    pdu->header.source_port = 0;
    pdu->header.dest_port = 0;
    pdu->header.seq_num = 0;
    pdu->header.ack_num = 0;
    pdu->header.syn = 0;
    pdu->header.ack = 0;
    pdu->header.fin = 0;
    pdu->payload.data = NULL;
    pdu->payload.size = 0;
}

void print_mic_tcp_pdu_infos(mic_tcp_pdu pdu, char* text) {
	printf("++++++++++++++++++++++++++++++++++++++++\n%s\n", text);
	printf("ACK flag: %d\n", pdu.header.ack);
	printf("FIN flag: %d\n", pdu.header.fin);
	printf("SYN flag: %d\n", pdu.header.syn);
	printf("Port source: %hu\n", pdu.header.source_port);
	printf("Port destination: %hu\n", pdu.header.dest_port);
	printf("Numero de seq: %u\n", pdu.header.seq_num);
	printf("Numero d'ack: %u\n", pdu.header.ack_num);
	printf("Taille du payload: %d\n", pdu.payload.size);
    printf("++++++++++++++++++++++++++++++++++++++++\n");
}
