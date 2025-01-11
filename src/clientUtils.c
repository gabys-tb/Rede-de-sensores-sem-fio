#include "clientUtils.h"


void print_error() {
    printf("Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
}

// Verifica se dois sensores são iguais
int check_same_sensor(ThisSensor s1, int s2[2]) {
    if (s1.coord[0] == s2[0] && s1.coord[1] == s2[1]) {
        return 1;
    } 
    return 0;
}

// Auxilia a verificação de sensores na ordenação na lista encadeada
int same_sensor(int s1[2], int s2[2]) {
    if (s1[0] == s2[0] && s1[1] == s2[1]) {
        return 1;
    } 
    return 0;
}

// Atribui os dados de medição para o tipo de sensor específico
void sensor_data(char *sensor, int *t, int range[2]) {
    if (strcmp(sensor, "temperature") == 0) {
        *t = 5;
        range[0] = 20;
        range[1] = 40;
    } else if (strcmp(sensor, "humidity") == 0) {
        *t = 7;
        range[0] = 10;
        range[1] = 90;
    } else {
        *t = 10;
        range[0] = 15;
        range[1] = 30;
    }
}

// Retorna um valor aleatório dentro do range passado como argumento
float random_measurement(float min, float max) {
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * (max - min);      /* [min, max] */
}

// Calcula a distância euclidiana entre dois pontos
float calculate_distance(ThisSensor s1, int s2[2]) {
    return sqrt(pow(s1.coord[0] - s2[0], 2) + pow(s1.coord[1] - s2[1], 2));
}

// Retorna a atualização a ser feita na medida do sensor
float update_measurement(ThisSensor this_sensor, float distance, float measurement) {
    return 0.1 * (1.0 / (distance + 1.0)) * (measurement - this_sensor.measurement);
}

// Seta valores do sensor dado a mensagem recebida
void set_sensor(struct sensor_message msg, int sensor[2]) {
    sensor[0] = msg.coords[0];
    sensor[1] = msg.coords[1];
}

// Seta valores da mensagem a ser enviada
void set_msg(struct sensor_message *msg, ThisSensor sensor) {
    msg->coords[0] = sensor.coord[0];
    msg->coords[1] = sensor.coord[1];
    msg->measurement = sensor.measurement;
}

// Insere um nó na lista encadeada, mantendo a lista ordenada pela distância
int insert_sorted(Node **head, int sensor[2], float distance) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    
    new_node->sensor[0] = sensor[0]; new_node->sensor[1] = sensor[1];
    new_node->distance = distance;
    new_node->next = NULL;
    int pos = 1;

    // Verifica se o sensor já está na cabeça
    if (*head != NULL && same_sensor(sensor, (*head)->sensor)) {
        free(new_node);
        return pos;
    } else if (*head == NULL || (*head)->distance > distance) {
        new_node->next = *head;
        *head = new_node;
    } else {
        // Encontra o local correto para inserir o nó
        Node *current = *head;
        pos++;
        while (current->next != NULL && current->next->distance < distance) {
            current = current->next;
            pos++;
        }
        if (current->next != NULL && same_sensor(current->next->sensor, sensor)) {
            free(new_node);
            return pos;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
    return pos;
}


// Remove um sensor e reordena a lista de acordo
void remove_node(Node **head, int coord[2]) {
    // Remove o nó da lista
    Node *current = *head;
    Node *previous = NULL;
    while (current != NULL) {
        if (current->sensor[0] == coord[0] && current->sensor[1] == coord[1]) {
            if (previous == NULL) {
                *head = current->next;  // Remove o nó do início da lista
            } else {
                previous->next = current->next;  // Remove o nó de qualquer lugar na lista
            }
            free(current);
            break;
        }
        previous = current;
        current = current->next;
    }
}
