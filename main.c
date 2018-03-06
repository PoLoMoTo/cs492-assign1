#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h> 
#include <errno.h>
#include <stdlib.h>

// Struct product queue
struct product {
	int productID;
	struct timeval timeGenerated;
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

// Time arrays:
// Consumption time in seconds 
double* consumptionTime;
// Turn around time in seconds
double* turnAroundTime; 

// Queue lock
pthread_mutex_t access_queue;

// Full and empty conditions
pthread_cond_t notFull;
pthread_cond_t notEmpty;

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
	int products = strtol(argv[3], NULL, 10);
	remaining_products = products;
	maxQueue = strtol(argv[4], NULL, 10);
	algorithm = strtol(argv[5], NULL, 10); // 0 = FCFS and 1 = RR
	quantum = strtol(argv[6], NULL, 10);
	unsigned int seed = strtol(argv[7], NULL, 10);

	// Initialize the random number generator with the seed
	srand(seed);

	// Initialize the queue lock
	pthread_mutex_init(&access_queue, NULL);

	// Initialize The condition variables
	pthread_cond_init(&notFull, NULL);
	pthread_cond_init(&notEmpty, NULL);

	// Initialize the time arrays
	consumptionTime = calloc(remaining_products, sizeof(struct timeval));
	turnAroundTime = calloc(remaining_products, sizeof(struct timeval));

	// Create threads for the consumers and producers
	pthread_t prod_thread[prodThreads];
	pthread_t consum_thread[consumThreads];
	int prodn[prodThreads];
	int consumn[consumThreads];

	void* producer();
	void* consumer();

	// Set producer start time
	struct timeval producerStartTime;
	gettimeofday(&producerStartTime, NULL);

	// Start the producers
	for (int i = 0; i < prodThreads; i++){
		prodn[i] = i;
		pthread_create(&prod_thread[i], NULL, producer, &prodn[i]);
	}

	// Set consumer start time
	struct timeval consumerStartTime;
	gettimeofday(&consumerStartTime, NULL);

	// Start the consumers
	for (int i = 0; i < consumThreads; i++){
		consumn[i] = i;
		pthread_create(&consum_thread[i], NULL, consumer, &consumn[i]);
	}

	// Wait for all producers to exit
	for (int i = 0; i < prodThreads; i++)
		pthread_join(prod_thread[i], NULL);

	// Calculate producer time
	struct timeval producerEndTime;
	gettimeofday(&producerEndTime, NULL);

	// Wait for all consumers to exit
	for (int i = 0; i < consumThreads; i++)
		pthread_join(consum_thread[i], NULL);

	// Calculate consumer time
	struct timeval consumerEndTime;
	gettimeofday(&consumerEndTime,NULL);

	// Destroy mutex and conditionals
	pthread_mutex_destroy(&access_queue);
	pthread_cond_destroy(&notEmpty);
	pthread_cond_destroy(&notFull);

	// Calculate the total time processing took
	printf("Total processing time: %f minutes\n", (float)((1000000 * consumerEndTime.tv_sec + consumerEndTime.tv_usec) - (1000000 * producerStartTime.tv_sec + producerStartTime.tv_usec))/60000000);

	// Get the statistics for the turn around time
	double minTurnAround = -1;
	double maxTurnAround = -1;
	double aveTurnAround = 0;
	for (int i = 0; i < products; i++){
		double TurnAround = turnAroundTime[i];
		aveTurnAround += TurnAround;
		if (TurnAround > maxTurnAround || maxTurnAround == -1)
			maxTurnAround = TurnAround;
		else if (TurnAround < minTurnAround || minTurnAround == -1)
			minTurnAround = TurnAround;
	}

	// Print turn around stats
	printf("Average turn around time: %f seconds\n", aveTurnAround/products);
	printf("Minimum turn around time: %f seconds\n", minTurnAround);
	printf("Maximum turn around time: %f seconds\n", maxTurnAround);

	// Get the statistics for the wait time calculated by subtracting the consumption time from the turn around time
	double minWait = -1;
	double maxWait = -1;
	double aveWait = 0;
	for (int i = 0; i < products; i++){
		double wait = turnAroundTime[i] - consumptionTime[i];
		aveWait += wait;
		if (wait > maxWait || maxWait == -1)
			maxWait = wait;
		else if (wait < minWait || minWait == -1)
			minWait = wait;
	}

	// Print wait stats
	printf("Average wait time: %f seconds\n", aveWait/products);
	printf("Minimum wait time: %f seconds\n", minWait);
	printf("Maximum wait time: %f seconds\n", maxWait);

	// Calculate and print producer and consumer throughputs
	printf("Producer throughput: %f products per minute\n", (float)(products / ((float)((1000000 * producerEndTime.tv_sec + producerEndTime.tv_usec) - (1000000 * producerStartTime.tv_sec + producerStartTime.tv_usec))/60000000)));
	printf("Consumer throughput: %f products per minute\n", (float)(products / ((float)((1000000 * consumerEndTime.tv_sec + consumerEndTime.tv_usec) - (1000000 * consumerStartTime.tv_sec + consumerStartTime.tv_usec))/60000000)));

	// Exit
	pthread_exit(0);
}

