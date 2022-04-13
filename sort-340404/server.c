//Claudio Praticò mat.340404

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "list.h"



int mandaMisure( LIST misure_sensori, LIST attuatore_lista, itemType attuatore){

    LIST tmp;
    LIST attuatore_lista_child;
    attuatore_lista_child = NewList();
    itemType *nodo;
    int skfd = attuatore.skfd;

    int num_sensori = getLength(misure_sensori);
    int i;

    tmp = attuatore_lista;
    while ( !isEmpty(tmp) ){
        nodo = Find(misure_sensori,tmp->item);//controllo di ricevere sensori che esistono all'interno del server
        if(!strcmp(tmp->item.nome_attuatore,attuatore.nome_attuatore) && (nodo != NULL)){
            attuatore_lista_child = EnqueueLast(attuatore_lista_child, tmp->item);
            //printf( "nomi sensori: %s da attuatore %s\n", tmp->item.nome, tmp->item.nome_attuatore );
        }
        tmp = tmp->next;
    }

    //invio della lista
    num_sensori = getLength(attuatore_lista_child);

    tmp = attuatore_lista_child;
    while (!isEmpty(tmp)) {
        nodo = Find(misure_sensori,tmp->item); //non c'è bisogno di verificare che il nodo non sia nullo

        for(i = 0; i<nodo->n_figli;i++){
            tmp->item.n_att = nodo->n_att;
            tmp->item.temperatura[i] = nodo->temperatura[i];
            tmp->item.n_figli = nodo->n_figli;
            tmp->item.num_sensori = num_sensori; //devo aggiornare il numero dei sensori nell'attuatore
        }
        tmp = tmp->next;
    }

    //invio della lista
    printf("Sending the current list of %d items...\n", num_sensori);
    if (send(skfd,&num_sensori,sizeof(num_sensori),0) == -1) {
        perror("Error sending message\n");
        exit(1);
    }

    tmp = attuatore_lista_child;
    while (!isEmpty(tmp)) {
        if (send(skfd,&tmp->item,sizeof(tmp->item),0) == -1) {
            perror("Error sending message\n");
            exit(1);
        }
        tmp = tmp->next;
    }
    close(skfd);
    exit(0);
}

