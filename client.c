#include "clientUtils.h"

int sock;

ThisSensor this_sensor;

// Termina o programa no caso de SIGINT
void handle_exit(int sig) {
    // Cria a estrutura para a medição negativa
    struct sensor_message msg;
    strcpy(msg.type, this_sensor.type);  
    msg.coords[0] = this_sensor.coord[0];
    msg.coords[1] = this_sensor.coord[1];
    msg.measurement = -1.0;  

    // Envia a medição negativa para o servidor
    send(sock, &msg, sizeof(msg), 0);

    // Fecha o socket
    close(sock);
    exit(0);  // Finaliza o cliente
}

int main(int argc, char *argv[]) {
    srand(time(NULL)); 

    signal(SIGINT, handle_exit);

    // Valida número de arumentos
    if (argc != 8) {
        fprintf(stderr, "Error: Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    // Valida -type
    if (strcmp(argv[3], "-type") != 0) {
        fprintf(stderr, "Error: Expected '-type' argument\n");
        print_error();
        return EXIT_FAILURE;
    }

    // Valida o tipo de sensor
    int time_sensor = 0;
    int range[2];
    if (strcmp(argv[4], "temperature") != 0 && strcmp(argv[4], "humidity") != 0 && strcmp(argv[4], "air_quality") != 0) {
        fprintf(stderr, "Error: Invalid sensor type\n");
        print_error();
        return EXIT_FAILURE;
    } else {
        sensor_data(argv[4], &time_sensor, range);
    }

    // Valida o argumento de coordenadas
    if (strcmp(argv[5], "-coords") != 0) {
        fprintf(stderr, "Error: Expected '-coords' argument\n");
        print_error();
        return EXIT_FAILURE;
    }

    // Converte as coordenadas para int e valida elas
    int x = atoi(argv[6]);
    int y = atoi(argv[7]);
    if (x < 0 || x > 9 || y < 0 || y > 9) {
        fprintf(stderr, "Error: Coordinates must be in the range 0-9\n");
        print_error();
        return EXIT_FAILURE;
    }
    
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    char *type = argv[4];
    int sock = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in6 server_addr6;

    // Verifica se o endereço é IPv6
    if (strchr(server_ip, ':')) {
        sock = socket(AF_INET6, SOCK_STREAM, 0);
        if (sock < 0) {
            return 1;
        }

        memset(&server_addr6, 0, sizeof(server_addr6));
        server_addr6.sin6_family = AF_INET6;
        server_addr6.sin6_port = htons(port);
        if (inet_pton(AF_INET6, server_ip, &server_addr6.sin6_addr) <= 0) {
            return 1;
        }

        if (connect(sock, (struct sockaddr *)&server_addr6, sizeof(server_addr6)) < 0) {
            close(sock);
            return 1;
        }
    } else { // Conexão do tipo IPv4
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return 1;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            return 1;
        }

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            return 1;
        }
    }

    struct sensor_message msg;
    this_sensor.coord[0] = x;
    this_sensor.coord[1] = y;
    this_sensor.measurement = random_measurement(range[0], range[1]);  
    snprintf(this_sensor.type, sizeof(this_sensor.type), "%s", type); 
    
    // Inicializa a mensagme do sensor
    snprintf(msg.type, sizeof(msg.type), "%s", type);  
    msg.coords[0] = x; 
    msg.coords[1] = y;  
    msg.measurement = this_sensor.measurement; // Seta valor medido inicialmente
    
    send(sock, &msg, sizeof(msg), 0);
    sleep(time_sensor); 
    
    Node *head = NULL;
    int received_sensor[2] = {0};
    char status[50];
    while (1) {
        int len = recv(sock, &msg, sizeof(msg), 0);
        // Verifica tipo de sensor da mensagem
        if (strcmp(msg.type, this_sensor.type) != 0) {
            continue;
        }
        if (len > 0) {
            // Atualiza o sensor auxiliar com os valores da mensagem da vez
            set_sensor(msg, received_sensor);

            if (msg.measurement == -1) {
                strcpy(status, "removed");
                remove_node(&head, received_sensor);

            } else if (!check_same_sensor(this_sensor, received_sensor)) {
                // Verifica se sensor já está na lista
                float distance = calculate_distance(this_sensor, received_sensor);
                int pos = insert_sorted(&head, received_sensor, distance);
                if (pos < 4) {
                    float adjustement = update_measurement(this_sensor, distance, msg.measurement );
                    // Corrige a medida se tiver ultrapassado o intervalo permitido
                    if(this_sensor.measurement + adjustement < range[0]) {
                        adjustement = this_sensor.measurement - range[0];
                        this_sensor.measurement = range[0];
                    } else if (this_sensor.measurement + adjustement > range[1]) {
                        adjustement = range[1] - this_sensor.measurement;
                        this_sensor.measurement = range[1];
                    } else {
                        this_sensor.measurement += adjustement;
                    }
                    sprintf(status, "correction of %.4f", adjustement);
                } else {
                    strcpy(status, "not neighbor");      
                }
            } else {
                strcpy(status, "same location");
            }
            // Exibe o log da mensagem recebida
            printf("log:\n%s sensor in (%d, %d)\nmeasurement: %.4f\naction: %s\n\n",
            msg.type, msg.coords[0], msg.coords[1], msg.measurement, status);

        } else {
            break;
        }

        // Envia mensagem com os dados do sensor atualizados
        set_msg(&msg, this_sensor);
        send(sock, &msg, sizeof(msg), 0);
        memset(&msg, 0, sizeof(msg));

        sleep(time_sensor); 
    }
   
    close(sock);
    return 0;
}
