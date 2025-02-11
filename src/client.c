#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

// Definire constante
#define SERV_HOST "34.246.184.49"
#define SERV_PORT 8080
#define JSON_CONTENT_TYPE "application/json"
#define REGISTER_PATH "/api/v1/tema/auth/register"
#define LOGIN_PATH "/api/v1/tema/auth/login"
#define ACCESS_PATH "/api/v1/tema/library/access"
#define BOOKS_PATH "/api/v1/tema/library/books"
#define LOGOUT_PATH "/api/v1/tema/auth/logout"

void process_register_command() {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    char username[100], password[100];

    // Citește username-ul de la utilizator
    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';
    if (strlen(username) == 0) {
        printf("Error! The username cannot be empty.\n");
        return;
    }

    // Citește parola de la utilizator
    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0';
    if (strlen(password) == 0) {
        printf("Error! The password cannot be empty.\n");
        return;
    }

    // Creează obiect JSON pentru username și parolă
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);

    // Serializează obiectul JSON
    char *payload[1];
    payload[0] = json_serialize_to_string_pretty(value);

    // Creează cererea POST pentru înregistrare
    char *message = compute_post_request(SERV_HOST, REGISTER_PATH, JSON_CONTENT_TYPE, 
                    payload, 1, NULL, 0, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verifică dacă a apărut o eroare
    if (strstr(response, "error") != NULL) {
        printf("Error! The username you entered is already taken.\n");
        return;
    }

    printf("Success! You are now registered.\n");

    // Eliberare memorie
    json_free_serialized_string(payload[0]);
    json_value_free(value);
    free(response);
            
    // Închide conexiunea
    close_connection(sockfd);
}

