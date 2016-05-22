#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#define NUM_THREADS 4
#define RW_OWNER_GROUP 0660
#define SIZE_NAME 15
#define SIZE_NUM 5
#define STR_FULL "cheio!"
#define STR_ENTRY "entrada"
#define TO_MICROSEC *1000000
#define BUF_SIZE 100
#define SHARED 0

typedef struct arrumadorArgs
{
	int parkingTime;
	int idVeiculo;
	pthread_mutex_t fifoAccess;
	pthread_cond_t readyToRead;
	
}arrumadorArgs;


void * controlador(void * pontoCardeal);
void * arrumador(void * infoVeiculo);
char * makeNameViatura(int idVeiculo, char * filenameViatura);
void readAllFromFifo(int filedesc);
int reserveParkingSpot();
int * createInfoVeiculo(int parkingTime, int idVeiculo);
int getOccupiedParkingSpots();
int freeParkingSpot();

//Variáveis globais que guardam os lugares do parque
int totalParkingSpots;
int availableParkingSpots;
int seconds;
int closed;
sem_t entrySemaphore;

//Array que guarda os threads id dos arrumadores
pthread_t * assistants;


int main(int argc, char * argv[])
{
	//TODO Verificacoes argumentos
	
	//argv[0] --> numero Lugares do parque
	//argv[1] --> periodo de abertura, em segundos
	
	//Atribuir ao numero de lugaresTotais/Disponiveis aquilo que for passado como argumento
	totalParkingSpots = *argv[0];
	availableParkingSpots = *argv[0];
	closed = 1;
	sem_init(&entrySemaphore, SHARED, 1);
	assistants = malloc(sizeof(pthread_t)*totalParkingSpots);
	
	
	pthread_t ctrlN, ctrlS, ctrlE, ctrlO; //identificadores dos controladores de cada entrada do parque
	
	char threadArg[NUM_THREADS]; //estrutura que contem os argumentos dos threads
	threadArg[0] = 'N';
	threadArg[1] = 'S';
	threadArg[2] = 'E';
	threadArg[3] = 'O';
	
	
	//Cria os threads para cada entrada do parque
	pthread_create(&ctrlN, NULL, controlador, &threadArg[0]);
	pthread_create(&ctrlS, NULL, controlador, &threadArg[1]);
	pthread_create(&ctrlE, NULL, controlador, &threadArg[2]);
	pthread_create(&ctrlO, NULL, controlador, &threadArg[3]);
	
	//Fica a dormir até à hora de fecho
	sleep(*argv[1]);
	
	//Fecha o parque
	closed = 1;
	
	//TODO Fechar as entradas dos controladores
		
	//Variavel de retorno dos thread_join
	void * ret;
	
	//Funções que esperam que cada um dos 4 controladores acabe o seu trabalho
	pthread_join(ctrlN, &ret);
	pthread_join(ctrlS, &ret);
	pthread_join(ctrlE, &ret);
	pthread_join(ctrlO, &ret);
	
	
	return 0;

}

/* O valor de retorno pode nao ser este */
/* char c --> Norte/Sul/Este/Oeste, diz qual é a entrada
 * 
 */
void * controlador(void * direction)
{
	int fifofd;
	//FILE * fifoPointer;
	//TODO Contador de entradas? int?
	
	//Escreve o nome do fifo com o ponto cardeal
	char fifoname[6] = "fifo";
	fifoname[4] = *(char *)direction;
	fifoname[5] = '\0';
	
	//Criar e abrir fifo
	mkfifo(fifoname, RW_OWNER_GROUP); //Cria
	fifofd = open(fifoname, O_RDONLY); //Abre como leitura
	//fifoPointer =fdopen(fifofd, "r"); //Cria o file descriptor
	
	//TODO Processamento das entradas/saidas do parque
//NOTAS: POSSIVEL IMPLEMENTACAO
	//CONCEITO: O controlador começa com a "bola". Sempre que ele não tiver a bola, e a apanhar, vai ler tudo
	//o que conseguir do fifo. Quando nao conseguir ler mais, lança a bola ao ar, e fica à espera que alguém
	//a atire novamente ao ar.


	//Criar Mutex e Cond. Variable
	pthread_mutex_t fifoAccess = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t readyToRead = PTHREAD_COND_INITIALIZER;

	// - while(read != 0)
	//quando nao conseguir ler
	
	char * buffer = malloc(sizeof(char) * BUF_SIZE);
	size_t bufferSize = sizeof(char) * BUF_SIZE;
	int idVeiculo, parkingTime;
	arrumadorArgs args;
	
	while(closed == 0) //Enquanto o parque não estiver fechado, vai processar sempre os carros que tentam entrar
	{
		if(getline(&buffer,&bufferSize,fdopen(fifofd, "r"))!= -1) //Le uma linha do fifo para o buffer
		{
		
		 sscanf(buffer,"%d %d", &idVeiculo, &parkingTime); //Retira a informação do buffer 
		
		//Criar os argumentos a passar à thread arrumador
		args.idVeiculo= idVeiculo;
		args.parkingTime = parkingTime;
		args.fifoAccess= fifoAccess;
		args.readyToRead = readyToRead;
		
		//Chama arrumador para encaminhar o veiculo
		pthread_create(&assistants[getOccupiedParkingSpots()], NULL, arrumador, (void *) &args);
		pthread_detach(assistants[getOccupiedParkingSpots()]);
		
		}
		else //Se o getline falhar (por causa de end of file)
		{
			pthread_cond_wait(&readyToRead,&fifoAccess);
			//TODO Verificar o caso de serem criadas viaturas perto da hora de fecho do parque, 
			//que sucessivamente fazem lock do mutex impedindo que o controlador leia as suas informaçoes antes de o parque fechar 
		}
		
	}
	
	
	//Quando não permite mais entradas no parque
	//Ler as infos restantes das entradas 
	readAllFromFifo(fifofd);
	
	//Fechar e destruir fifo
	close(fifofd); //Fecha
	unlink(fifoname); //Destroi
	
	//TODO?? So acaba o trabalho quando todos as entradas que registou se converterem em saidas
	
	return 0;
}