void* producer(int* i){
	while (1){
		// Obtain lock on the queue
		pthread_mutex_lock(&access_queue);

		// Wait for signal
		if (maxQueue != 0)
			while (queueLength() >= maxQueue)
				pthread_cond_wait(&notFull, &access_queue);

		// Make sure there are more products
		if (remaining_products <= 0){
			// No more products to produce, release the queue, signal again that the queue is not full, and exit thread
			pthread_mutex_unlock(&access_queue);
			pthread_cond_signal(&notFull);  // This signal is required to make sure all producers get the signal that all products were produced
			pthread_exit(NULL);
		} else {
			// Create the new product
			struct product* new_product = malloc(sizeof(struct product));
			new_product->productID = remaining_products;
			new_product->next = NULL;
			new_product->life = rand() % 1024;
			gettimeofday(&new_product->timeGenerated,NULL);

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

			printf("Producer #%d produced product #%d\n", *i, new_product->productID);

			// Release the lock on the queue
			pthread_mutex_unlock(&access_queue);

			// Signal that the queue is not empty
			pthread_cond_signal(&notEmpty);
		}

		// Sleep for 100 milliseconds
		usleep(100000);
	}
}

void* consumer(int* i){
	while (1){
		// Obtain lock on the queue
		pthread_mutex_lock(&access_queue);

		// Wait for signal if all items are not consumed
		while (head == NULL){
			if (remaining_products <= 0){
				pthread_mutex_unlock(&access_queue);
				pthread_exit(NULL);
			} else
				pthread_cond_wait(&notEmpty, &access_queue);
		}

		// Pull first item from the queue
		struct product* current_product = head;
		head = current_product->next;

		if (algorithm == 0){
			// Get the time of the start of the consume
			struct timeval startConsumeTime;
			gettimeofday(&startConsumeTime, NULL);

			// Consume the product
			for (int i = 0; i < current_product->life; i++)
				fibonacci(10);

			// Get the time of the end of the consume
			struct timeval endConsumeTime;
			gettimeofday(&endConsumeTime, NULL);

			// Save the consume time
			consumptionTime[current_product->productID-1] += (double)((1000000 * endConsumeTime.tv_sec + endConsumeTime.tv_usec) - (1000000 * startConsumeTime.tv_sec + startConsumeTime.tv_usec))/1000000;

			// Save the finish time
			turnAroundTime[current_product->productID-1] = (double)((1000000 * endConsumeTime.tv_sec + endConsumeTime.tv_usec) - (1000000 * current_product->timeGenerated.tv_sec + current_product->timeGenerated.tv_usec))/1000000;

			printf("Consumer #%d consumed product #%d\n", *i, current_product->productID);

			// Free the pulled node
			free(current_product);

			// Unlock the queue and signal
			pthread_mutex_unlock(&access_queue);
			if (head == NULL && remaining_products <= 0){
				pthread_cond_signal(&notEmpty);
				pthread_exit(NULL);
			} else if (remaining_products <= 0)
				pthread_cond_signal(&notEmpty);
			else
				pthread_cond_signal(&notFull);

		} else {
			// Consume the product the quantum amount
			if (current_product->life - quantum < 0){
				// Get the time of the start of the consume
				struct timeval startConsumeTime;
				gettimeofday(&startConsumeTime, NULL);

				for (int i = 0; i < current_product->life; i++)
					fibonacci(10);

				// Get the time of the end of the consume
				struct timeval endConsumeTime;
				gettimeofday(&endConsumeTime, NULL);

				// Save the consume time
				consumptionTime[current_product->productID-1] += (float)((1000000 * endConsumeTime.tv_sec + endConsumeTime.tv_usec) - (1000000 * startConsumeTime.tv_sec + startConsumeTime.tv_usec))/1000000;

				// Decrement the product's life counter
				current_product->life = 0;
			} else {
				// Get the time of the start of the consume
				struct timeval startConsumeTime;
				gettimeofday(&startConsumeTime, NULL);

				for (int i = 0; i < quantum; i++)
					fibonacci(10);

				// Get the time of the end of the consume
				struct timeval endConsumeTime;
				gettimeofday(&endConsumeTime, NULL);

				// Save the consume time
				consumptionTime[current_product->productID-1] += (float)((1000000 * endConsumeTime.tv_sec + endConsumeTime.tv_usec) - (1000000 * startConsumeTime.tv_sec + startConsumeTime.tv_usec))/1000000;

				// Decrement the product's life counter
				current_product->life -= quantum;
			}

			// If the product's life is over free it, otherwise add it back to the end of the queue
			if (current_product->life <= 0){
				// Get the final time
				struct timeval endConsumeTime;
				gettimeofday(&endConsumeTime, NULL);

				// Save the finish time
				turnAroundTime[current_product->productID-1] = (float)((1000000 * endConsumeTime.tv_sec + endConsumeTime.tv_usec) - (1000000 * current_product->timeGenerated.tv_sec + current_product->timeGenerated.tv_usec))/1000000;

				printf("Consumer #%d consumed product #%d\n", *i, current_product->productID);
				free(current_product);

				// If this was the last item then exit otherwise unlock and signal
				if (remaining_products <= 0){
					pthread_mutex_unlock(&access_queue);
					if (head != NULL)
						pthread_cond_signal(&notEmpty);
					else
						pthread_exit(NULL);
				} else {
					pthread_mutex_unlock(&access_queue);
					pthread_cond_signal(&notFull);
				}
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

				printf("Consumer #%d consumed product #%d\n", *i, current_product->productID);

				// Release the lock on the queue and signal
				pthread_mutex_unlock(&access_queue);
				pthread_cond_signal(&notEmpty);
			}
		}

		// Sleep for 100 milliseconds
		usleep(100000);
	}
}
