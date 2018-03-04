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

// Head of the queue
struct product* head = NULL;

// Remaining products to produce
int remaining_products = 0;
// Don't need to track products to consume, if the queue is empty and there are no
// more products to be produced then all products must have been consumed.

// Maximum length for the queue
int maxQueue = 0;

// Algorithm to use, 0 for FCFS, 1 for Round Robin
int algorithm = 0;

// Quantum time for the Round Robin algorithm
int quantum = 0;

pthread_mutex_t access_queue;

int fibonacci(int n){
	if (n == 0)
		return 0;
	else if (n == 1)
		return 1;
	else {
		return n + fibonacci(n-1);
	}
}

int queueLength(){
	int length = 0;
	struct product* current = head;
	while (current != NULL && current->next != NULL){
		current = current->next;
		length++;
	}
	return length;
}

int main(int argc, char const *argv[]){

	// Check for the proper number of arguments
	if (argc < 8 || argc > 8){
		printf("Incorrect number of arguments!");
		return 2;
	}

	// Assign the arguments
	int prodThreads = strtol(argv[1], NULL, 10);
	int consumThreads = strtol(argv[2], NULL, 10);
	remaining_products = strtol(argv[3], NULL, 10);
	maxQueue = strtol(argv[4], NULL, 10);
	algorithm = strtol(argv[5], NULL, 10); // 0 = FCFS and 1 = RR
	quantum = strtol(argv[6], NULL, 10);
	unsigned int seed = strtol(argv[7], NULL, 10);

	if (algorithm)
		printf("-----ROUND ROBIN-----\n");
	else
		printf("-----FCFS-----\n");

	// Initialize the random number generator with the seed
	srand(seed);

	// Initialize the queue lock
	pthread_mutex_init(&access_queue, NULL);

	// Create threads for the consumers and producers
	pthread_t prod_thread[prodThreads];
	pthread_t consum_thread[consumThreads];
	int prodn[prodThreads];
	int consumn[consumThreads];

	void* producer();
	void* consumer();

	// Initialize the threads
	for (int i = 0; i < prodThreads; i++){
		prodn[i] = i;
		pthread_create(&prod_thread[i], NULL, producer, &prodn[i]);
	}
	for (int i = 0; i < consumThreads; i++){
		consumn[i] = i;
		pthread_create(&consum_thread[i], NULL, consumer, &consumn[i]);
	}

	// Start the threads
	for (int i = 0; i < prodThreads; i++)
		pthread_join(prod_thread[i], NULL);
	for (int i = 0; i < consumThreads; i++)
		pthread_join(consum_thread[i], NULL);

	// Wait for all threads the exit then quit.
	pthread_exit(0);
}

void* producer(int* i){
	while (1){
		// Obtain lock on the queue
		pthread_mutex_lock(&access_queue);

		// Make sure there are more products
		if (remaining_products <= 0){
			// No more products to produce, release the queue and exit thread
			pthread_mutex_unlock(&access_queue);
 			pthread_exit(NULL);
		} else if (maxQueue == 0 || queueLength() < maxQueue){
			// Create the new product
			struct product* new_product = malloc(sizeof(struct product));
			new_product->productID = remaining_products;
			new_product->next = NULL;
			new_product->life = rand() % 1024;
			new_product->timeGenerated = time(NULL);

			// Add new product to the queue, create queue if needed
			if (head == NULL)
				head = new_product;
			else {
				struct product* last = head;
				while (last->next != NULL)
					last = last->next;
				last->next = new_product;
			}

			// Decrement the count of remaining products to produce
			remaining_products--;

			printf("Producer #%d produced product #%d with queue length: %d\n", *i, new_product->productID, queueLength());
		}

		// Release the lock on the queue
		pthread_mutex_unlock(&access_queue);

		// Sleep for 100 milliseconds
		usleep(100000);
	}
}

void* consumer(int* i){
	while (1){
		// Obtain lock on the queue
		pthread_mutex_lock(&access_queue);

		// Make sure the queue is not empty
		if (head == NULL){
			// If the queue is empty release the lock
			pthread_mutex_unlock(&access_queue);

			// If there are also no remaining products to create then exit
			if (remaining_products <= 0)
				pthread_exit(NULL);
		} else {
			// Pull first item from the queue
			struct product* current_product = head;
			head = current_product->next;

			if (algorithm == 0){
				// Consume the product
				for (int i = 0; i < current_product->life; i++)
					fibonacci(10);

				printf("Consumer #%d consumed product #%d with life #%d\n", *i, current_product->productID, current_product->life);

				// Free the pulled node
				free(current_product);
			} else {
				// Consume the product the quantum amount
				if (current_product->life - quantum < 0){
					for (int i = 0; i < current_product->life; i++)
						fibonacci(10);

					// Decrement the product's life counter
					current_product->life = 0;
				} else {
					for (int i = 0; i < quantum; i++)
						fibonacci(10);

					// Decrement the product's life counter
					current_product->life -= quantum;
				}

				// If the product's life is over free it, otherwise add it back to the end of the queue
				if (current_product->life <= 0){
					printf("Consumer #%d consumed product #%d with life #%d --DESTROYED--\n", *i, current_product->productID, current_product->life);
					free(current_product);
				} else {
					// Reset the product's next since it is going to the end of the queue
					current_product->next = NULL;

					// Put the product at the end of the queue (or the head if the queue is now empty)
					if (head != NULL){
						struct product* last = head;
						while (last->next != NULL)
							last = last->next;
						last->next = current_product;
					} else {
						head = current_product;
					}

					printf("Consumer #%d consumed product #%d with life #%d\n", *i, current_product->productID, current_product->life);
				}
			}

			// Release the lock on the queue
			pthread_mutex_unlock(&access_queue);
		}

		// Sleep for 100 milliseconds
		usleep(100000);
	}
}
