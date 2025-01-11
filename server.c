#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define MAX_TOPICS 3
#define MAX_SENSORS_PER_TOPIC 30
#define MAX_CLIENTS 100

#define BUFFER_SIZE 1024

struct sensor_message {
    char type[12];
    int coords[2];
    float measurement;
};

typedef struct client {
    int sock;
    struct sockaddr_storage address;
    socklen_t addr_len;
    char topic[50];
} client_t;

// Variáveis para gerenciar clientes
client_t *subscribed_clients[MAX_TOPICS][MAX_SENSORS_PER_TOPIC];
pthread_mutex_t topics_mutex = PTHREAD_MUTEX_INITIALIZER;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


void *handle_client(void *arg);
void add_client(client_t *cli);

int main(int argc, char *argv[]) {
    // Verifica argumentos do terminal
    if (argc < 3) {
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[2]);
    int server_fd;
    struct sockaddr_storage server_storage;
    socklen_t addr_size = sizeof(server_storage);

    // Verifica o tipo conexão (IPv6 ou IPv4)
    if (strcmp(argv[1], "v4") == 0) {
        struct sockaddr_in server_addr;
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(argv[1], "v6") == 0) {
        struct sockaddr_in6 server_addr6;
        server_fd = socket(AF_INET6, SOCK_STREAM, 0);
        memset(&server_addr6, 0, sizeof(server_addr6));
        server_addr6.sin6_family = AF_INET6;
        server_addr6.sin6_addr = in6addr_any;
        server_addr6.sin6_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&server_addr6, sizeof(server_addr6)) < 0) {
            exit(EXIT_FAILURE);
        }
    } else {
        exit(EXIT_FAILURE);
    }

    // Inicializa variáveis globais
    for (int i = 0; i < MAX_TOPICS; i++) {
        for (int j = 0; j < MAX_SENSORS_PER_TOPIC; j++) {
            subscribed_clients[i][j] = NULL;
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }

    listen(server_fd, 15);

    signal(SIGPIPE, SIG_IGN);

    // Loop para conectar clientes 
    while (1) {
        int client_sock = accept(server_fd, (struct sockaddr *)&server_storage, &addr_size);
        if (client_sock < 0) {
            continue;
        }

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        if (cli == NULL) {
            close(client_sock);
            continue;
        }
        
        cli->sock = client_sock;
        memcpy(&cli->address, &server_storage, addr_size);
        cli->addr_len = addr_size;
        strcpy(cli->topic, "default"); // Tópico default

        add_client(cli);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)cli);
        pthread_detach(thread_id); 
    }

    return 0;
}

// Adiciona cliente à lista global
void add_client(client_t *cli) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i]) {
            clients[i] = cli;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Remove cliente da lista global
void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->sock == sock) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Mapeia o tópico à linha respectiva na matriz de inscritos por tópico
int map_topic(const char *topic) {
    if (strcmp(topic, "temperature") == 0) {
        return 0;
    } else if (strcmp(topic, "humidity") == 0) {
        return 1; 
    } else {
        return 2;
    }
}

// Inscreve cliente no seu respectivo tópico
void subscribe_client_to_topic(client_t *cli, int topic_number) {
    pthread_mutex_lock(&topics_mutex);
    for (int j = 0; j < MAX_SENSORS_PER_TOPIC; j++) {
        if (!subscribed_clients[topic_number][j]) {
            subscribed_clients[topic_number][j] = cli;
            strcpy(cli->topic, subscribed_clients[topic_number][j]->topic);
            break;
        }
    }
    pthread_mutex_unlock(&topics_mutex);
}

// Publica a mensagem no tópico devido
void publish_to_topic(struct sensor_message message, int topic_number) {
    pthread_mutex_lock(&topics_mutex);
    for (int j = 0; j < MAX_SENSORS_PER_TOPIC; j++) {
        if (subscribed_clients[topic_number][j]) {
            // Verifica se o envio da mensagem foi bem sucedido
            if (write(subscribed_clients[topic_number][j]->sock, &message, sizeof(message)) < 0) {
                // Se não foi, remove cliente da matriz
                subscribed_clients[topic_number][j] = NULL;
            }
        }    
    }
    pthread_mutex_unlock(&topics_mutex);
}

void *handle_client(void *arg) {
    client_t *cli = (client_t *)arg;
   
    char buffer[sizeof(struct sensor_message)] = {0};
    struct sensor_message msg;
    int subscribed = 0;
    int topic_number = -1;
    int this_sensor[2];
    
    while (1) {
        // Lê a mensagem do cliente
        ssize_t read_size = read(cli->sock, buffer, sizeof(struct sensor_message));
        memcpy(&msg, buffer, sizeof(struct sensor_message));
        if (read_size > 0) {
            if (!subscribed) {
                // A primeira mensagem inscreve o sensor no tópico
                topic_number = map_topic(msg.type);
                subscribe_client_to_topic(cli, topic_number);
                subscribed = 1;
                this_sensor[0] = msg.coords[0];
                this_sensor[1] = msg.coords[1];
            }
            // Printa o log
            printf("log:\n%s sensor in (%d, %d)\nmeasurement: %.4f\n\n", msg.type, msg.coords[0], msg.coords[1], msg.measurement);
            // Broadcast da mensagem para todos inscritos do mesmo tópico
            publish_to_topic(msg, topic_number);
            if (msg.measurement == -1) {
                // Termina a conexão
                break;
            }
        } else if (read_size == 0) { // Se a leitura falha, entende a conexão como terminada
            float f = -1;
            printf("log:\n%s sensor in (%d, %d)\nmeasurement: %.4f\n\n", msg.type, this_sensor[0], this_sensor[1], f);
            break;
        } else {
            float f = -1;
            printf("log:\n%s sensor in (%d, %d)\nmeasurement: %.4f\n\n", msg.type, this_sensor[0], this_sensor[1], f);
            break;
        }
    }
    close(cli->sock);
    remove_client(cli->sock);
    free(cli);
    return NULL;
}
