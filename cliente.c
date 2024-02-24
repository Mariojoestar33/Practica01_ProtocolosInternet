#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080
#define MAXLINE 4096

//Funcion para limpiar la pantalla
void limpiar() {
    system("clear");
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    // Crear un socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Fallo al crear el socket...");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Configurar la información del servidor
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_port = htons(PORT); // Puerto del servidor
    servaddr.sin_addr.s_addr = INADDR_ANY; //Aceptar cualquier dirección IP

    // Variables para almacenar el mensaje del usuario
    char input[MAXLINE];

    while (1) {
        limpiar();
        // Solicitar al usuario que ingrese un mensaje
        printf("Ingrese un mensaje (o escriba 'exit' para salir): ");
        fgets(input, MAXLINE, stdin);

        // Eliminar el salto de línea del final del mensaje
        input[strcspn(input, "\n")] = 0;

        // Salir del ciclo y cerrar el socket si se escribe "exit"
        if (strcmp(input, "exit") == 0)
            break;

        // Enviar el mensaje al servidor
        sendto(sockfd, (const char *)input, strlen(input), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        printf("Mensaje enviado.\n");

        // Recibir la respuesta del servidor
        int n;
        while ((n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, NULL, NULL)) > 0) {
            buffer[n] = '\0'; // Asegurarse de que la cadena esté terminada correctamente

            // Verificar si es el delimitador de final de archivo
            if (strcmp(buffer, "EOF") == 0) {
                printf("Archivo recibido completamente.\n");
                break; // Salir del bucle de recepción
            }
            // Verificar si es el delimitador de final de archivo
            if (strcmp(buffer, "EOR") == 0) {
                printf("Respuesta recibida completamente.\n");
                break; // Salir del bucle de recepción
            }
            // Imprimir la respuesta del servidor
            printf("Respuesta del servidor: %s\n", buffer);

            memset(buffer, 0, sizeof(buffer));
        }
    }

    // Cerrar el socket
    close(sockfd);

    return 0;
}