#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include <errno.h>
#include <stdlib.h>

struct product {
	int productID;
	time_t timeGenerated;
	int life;
	struct product* next;
};

struct product* head = NULL;
int remaining_products = 0;

pthread_mutex_t access_queue;

int main(int argc, char const *argv[]){
	if (argc < 8 || argc > 8){
		printf("Incorrect number of arguments!");
		return 2;
	}
	int prodThreads = strtol(argv[1], NULL, 10);
	int consumThreads = strtol(argv[2], NULL, 10);
	remaining_products = strtol(argv[3], NULL, 10);
	int queueSize = *argv[4];
	int algorithm = *argv[5]; // 0 = FCFS and 1 = RR
	int quantum = *argv[6];
	int seed = *argv[7];

	pthread_mutex_init(&access_queue, NULL);

	pthread_t prod_thread[prodThreads];
	pthread_t consum_thread[consumThreads];
	int prodn[prodThreads];
	int consumn[consumThreads];

	void* producer();
	void* consumer();

	for (int i = 0; i < prodThreads; i++){
		prodn[i] = i;
		pthread_create(&prod_thread[i], NULL, producer, &prodn[i]);
	}

	for (int i = 0; i < consumThreads; i++){
		consumn[i] = i;
		pthread_create(&consum_thread[i], NULL, consumer, &consumn[i]);
	}


	for (int i = 0; i < prodThreads; i++)
		pthread_join(prod_thread[i], NULL);

	for (int i = 0; i < consumThreads; i++)
		pthread_join(consum_thread[i], NULL);

	pthread_exit(0);
}

void* producer(int* i){
	while (1){
		//Obtain lock on the queue
		pthread_mutex_lock(&access_queue);
		if (!remaining_products){
			pthread_mutex_unlock(&access_queue);
 			pthread_exit(NULL);
		}

		//Create the new product, add it to the queue, and decrement the count of remaining products to produce
		struct product* new_product = malloc(sizeof(struct product));
		new_product->productID = remaining_products;
		new_product->next = NULL;

		if (head == NULL)
			head = new_product;
		else {
			struct product* last = head;
			while (last->next != NULL)
				last = last->next;
			last->next = new_product;
		}

		printf("Produced Product #%d\n", remaining_products);
		printf("Queue: ");
		struct product* lasttwo = head;
		while (lasttwo->next != NULL){
			printf("%d, ", lasttwo->productID);
			lasttwo = lasttwo->next;
		}
		printf("%d\n", lasttwo->productID);

		remaining_products--;

		// Release the lock on the queue
		pthread_mutex_unlock(&access_queue);

		// Sleep for 100 milliseconds
		usleep(100000);
	}
}

void* consumer(int* i){
	/*while(head == NULL){}
	while (*head != NULL || remaining_products){
		// Obtain lock on the queue
		pthread_mutex_lock(&access_queue);

		printf("Queue: ");
		struct product* last = *head;
		while (last->next != NULL){
			printf("%d, ", last->productID);
			last = last->next;
		}
		printf("%d\n", last->productID);

		// Pull first item from the queue
		struct product* current_product = *head;
		*head = current_product->next;

		printf("Consumed Product ID: %d\n", current_product->productID);
		free(current_product);

		// Release the lock on the queue
		pthread_mutex_unlock(&access_queue);

		// Sleep for 100 milliseconds
		usleep(100000);
	}*/
 	pthread_exit(NULL);
}