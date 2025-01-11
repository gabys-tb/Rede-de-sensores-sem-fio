#ifndef SENSOR_HEAP_H
#define SENSOR_HEAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <time.h>


// Definição das estruturas
#define MAX_NEIGHBORS 100

struct sensor_message {
    char type[12];
    int coords[2];
    float measurement;
};


typedef struct {
    int coord[2];  // Coordenadas
    float measurement;
    char type[12];
} ThisSensor;


// Definindo o nó da lista encadeada para armazenar um sensor e sua distância
typedef struct Node {
    int sensor[2]; // Coordenadas do sensor
    float distance;  // Distância do sensor de referência
    struct Node *next;  // Ponteiro para o próximo nó
} Node;


float calculate_distance(ThisSensor s1, int s2[2]);
void print_error();
void sensor_data(char *sensor, int *t, int range[2]);
float random_measurement(float min, float max);
float update_measurement(ThisSensor this_sensor, float distance, float measurement);
int check_same_sensor(ThisSensor s1, int s2[2]);
void set_sensor(struct sensor_message msg, int sensor[2]);
void set_msg(struct sensor_message *msg, ThisSensor sensor);
int insert_sorted(Node **head, int sensor[2], float distance);
void remove_node(Node **head, int coord[2]);

#endif // SENSOR_HEAP_H