int process_login_command(char *cookies[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    char username[100], password[100];

    // Citește username-ul de la utilizator
    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';

    // Citește parola de la utilizator
    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0';

    // Creează obiect JSON pentru username și parolă
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);

    // Serializează obiectul JSON
    char *payload[1];
    payload[0] = json_serialize_to_string_pretty(value);

    // Creează cererea POST pentru login
    char *message = compute_post_request(SERV_HOST, LOGIN_PATH, JSON_CONTENT_TYPE, 
                    payload, 1, NULL, 0, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verifică dacă a apărut o eroare
    if (strstr(response, "error") != NULL) {
        printf("Error! The username and password you entered don't match to any user.\n");
        return 0;
    }
    
    // Extrage cookie-ul de sesiune din răspuns
    char *cookie_start = strstr(response, "connect.sid=");
    if (cookie_start == NULL) {
        printf("Error! Didn't receive session cookie from the server.\n");
        return 0;
    }

    char *cookie_end = cookie_start;
    int cookie_len = 0;

    while (*cookie_end != ';') {
        cookie_end++;
        cookie_len++;
    }

    char *session_cookie = (char *)malloc(sizeof(char) * (cookie_len + 1));
    if (session_cookie == NULL) {
        printf("Error! Couldn't allocate memory for the session cookie.\n");
        return 0;
    }

    strncpy(session_cookie, cookie_start, cookie_len);
    session_cookie[cookie_len] = '\0';

    cookies[0] = session_cookie;

    printf("Success! You are now logged in.\n");

    // Eliberare memorie
    json_free_serialized_string(payload[0]);
    json_value_free(value);
    free(response);
            
    // Închide conexiunea
    close_connection(sockfd);

    return 1;
}

int process_enter_library_command(char *cookies[], char token[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    // Creează cererea GET pentru acces la bibliotecă
    char *message = compute_get_request(SERV_HOST, ACCESS_PATH, NULL,
                    cookies, 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Extrage token-ul din răspuns
    char *token_start = strstr(response, "token");
    if (token_start == NULL) {
        printf("Error! Didn't receive token from the server.\n");
        return 0;
    }

    token_start += strlen("token\":\"");
    char *token_end = token_start;
    int token_len = 0;

    while (*token_end != '"') {
        token_end++;
        token_len++;
    }

    strncpy(token, token_start, token_len);
    token[token_len] = '\0';

    printf("Success! You now have access to the library.\n");

    return 1;
}

void process_get_books_command(char *cookies[], char token[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    // Creează cererea GET pentru lista de cărți
    char *message = compute_get_request(SERV_HOST, BOOKS_PATH, NULL,
                    cookies, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Extrage răspunsul JSON cu lista de cărți
    char* books_response = strstr(response, "[");
    if (books_response == NULL) {
        printf("Error! Didn't receive book list from the server.\n");
        return;
    }

    if (strstr(response, "[{") == NULL) {
        printf("There are no books in the library.\n");
        return;
    }

    JSON_Value *value = json_parse_string(books_response);
    char *books_string = json_serialize_to_string_pretty(value);

    printf ("%s\n", books_string);

    // Eliberare memorie
    json_value_free(value);

    // Închide conexiunea
    close_connection(sockfd);
}

void process_get_book_command(char *cookies[], char token[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    int book_id;
    
    // Citește ID-ul cărții de la utilizator
    printf("id=");
    scanf("%d", &book_id);
    getchar();

    char BOOK_PATH[100];
    sprintf(BOOK_PATH, "%s/%d",BOOKS_PATH, book_id);

    // Creează cererea GET pentru detaliile unei cărți
    char *message = compute_get_request(SERV_HOST, BOOK_PATH, NULL,
                    cookies, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verifică dacă a apărut o eroare
    if (strstr(response, "error") != NULL) {
        printf("Error! The book id you entered is invalid.\n");
        return;
    }

    // Extrage răspunsul JSON cu detaliile cărții
    char *book_response = strstr(response, "{");
    if (book_response == NULL) {
        printf("Error! Didn't receive book from the server.\n");
        return;
    }

    JSON_Value *value = json_parse_string(book_response);
    char *book_string = json_serialize_to_string_pretty(value);

    printf("%s\n", book_string);

    // Eliberare memorie
    json_value_free(value);

    // Închide conexiunea
    close_connection(sockfd);
}

void process_add_book_command(char *cookies[], char token[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    char title[100], author[100], genre[100], publisher[100];
    int page_count;

    // Citește detaliile cărții de la utilizator
    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strlen(title) - 1] = '\0';
    if (strlen(title) == 0) {
        printf("Error! The title of the book cannot be empty.\n");
        return;
    }

    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strlen(author) - 1] = '\0';
    if (strlen(author) == 0) {
        printf("Error! The author of the book cannot be empty.\n");
        return;
    }

    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strlen(genre) - 1] = '\0';
    if (strlen(genre) == 0) {
        printf("Error! The genre of the book cannot be empty.\n");
        return;
    }

    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strlen(publisher) - 1] = '\0';
    if (strlen(publisher) == 0) {
        printf("Error! The publisher of the book cannot be empty.\n");
        return;
    }

    printf("page_count=");
    scanf("%d", &page_count);
    getchar();
    if (page_count <= 0) {
        printf("Error! The page count of the book must be at least one.\n");
        return;
    }

    // Creează obiect JSON pentru detaliile cărții
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "title", title);
    json_object_set_string(object, "author", author);
    json_object_set_string(object, "genre", genre);
    json_object_set_string(object, "publisher", publisher);
    json_object_set_number(object, "page_count", page_count);

    // Serializează obiectul JSON
    char *payload[1];
    payload[0] = json_serialize_to_string_pretty(value);

    // Creează cererea POST pentru adăugarea unei cărți
    char *message = compute_post_request(SERV_HOST, BOOKS_PATH, JSON_CONTENT_TYPE,
                    payload, 1, cookies, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verifică dacă a apărut o eroare
    if (strstr(response, "error") != NULL) {
        printf("Error! Server's response was not the one expected.\n");
        return;
    }

    printf("Success! Your book is now added to the library.\n");

    // Închide conexiunea
    close_connection(sockfd);
}

void process_delete_book_command(char *cookie[], char token[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    int book_id;
    
    // Citește ID-ul cărții de la utilizator
    printf("id=");
    scanf("%d", &book_id);
    getchar();

    char BOOK_PATH[100];
    sprintf(BOOK_PATH, "%s/%d",BOOKS_PATH, book_id);

    // Creează cererea DELETE pentru ștergerea unei cărți
    char *message = compute_delete_request(SERV_HOST, BOOK_PATH, NULL,
                    cookie, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verifică dacă a apărut o eroare
    if (strstr(response, "error") != NULL) {
        printf("Error! The book id you entered is invalid.\n");
        return;
    }

    printf("Success! The book with the entered id is now deleted from the library.\n");

    // Închide conexiunea
    close_connection(sockfd);
}

int process_logout_command(char *cookies[]) {
    // Deschide conexiunea la server
    int sockfd = open_connection(SERV_HOST, SERV_PORT, AF_INET, SOCK_STREAM, 0);

    // Creează cererea GET pentru logout
    char *message = compute_get_request(SERV_HOST, LOGOUT_PATH, NULL,
                    cookies, 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Verifică dacă a apărut o eroare
    if (strstr(response, "error") != NULL) {
        printf("Error! Server's response was not the one expected.\n");
        return 0;
    }

    printf("Success! You are now logged out.\n");

    return 1;
}

int main(int argc, char *argv[])
{
    int server_is_running = 1, is_logged_in = 0, access_to_library = 0;
    char *cookies[1], token[BUFLEN];

    // Mesaje de bun venit și lista de comenzi disponibile
    printf("Welcome to your online library!\n");
    printf("You can use the following commands:\n");
    printf("1. register\n");
    printf("2. login\n");
    printf("3. enter_library\n");
    printf("4. get_books\n");
    printf("5. get_book\n");
    printf("6. add_book\n");
    printf("7. delete_book\n");
    printf("8. logout\n");
    printf("9. exit\n");

    while (server_is_running) {
        char command[100];
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0';

        // Procesarea comenzilor
        if (!strcmp(command, "register")) {
            process_register_command();
        } else if (!strcmp(command, "login")) {
            if (is_logged_in) {
                printf("Error! You are already logged in.\n");
            } else {
                is_logged_in = process_login_command(cookies);
            }
        } else if (!strcmp(command, "enter_library")) {
            if (!is_logged_in) {
                printf("Error! You must be logged in to enter the library.\n");
            } else if (access_to_library) {
                printf("Error! You already have access to the library.\n");
            } else {
                access_to_library = process_enter_library_command(cookies, token);
            }
        } else if (!strcmp(command, "get_books")) {
            if (!is_logged_in) {
                printf("Error! You are not logged in.\n");
            } else if (!access_to_library) {
                printf("Error! You don't have access to the library.\n");
            } else {
                process_get_books_command(cookies, token);
            }
        } else if (!strcmp(command, "get_book")) {
            if (!is_logged_in) {
                printf("Error! You are not logged in.\n");
            } else if (!access_to_library) {
                printf("Error! You don't have access to the library.\n");
            } else {
                process_get_book_command(cookies, token);
            }
        } else if (!strcmp(command, "add_book")) {
            if (!is_logged_in) {
                printf("Error! You are not logged in.\n");
            } else if (!access_to_library) {
                printf("Error! You don't have access to the library.\n");
            } else {
                process_add_book_command(cookies, token);
            }
        } else if (!strcmp(command, "delete_book")) {
            if (!is_logged_in) {
                printf("Error! You are not logged in.\n");
            } else if (!access_to_library) {
                printf("Error! You don't have access to the library.\n");
            } else {
                process_delete_book_command(cookies, token);
            }
        } else if (!strcmp(command, "logout")) {
            if (!is_logged_in) {
                printf("Error! You are not logged in.\n");
            } else {
                is_logged_in = !process_logout_command(cookies);
                access_to_library = is_logged_in;
            }
        } else if (!strcmp(command, "exit")) {
            break;
        } else {
            printf("Error! The command you entered is invalid.\n");
        }
    }

    return 0;
}
