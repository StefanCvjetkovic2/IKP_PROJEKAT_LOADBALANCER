#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#define CLIENT_NAME_LEN 10
#define BUFFER_WITHOUT_NAME 246
#define BUFFER_SIZE CLIENT_NAME_LEN+BUFFER_WITHOUT_NAME

// A structure to represent a queue
typedef struct messageStruct {
	char clientName[CLIENT_NAME_LEN];
	char bufferNoName[BUFFER_WITHOUT_NAME];
}messageStruct;

typedef struct {
	CRITICAL_SECTION cs;
	int front, rear, currentSize;
	int capacity;
	messageStruct** messageArray;
}queue;

extern queue* q;

void create_queue(int capacity);
int is_queue_full();
int is_queue_empty();
void enqueue(messageStruct* message);
void dequeue(messageStruct** message);
void print_queue();
int get_current_size_queue();
int get_capacity_queue();
void delete_queue();
