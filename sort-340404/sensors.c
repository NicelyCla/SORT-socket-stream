//Claudio Praticò mat.340404

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "list.h"


char *host_name = "127.0.0.1"; // local host 


int main(int argc, char *argv[]) 
{
    int i, pid;
    tipoSensore sensore;
    int dado_wait;
    int N_t;
    int num_att = 0; //numero di attuatori che hanno letto la misura
    pid_t wpid;
    int status = 0;
	
	if (argc < 3) {
		printf("Usage: sensor NameSensor nMisure.\n");
        exit(1);
    }
	else{
		strcpy(sensore.nome_sensore, argv[1] );
        N_t=atoi(argv[2]);
    }
	
    //Creo N_t figli
    for(i=0;i<N_t;i++) // loop will run n times (n=5)
    {
        pid = fork();
        if(pid == 0)
        {
		    //printf("Sono il figlio %i: pid = %d\n",i, getpid());
            sensore.pid[i] = getpid();
            sensore.i=i;
            sensore.N_t=N_t;

            srand(getpid());
            int max = 35;
            int min = -5;
            float range = (max - min); 
            float div = RAND_MAX / range;
            sensore.temperatura = min + (rand() / div); //rand()%((nMax+1)-nMin) + nMin;

            //Ciascun processo figlio si connette all'hub
            //-------------------------------------------------//
	        struct sockaddr_in serv_addr;
         	struct hostent* server;	

	        if ( ( server = gethostbyname(host_name) ) == 0 ) 
	        {
		        perror("Error resolving local host\n");
		        exit(1);
	        }

	        bzero(&serv_addr, sizeof(serv_addr));
	        serv_addr.sin_family = AF_INET;
	        serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
	        serv_addr.sin_port = htons(PORT_HUB);
	        
	        int sockfd = socket( PF_INET, SOCK_STREAM, 0 );

	        if ( sockfd == -1 ) 
	        {
		        perror("Error opening socket\n");
		        exit(1);
	        }    

	        if ( connect(sockfd, (void*)&serv_addr, sizeof(serv_addr) ) == -1 ) 
	        {
		        perror("Error connecting to socket\n");
		        exit(1);
	        }

            //il processo figlio attende "dado_wait" secondi
            dado_wait = rand()%((6+1)-0) + 0;
            sleep(dado_wait);            

	        printf("Sending temperatures \"%.2f\" to hub... from: %s\n", sensore.temperatura, sensore.nome_sensore);
	        // This sends the sensor
	        if ( send(sockfd, &sensore, sizeof(sensore), 0) == -1 ) 
	        {
		        perror("Error on send name sensor\n");
		        exit(1);
	        }
            

	        printf("Temperature sent. Waiting numbers of actuators that have read the measurement...\n\n");
	        
	        if ( recv(sockfd, &num_att, sizeof(num_att), 0) == -1 ) 
	        {
		        perror("Error in receiving response from hub\n");
		        exit(1);
	        }

	        printf("Number of actuators who entered the measures: \"%d\"\n\n", num_att);

	        close(sockfd);
            //-------------------------------------------------//

			exit(0);

        }

    }
    while((wpid = wait(&status)) >0);
    //printf("non sono mai uscito");
	return 0;
}



