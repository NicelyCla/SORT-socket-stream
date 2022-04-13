//Claudio Praticò mat.340404

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "list.h"



char *host_name = "127.0.0.1"; // local host 

void switch_heater(LIST sensori_da_sott, int num_sensori, int Tgoal){
    LIST tmp;
    int k = 0;
    tmp = sensori_da_sott;
    while (!isEmpty(tmp)) {
        if(tmp->item.media<Tgoal){
            k++;
        }
        tmp = tmp->next;
    }

    //printf("%d\n",num_sensori);
    if ((k/num_sensori>=0.5)){
        printf("I turn on the heaters\n");
    }
    else{
        printf("I turn off the heaters\n");
    }
    printf("Please, unsubscribe the process...\n");
}

void client( LIST list_to_send, itemType nodoLista, int Tgoal, int tipo ){

	struct sockaddr_in serv_addr;
 	struct hostent* server;	
	int i,j,num_sensori,num_att_dis;
    char buf[20] = "a";
    LIST misure_ricevute;
    misure_ricevute = NewList();
    LIST tmp;

	if ( ( server = gethostbyname(host_name) ) == 0 ) 
	{
		perror("Error resolving local host\n");
		exit(1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
	serv_addr.sin_port = htons(PORT_SERVER);
	
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

    //mi serve per dire al server che si tratta di una connessione Attuatori
    nodoLista.tipo = tipo; //fa da discrimante fra le connessioni di tipo
    if (send(sockfd,&nodoLista,sizeof(nodoLista),0) == -1) {
	    perror("Error sending message\n");
	    exit(1);
    }


    if(tipo == CLIENT_ATT_ISCRIZIONE){

        //invio della lista
        num_sensori = getLength(list_to_send);
        printf("Sending the current list of %d items...\n", num_sensori);
        if (send(sockfd,&num_sensori,sizeof(num_sensori),0) == -1) {
	        perror("Error sending message\n");
	        exit(1);
        }

        tmp = list_to_send;
        while (!isEmpty(tmp)) {
	        if (send(sockfd,&tmp->item,sizeof(tmp->item),0) == -1) {
		        perror("Error sending message\n");
		        exit(1);
	        }
	        tmp = tmp->next;
        }

        if (recv(sockfd,&num_sensori,sizeof(num_sensori),0) == -1) {
	        perror("Error receiving message\n");
	        exit(1);
        }
        //printf("%d\n",num_sensori);
        for (i = 0; i < num_sensori; i++) {
	        if (recv(sockfd,&nodoLista,sizeof(nodoLista),0) == -1) {
		        perror("Error receiving message\n");
		        exit(1);
	        }
            misure_ricevute = EnqueueLast(misure_ricevute,nodoLista);
        }

        printf("\n\nTgoal: %d\n", Tgoal);
        tmp = misure_ricevute;
        while (!isEmpty(tmp)) {
            //stampo a schermo
            tmp->item.media = 0; //la resetto inizialmente

            printf(" <- Temperatures from sensor's name %s are: ",tmp->item.nome);
            for(j = 0; j<tmp->item.n_figli;j++)
                printf("%.2f, ",tmp->item.temperatura[j]);
            printf("\n");

            if(tmp->item.n_figli >=3){
                for(j = 0; j<tmp->item.n_figli;j++)
                    tmp->item.media += tmp->item.temperatura[j];
                tmp->item.media/=tmp->item.n_figli;
                printf("    Means of measures: %.3f\n", tmp->item.media);
            }

            tmp = tmp->next;
        }

        switch_heater(misure_ricevute, num_sensori, Tgoal);

        while(1){
            bzero(buf, 20);
            if (recv(sockfd, buf, BUF_SIZE, 0) == -1) {
                perror("Error on receive");
                exit(1);
            }
            if(strcmp(buf,"")){
                break;
            }
            sleep(1);
        }
        printf("Reception finished, actuator unsubscribed successfully!\n");

        close(sockfd);
    }
    else if( tipo == CLIENT_ATT_DISISCRIZIONE ){
        //invio della lista
        num_att_dis = getLength(list_to_send);
        printf("Sending the current list of %d items...\n", num_att_dis);
        if (send(sockfd,&num_att_dis,sizeof(num_att_dis),0) == -1) {
	        perror("Error sending message\n");
	        exit(1);
        }

        tmp = list_to_send;
        while (!isEmpty(tmp)) {
	        if (send(sockfd,&tmp->item,sizeof(tmp->item),0) == -1) {
		        perror("Error sending message\n");
		        exit(1);
	        }
	        tmp = tmp->next;
        }
        close(sockfd);
    }
}

void tipoDisiscrizione( itemType nameAtt, int tipo ){

    char name_att[30] = "a";
	LIST att_da_dscr;
    LIST tmp;
	att_da_dscr = NewList();

    
    //ciclo ricezione della lista utente
    printf("Enter 0 to stop...\n");
    while (strcmp(name_att,"0") != 0){
        printf("Actuator name to unsubscribe: ");

        scanf("%s", name_att);

        //controllo che non ci sia già con lo stesso nome
        tmp = att_da_dscr;
        while ( !isEmpty(tmp) ) 
        {
            if (!strcmp(name_att, tmp->item.nome_attuatore)){
                printf("Name already queued to be unsubscribed!\n");
                break;
            }
            tmp = tmp -> next;
        }

        //serve a non mettere lo zero né i sensori ripetuti
        if(strcmp(name_att,"0") && isEmpty(tmp)){ 
            strcpy( nameAtt.nome_attuatore, name_att );
            nameAtt.tipo = CLIENT_ATT_ISCRIZIONE;
		    att_da_dscr = EnqueueLast(att_da_dscr, nameAtt);
        }
    }

    printf("The actuators to be unsubscribed will be: ");
    tmp = att_da_dscr;
    while ( !isEmpty(tmp) ) 
    {
        printf("%s, ", tmp->item.nome_attuatore);
        tmp = tmp -> next;
    }
   
    client(	att_da_dscr, nameAtt, 0, tipo );

}

void tipoIscrizione( itemType nameSensor, int tipo ){
    int Tgoal;
    char name_sensor[30] = "a";
	LIST sensori_da_sott;
    LIST tmp;
	sensori_da_sott = NewList();

    //ricezione della Tgoal
    printf("Tgoal: ");
    scanf("%d", &Tgoal);
    
    //ciclo ricezione della lista utente
    printf("Enter 0 to stop...\n");
    while (strcmp(name_sensor,"0") != 0){
        printf("Sensor name to subscribe: ");

        scanf("%s", name_sensor);

        //controllo che non ci sia già con lo stesso nome
        tmp = sensori_da_sott;
        while ( !isEmpty(tmp) ) 
        {
            if (!strcmp(name_sensor, tmp->item.nome)){
                printf("Name already queued to be subscribed!\n");
                break;
            }
            tmp = tmp -> next;
        }

        //serve a non mettere lo zero né i sensori ripetuti
        if(strcmp(name_sensor,"0") && isEmpty(tmp)){ 
            strcpy( nameSensor.nome, name_sensor );
		    sensori_da_sott = EnqueueLast(sensori_da_sott, nameSensor);
        }
    }

    printf("The sensors to be subscribed will be: ");
    tmp = sensori_da_sott;
    while ( !isEmpty(tmp) ) 
    {
        printf("%s, ", tmp->item.nome);
        tmp = tmp -> next;
    }
   
    printf("Listening...\n");
    client(	sensori_da_sott, nameSensor, Tgoal, tipo );

}

int main(int argc, char *argv[]) 
{
	char nome[30]; //nome attuatore
	itemType nodoLista;
    char tipo;

	if (argc < 3){
		printf("Usage: attuatori \"name\" \"i/d\"(iscrizione/disiscrizione)\n");
        exit(1);
    }
	else{
		strcpy(nome, argv[1] );
        tipo = argv[2][0];
        if(tipo == 'i')
            strcpy(nodoLista.nome_attuatore,nome);
        else{
            strcpy(nodoLista.nome,nome); //in questo caso, e solo in questo caso, nome si riferisce al nome dell'attuatore
        }

    }

    if(tipo == 'i'){
        tipoIscrizione(nodoLista, CLIENT_ATT_ISCRIZIONE);
    }
    else if(tipo == 'd'){
        tipoDisiscrizione(nodoLista, CLIENT_ATT_DISISCRIZIONE);
    }
    else{
        printf("Scegliere correttamente la modalità i(iscrizione) oppure d(disiscrizione)\n");
        exit(0);
    }


	return 0;
}



