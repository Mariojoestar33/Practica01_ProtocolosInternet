#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080
#define MAXLINE 4096

// Función para enviar la lista de elementos
void send_list(int sockfd, struct sockaddr_in *cliaddr, socklen_t len) {
    const char *list[] = {"Bohemian Rhapsody - ID = 'Bohemian'", "Radio Ga Ga - ID = 'Radio'", "I want to break free - ID = 'Break'"};
    int num_elements = sizeof(list) / sizeof(list[0]);
    char buffer[MAXLINE];
    // Construir la lista en una cadena
    strcpy(buffer, "Lista de elementos: (Si desea recibir uno, escriba su ID)\n");
    for (int i = 0; i < num_elements; ++i) {
        strcat(buffer, list[i]);
        strcat(buffer, "\n");
    }
    // Enviar la lista al cliente
    sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)cliaddr, len);
    // Enviar delimitador de final de archivo al cliente
    char *end_of_file = "EOR";
    sendto(sockfd, end_of_file, strlen(end_of_file), 0, (const struct sockaddr *)cliaddr, len);
}

//Funcion para leer un archivo y mandar su contenido por medio del socket
void send_file_content(int sockfd, struct sockaddr_in *cliaddr, socklen_t len, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }
    char buffer[MAXLINE];
    size_t bytes_read;
    // Leer el contenido del archivo y enviarlo al cliente
    while ((bytes_read = fread(buffer, 1, MAXLINE, file)) > 0) {
        sendto(sockfd, buffer, bytes_read, 0, (const struct sockaddr *)cliaddr, len);
    }
    // Enviar delimitador de final de archivo al cliente
    char *end_of_file = "EOF";
    sendto(sockfd, end_of_file, strlen(end_of_file), 0, (const struct sockaddr *)cliaddr, len);
    fclose(file); //Cierre del archivo
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAXLINE];
    const char *welcome_message = "¡Bienvenido al servidor UDP!\n-lista";
    // Crear un socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Fallo al crear el socket...");
        exit(EXIT_FAILURE);
    }
    // Inicializar la estructura del servidor
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    // Configurar la información del servidor
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY; // Recibira paquetes de cualquier dirección IP de la máquina
    servaddr.sin_port = htons(PORT); //Número de puerto donde se recibirán paquetes
    // Vincular el socket con la dirección y el puerto
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error al vincular el socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket creado y vinculado correctamente.\n");

    // Escuchar en el socket y responder a los request
    while(1) {
    
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0'; // Asegurarse de que la cadena esté terminada correctamente
        printf("Mensaje recibido: %s\n", buffer);

        // Verificar si el mensaje recibido es "lista"
        if (strcmp(buffer, "lista") == 0) {
            send_list(sockfd, &cliaddr, len);
            printf("Lista de elementos enviada.\n");
            continue;
        //Se verifica si la peticion es alguno de los archivos que se tienen en la lista
        } else if (strcmp(buffer, "Bohemian") == 0) {
            send_file_content(sockfd, &cliaddr, len, "Bohemian.txt");
            printf("Contenido de Bohemian.txt enviado.\n");
            continue;
        } else if (strcmp(buffer, "Radio") == 0) {
            send_file_content(sockfd, &cliaddr, len, "Radio.txt");
            printf("Contenido de Radio.txt enviado.\n");
            continue;
        } else if (strcmp(buffer, "Break") == 0) {
            send_file_content(sockfd, &cliaddr, len, "Break.txt");
            printf("Contenido de Break.txt enviado.\n");
            continue;
        } else {
            // Enviar un mensaje de bienvenida al cliente
            sendto(sockfd, welcome_message, strlen(welcome_message), 0, (const struct sockaddr *)&cliaddr, len);
            char *end_of_file = "EOR";
            sendto(sockfd, end_of_file, strlen(end_of_file), 0, (const struct sockaddr *)&cliaddr, len);
            printf("Mensaje de bienvenida enviado.\n");
        }
        memset(buffer, 0, sizeof(buffer));
    }

    close(sockfd); // Cerrar el socket

    return 0;
}