/*
 * Devemos ter de passar as informaçoes:
 * - Tempo de estacionamento
 * - Identificador da viatura
 */
void * arrumador(void * infoVeiculo)
{
//Abrir FIFO privado da viatura (verificar o nome do fifo da viatura!!!!)
	char * fifoname = malloc(sizeof(char) * SIZE_NAME);
	arrumadorArgs tempArgs = (*(arrumadorArgs *)infoVeiculo); //Para facilitar o acesso aos argumentos
	
	makeNameViatura(tempArgs.idVeiculo, fifoname);
	
	int fd = open(fifoname, O_RDONLY); 
	double mSec =0; //tempo que vai estar estacionado
		
//Verificar se há lugar no parque ou não
	sem_wait(&entrySemaphore); //Espera que o availableparkingspots seja atualizado
	
	if(availableParkingSpots == 0)
	{
		//Se não houver, diz que está "cheio!" no FIFO privado da viatura, (que é lido pela mesma)
		write(fd, STR_FULL, sizeof(STR_FULL));
	}
	else if(availableParkingSpots > 0)
	{	
		//Se houver, reserva a vaga, e envia "entrada" para o FIFO privado.
		reserveParkingSpot();
		write(fd, STR_ENTRY, sizeof(STR_ENTRY));
		
		//Ligar um temporizador, e quando terminar, acompanhar a viatura á saida, enviando "saida" para o FIFO.
		
		mSec = tempArgs.parkingTime/CLOCKS_PER_SEC;
		mSec = mSec TO_MICROSEC;
		
	}
	
	sem_post(&entrySemaphore); //Coloca o semaforo a 1, de modo a informar que o availableParkingSpots está atualizado
	
//Estacionameto do carro	
	if (mSec!=0)
	{
		usleep(mSec); //Tempo estacionado
		sem_wait(&entrySemaphore);//Espera que o availableparkingspots seja atualizado
		freeParkingSpot();//Liberta um lugar ocupado
		sem_post(&entrySemaphore);//Coloca o semaforo a 1, de modo a informar que o availableParkingSpots está atualizado
	}
	
	return 0;
}

/*
 * As funções exteriores é que são responsáveis por libertar a memória do filenameViatura
 */
char * makeNameViatura(int idVeiculo, char * filenameViatura)
{
	char * numViatura = malloc(sizeof(char) * SIZE_NUM);
	
	strcpy(filenameViatura, "viatura");
	sprintf(numViatura, "%d", idVeiculo);
	
	strcat(filenameViatura, numViatura);
	
	free(numViatura);
	
	return filenameViatura;
}

void readAllFromFifo(int filedesc)
{
	void * mybuffer = malloc(sizeof(char) * 100); //Buffer temporário onde se guarda as informações lidas
	
	while(1)
	{
		if(read(filedesc, mybuffer, sizeof(mybuffer)) == 0) //read retorna 0 quando chega ao End Of File
			break;
	}
	
	free(mybuffer);
}

int reserveParkingSpot()
{
	if(availableParkingSpots > 0) 
	{
		availableParkingSpots--;
		return availableParkingSpots;
	}
	else
	{
		return -1;
	}
}

int getOccupiedParkingSpots()
{
	return totalParkingSpots - availableParkingSpots;
}

int freeParkingSpot()
{
	if (availableParkingSpots + 1 > totalParkingSpots)
	{
		return -1;//Caso exceda o número de lugares totais
	}
	
	return availableParkingSpots++;
}
/*
 * Cria o apontador necessário para passar como argumento à thread "arrumador"
 * As funções exteriores é que são responsáveis por libertar a memória do infoVeiculo
 */

