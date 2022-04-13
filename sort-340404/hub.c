//Claudio Praticò mat.340404

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include "list.h"



int client(	itemType misurazione, int J){
    //Ciascun processo figlio si connette al server
    //-------------------------------------------------//
    char *host_name = "127.0.0.1"; // local host
    struct sockaddr_in serv_addr;
 	struct hostent* server;
    int n_att;
    
    if ( ( server = gethostbyname(host_name) ) == 0 ) 
    {
        perror("Error resolving local host\n");
        exit(1);
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
    serv_addr.sin_port = htons(PORT_SERVER);
    
    int sockfd_child = socket( PF_INET, SOCK_STREAM, 0 );

    if ( sockfd_child == -1 ) 
    {
        perror("Error opening socket\n");
        exit(1);
    }    

    if ( connect(sockfd_child, (void*)&serv_addr, sizeof(serv_addr) ) == -1 ) 
    {
        perror("Error connecting to socket\n");
        exit(1);
    }
    
    misurazione.tipo = CLIENT_HUB;
    misurazione.n_att = 0;
    misurazione.skfd = sockfd_child;
    misurazione.J = J;
    //printf("skfd %d \n", misurazione.skfd);
    
    printf("Sending temperatures to server... from: %s\n", misurazione.nome);
    // This sends the sensor
    if ( send(sockfd_child, &misurazione, sizeof(misurazione), 0) == -1 ) 
    {
        perror("Error on send name sensor\n");
        exit(1);
    }

    
    printf("Temperature sent, waiting response...\n\n");


    if ( recv(sockfd_child, &misurazione, sizeof(misurazione), 0) == -1 ) 
    {
        perror("Error in receiving response from server\n");
        exit(1);
    }
    n_att = misurazione.n_att;
    printf("Measures from sensor %s entered by %d actuators!\n",misurazione.nome ,misurazione.n_att);

    close(sockfd_child);
    //-------------------------------------------------//

    return n_att;

}

int main(int argc, char *argv[]) 
{
    tipoSensore sensore;
    int count_sensors_childreen = 0;
    int count_sensors_process = 0;
    int i = 0;
    int j = 0;
    int J = 2;
    int newsockfd;
    int num_att = 0; //numero di attuatori che hanno letto la misura
	int options = 1;

	LIST misure_sensori;
    LIST tmp;
	misure_sensori = NewList();
	itemType misurazione;

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	if (argc < 2) {
		printf("Usage: hub J, verrà scelto di default J = 2\n");
    }
	else{
        J = atoi(argv[1]);
    }

	// Socket opening
	int sockfd = socket( PF_INET, SOCK_STREAM, 0 );  
	if ( sockfd == -1 ) 
	{
		perror("Error opening socket");
		exit(1);
	}
	

	if(setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof (options)) < 0) {
		perror("Error on setsockopt");
		exit(1);
	}

	bzero( &serv_addr, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_HUB);

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

		printf("Waiting for a new connection...\n\n");


		// New connection acceptance
		newsockfd = accept( sockfd, (struct sockaddr *)&cli_addr, &address_size );
        //newsockfd_array[i] = newsockfd;

	    if (newsockfd == -1) 
	    {
		    perror("Error on accept");
		    exit(1);
	    }
	    count_sensors_childreen++;

	    // Message reception
	    if ( recv( newsockfd, &sensore, sizeof(sensore), 0 ) == -1) 
	    {
            perror("Error on receive");
            exit(1);
	    }
	        
        printf("Received from sensor's name: \"%s\", child-pid: \"%d\", temperature: \"%.2f\"\n",sensore.nome_sensore, sensore.pid[sensore.i], sensore.temperatura);
		
        strcpy( misurazione.nome, sensore.nome_sensore );
        misurazione.temperatura[i] = sensore.temperatura;
        misurazione.n_figli = sensore.N_t;
        misurazione.sockfd[i] = newsockfd;
        misurazione.n_att = 0;

        i++;

        if(i==sensore.N_t){
		    misure_sensori = EnqueueLast(misure_sensori, misurazione); //in questo ordine verranno richiamati con una politica FIFO
            count_sensors_process++;
            printf("Currently connected (total connections): %d\n",count_sensors_childreen); //numero di connessioni != numero di "processi" attivi
            printf("Currently connected (Total processes): %d\n",count_sensors_process);
            i = 0;
        }


        if(count_sensors_process == J){
            PrintList(misure_sensori);
            printf("Creating %d childreen...\n", J);

            tmp = misure_sensori;
            while ( !isEmpty(tmp) ) 
            {
                if(fork() == 0)
                {
                    num_att = client(tmp->item, J);
                    for(j = 0; j<tmp->item.n_figli; j++){
                        if ( send(tmp->item.sockfd[j], &num_att, sizeof(num_att), 0) == -1 ) 
                        {
                            perror("Error on send name sensor\n");
                            exit(1);
                        }
                    }
                    close(tmp->item.sockfd[j]);
                    exit(0);
                }

                tmp = tmp -> next;

            }

            //resetto tutti i valori:
            tmp = misure_sensori;
	        misure_sensori = DeleteList(misure_sensori);
            count_sensors_process = 0;
            count_sensors_childreen = 0;
            i=0;

        }
	
	}

	close(sockfd);
	return 0;
}