int main() 
{
	itemType msg;

    itemType *nodo;
	LIST misure_sensori;
    LIST attuatore_lista;
    LIST attuatori_figli;

	misure_sensori = NewList();
    attuatore_lista = NewList();
    attuatori_figli = NewList();
    
    LIST tmp, misura;

    int num_sensori; //numero sensori ricevuti dall'attuatore (valore successivamente variabile)
    int num_att_dis; //numero attuatori da disiscrivere
    int i,j;
    j=1;


	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	// Socket opening
	int sockfd = socket( PF_INET, SOCK_STREAM, 0 );  
	if ( sockfd == -1 ) 
	{
		perror("Error opening socket");
		exit(1);
	}
	
	int options = 1;
	if(setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof (options)) < 0) {
		perror("Error on setsockopt");
		exit(1);
	}

	bzero( &serv_addr, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_SERVER);

	// Address bindind to socket
	if ( bind( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) == -1 ) 
	{
		perror("Error on binding");
		exit(1);
	}
	
	// Maximum number of connection kept in the socket queue
	if ( listen( sockfd, 20 ) == -1 ) 
	{
		perror("Error on listen");
		exit(1);
	}


	socklen_t address_size = sizeof( cli_addr );	
	

	while(1) 
	{
        
		printf("\nWaiting for a new connection...\n");
		
		// New connection acceptance		
		int newsockfd = accept( sockfd, (struct sockaddr *)&cli_addr, &address_size );      
		if (newsockfd == -1) 
		{
			perror("Error on accept");
			exit(1);
		}
		
		// Message reception
	    if ( recv( newsockfd, &msg, sizeof(msg), 0 ) == -1) 
		{
			perror("Error on receive");
			exit(1);
		}

        if (msg.tipo == CLIENT_HUB){
            printf("Received from HUB the measurements of the sensor %s!\n",msg.nome);
            msg.skfd = newsockfd;

            nodo = Find(misure_sensori,msg);
            if(nodo == NULL){
               msg.n_att = 0;
               misure_sensori = EnqueueLast(misure_sensori, msg);
            }
            
            else{
                for(i=0;i<nodo->n_figli;i++){
                    nodo->temperatura[i] = msg.temperatura[i];
                    nodo->skfd = newsockfd;
                }
            }

            //aspetto di ricevere tutte le misure, per questo è necessario il parametro J nel mio caso
            if(j==msg.J){

                //conto quanti attuatori sottoscriveranno le misure
                tmp = attuatore_lista;
                while ( !isEmpty(tmp) ){
                    misura = misure_sensori;
                    while ( !isEmpty(misura) ){
                        if(strcmp(misura->item.nome,tmp->item.nome)==0){
                            misura->item.n_att++;

                        }

                        misura = misura->next;
                    }
                    tmp = tmp -> next;
                }
                
                //Per ogni attuatore, mando le misure tramite un figlio, la lista
                tmp = attuatori_figli;
                while ( !isEmpty(tmp) ) 
                {

                    nodo = Find(misure_sensori,tmp->item);//controllo di ricevere sensori che esistono all'interno del server
                    if (nodo != 0){
                        //printf("nome attuatore e nome misura: %s e %s\n", tmp->item.nome_attuatore, tmp->item.nome);
                        if(fork() == 0)
                        {
                            mandaMisure(misure_sensori, attuatore_lista, tmp->item );
                        }

                    }
                    
                    tmp = tmp -> next;
                }
                
                //Comunico all'hub che i sensori sono stati letti
                tmp = misure_sensori;
                while (!isEmpty(tmp)) {

                    if (send(tmp->item.skfd,&tmp->item,sizeof(tmp->item),0) == -1) {
                        perror("Error sending message\n");
                        exit(1);
                    }
                    close(tmp->item.skfd);
                    tmp = tmp->next;
                }

                //Una volta mandate, resetto la lista delle misure
	            misure_sensori = DeleteList(misure_sensori);
            
                PrintList(misure_sensori);
                j=1;
            }
            j++; //necessario
            
        }
        else if(msg.tipo == CLIENT_ATT_ISCRIZIONE){
		    printf("Actuator[subscribe]'s name [%s]:\n",msg.nome_attuatore);

            //ricezione della lista
	        if (recv(newsockfd,&num_sensori,sizeof(num_sensori),0) == -1) {
		        perror("Error receiving message\n");
		        exit(1);
	        }
	        for (i = 0; i < num_sensori; i++) {
		        if (recv(newsockfd,&msg,sizeof(msg),0) == -1) {
			        perror("Error receiving message\n");
			        exit(1);
		        }
                msg.skfd = newsockfd;
                msg.n_figli = 0;

                attuatore_lista = EnqueueLast(attuatore_lista, msg);
                printf(" -> Actuator requires subscription to: %s\n", msg.nome);
	        }
            
            tmp = attuatore_lista;
            while ( !isEmpty(tmp) ){
                nodo = Find_att(attuatori_figli, tmp->item);
                if(nodo==NULL){
                    attuatori_figli = EnqueueLast(attuatori_figli, tmp->item);
                }
                tmp = tmp->next;
            }
            
            
        }

        else if(msg.tipo == CLIENT_ATT_DISISCRIZIONE){
            //in questo caso, e solo in questo caso, msg.nome si riferisce al nome dell'attuatore

		    printf("Actuator[unsubscribe]'s name [%s]:\n",msg.nome); 

            //ricezione della lista
	        if (recv(newsockfd,&num_att_dis,sizeof(num_att_dis),0) == -1) {
		        perror("Error receiving message\n");
		        exit(1);
	        }
	        for (i = 0; i < num_att_dis; i++) {
		        if (recv(newsockfd,&msg,sizeof(msg),0) == -1) {
			        perror("Error receiving message\n");
			        exit(1);
		        }

                printf(" -> Actuator requires to unsubscribe: %s\n", msg.nome_attuatore);

                tmp = attuatori_figli;
                while ( !isEmpty(tmp) ){
                    if(!strcmp(tmp->item.nome_attuatore, msg.nome_attuatore)){

                        //Mando la stringa vuota per interrompere la ricezione e chiudere l'attuatore in questione
                        if (send(tmp->item.skfd, "0", 1, 0) == -1) {
                            perror("Error on send\n");
                            exit(1);
                        }
                        
                        close(tmp->item.skfd);
                    }
                    tmp = tmp->next;
                }

                tmp = attuatore_lista;
                while ( !isEmpty(tmp) ){
                    attuatore_lista =  Dequeue_att( attuatore_lista, msg ); //toglie l'elemento dalla lista in base all'attuatore disiscritto
                    
                    tmp = tmp->next;
                }

                attuatori_figli =  Dequeue_att( attuatori_figli, msg ); //toglie l'elemento dalla lista in base all'attuatore disiscritto, in questo caso non serve il ciclo
                

            }
            
        }

        else{
            printf("type: %d\n", msg.tipo);
		    printf("Unknown connection! %s\n", msg.nome);
        }

	}

	close(sockfd);
	return 0;
}





