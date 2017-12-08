#include <stdio.h>
#include <mysql.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 7820
#define MAX 5

int new_socket;
void processLoginRequest();
void processSignUpRequest();
void editCustomerProfile();
void viewCustomerProfile(char*);

MYSQL* connectToDatabase() 
{

    MYSQL *conn;
    char *server = "localhost";
    char *user = "root";
    char *password = "root"; /* set me first */
    char *database = "Distributed_Banking_System";

    conn = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn, server,
                            user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    return conn;
}

void closeDatabaseConnection(MYSQL *conn) 
{
    mysql_close(conn);
}

int main(int argc, char** argv) 
{
    int file_descriptor_server;
    struct sockaddr_in server_address;
    int address_length = sizeof(server_address);
    char buffer[2048];

    if(argc != 1) 
    {
        printf("Invalid number of arguments.\nPlease do not enter any arguments for server.\n");
        return 0;
    }

    //Creating socket by calling socket function.
    file_descriptor_server = socket(AF_INET, SOCK_STREAM, 0);

    //Check whether the socket is created or not. If it is not created then display error message.
    if(file_descriptor_server < 0) 
    {
        perror("Cannot create a socket");
        exit(EXIT_FAILURE);
    }

    //Assigning the address family, port number and server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    //Check whether the socket is bind to the server address. It it is not binded to it then display error message
    if(bind(file_descriptor_server, (struct sockaddr*)&server_address,address_length) < 0 ) {
        perror("Cannot bind to port");
        exit(EXIT_FAILURE);
    }

    //Listen for incoming requests on the listening socket, and display an error message if the listening queue becomes full.
    if(listen(file_descriptor_server, MAX) < 0) {
        perror("Queue may be full");
        exit(EXIT_FAILURE);
    }

    while(1) {

        //Accepting a client's connection request and creating a new connected socket to handle the interaction with the client.
        new_socket = accept(file_descriptor_server, (struct sockaddr *) &server_address, (socklen_t *) &address_length);

        //Check whether the client's connection request was accepted, if it was not accepted then display an error message.
        if (new_socket < 0) {
            perror("Cannot accept connection");
            exit(EXIT_FAILURE);
        }

        printf("Server started\n");
        int n = read(new_socket, buffer, sizeof(buffer));
        write(1, buffer, n);
        write(1, "\n", 1);


        int flag = 0;

        while (1) 
        {
            
            n = read(new_socket, buffer, sizeof(buffer));
            write(1, buffer, n);  //prints the operation customer has requested
            write(1, "\n", 1);

            if(!strcmp(buffer, "LoginCustomer")) 
            {
                processLoginRequest();
            }
            else if(!strcmp(buffer, "SignUpCustomer")) 
            {   
                processSignUpRequest();
            }
            else if(!strcmp(buffer, "EditProfile"))
            {
                editCustomerProfile();
            }
            else if(!strcmp(buffer, "AllDone")) 
            {
                break;
            }
        }
    }

    return 0;
}

void processLoginRequest() {
    
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    
    conn = connectToDatabase();
    
    char buffer[2048];
    int n = read(new_socket, buffer, sizeof(buffer));
    write(1, buffer, n);
    write(1, "\n", 1);
    
    const char s[2] = ",";
    char *token;
    
    /* get the first token */
    token = strtok(buffer, s);
    char data[2][20];
    strcpy(data[0],token);
    
    /* walk through other tokens */
    int i = 1;
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, s);
        if (token)
            strcpy(data[i], token);
        i++;
    }
    
    char query[200];
    
    strcpy(query, "select * from login WHERE username = '");
    strcat(query, data[0]);
    strcat(query, "'");
    printf("Query - %s\n", query);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    
    res = mysql_use_result(conn);
    if (res) {
        row = mysql_fetch_row(res);
        if (row) {
            printf("%s \n", row[1]);
            if (!strcmp(data[1], row[1])) {
                printf("Valid login credentials\n");
                if(!strcmp(row[2], "customer")) {
                    viewCustomerProfile(row[0]);
                }
            } else {
                printf("Invalid login credentials\n");
            }
        } else {
            printf("User not present in the system\n"); }
        mysql_free_result(res);
    }
    closeDatabaseConnection(conn);
}

void processSignUpRequest()
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = connectToDatabase();

    char buffer[2048];
    memset(&buffer[0], 0, sizeof(buffer)); 
    int n = read(new_socket, buffer, sizeof(buffer));
    write(1, buffer, n);  //values received from client
    write(1, "\n", 1);

    const char s[2] = ",";
    char *token;

    /* get the first token */
    token = strtok(buffer, s);
    char data[9][20];
    strcpy(data[0],token);

    /* walk through other tokens */
    int i = 1;
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, s);
        if (token)
            strcpy(data[i], token);
        i++;
    }

    char query[400];

    strcpy(query, "insert into Customer_Account_Request(first_name,last_name,dob,address,SSN,contact_number,email,account_type,password,approved) values('");
    strcat(query, data[0]);
    strcat(query,"','");
    strcat(query, data[1]);
    strcat(query,"',DATE '");
    strcat(query, data[2]);
    strcat(query, "','");
    strcat(query, data[3]);
    strcat(query, "',");
    strcat(query, data[4]);
    strcat(query, ",");
    strcat(query, data[5]);
    strcat(query, ",'");
    strcat(query, data[6]);
    strcat(query, "','");
    strcat(query, data[7]);
    strcat(query, "','"); 
    strcat(query, data[8]);
    strcat(query, "','Pending')"); 
    printf("Query - %s\n", query);
    
    if (mysql_query(conn, query)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    else
    {
      printf("\n \t\t Customer record inserted successfully!\n");
    }

    closeDatabaseConnection(conn);

}

void viewCustomerProfile(char* user) {
    
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    
    conn = connectToDatabase();
    
    char buffer[2048];
    char query[200];
    strcpy(query, "select * from Customer_Details WHERE username = '");
    strcat(query, user);
    strcat(query, "'");
    printf("Query - %s\n", query);
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    
    res = mysql_use_result(conn);
    if (res) {
        row = mysql_fetch_row(res);
        
        strcpy(buffer,row[1]);
        strcat(buffer,",");
        strcat(buffer,row[2]);
        strcat(buffer,",");
        strcat(buffer,row[3]);
        strcat(buffer,",");
        strcat(buffer,row[4]);
        strcat(buffer,",");
        strcat(buffer,row[6]);
        strcat(buffer,",");
        strcat(buffer,row[7]);
        strcat(buffer,",");
        strcat(buffer,row[8]);
        strcat(buffer,",");
        strcat(buffer,row[9]);
        
        write(new_socket,buffer,sizeof(buffer));
        
        mysql_free_result(res);
    }
    closeDatabaseConnection(conn);
}

void editCustomerProfile() {

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = connectToDatabase();

    char buffer[2048];
    memset(&buffer[0], 0, sizeof(buffer)); 
    int n = read(new_socket, buffer, sizeof(buffer));
    write(1, buffer, n);  //values received from client
    write(1, "\n", 1);

    const char s[2] = ",";
    char *token;

    /* get the first token */
    token = strtok(buffer, s);
    char data[3][20];
    strcpy(data[0],token);

    /* walk through other tokens */
    int i = 1;
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, s);
        if (token)
            strcpy(data[i], token);
        i++;
    }

    char query[400];

    if(!strcmp(data[1], "Email")) {
        strcpy(query, "UPDATE Customer_Details SET email ='");
        strcat(query, data[0]);
        strcat(query,"' WHERE username ='");
        strcat(query, data[2]);
        strcat(query,"';"); 
        printf("Query - %s\n", query);
    }
    else if(!strcmp(data[1], "Contact")) {
        strcpy(query, "UPDATE Customer_Details SET contact_number =");
        strcat(query, data[0]);
        strcat(query," WHERE username ='");
        strcat(query, data[2]);
        strcat(query,"';"); 
        printf("Query - %s\n", query);
    }
    else if(!strcmp(data[1], "Address")) {
        strcpy(query, "UPDATE Customer_Details SET address ='");
        strcat(query, data[0]);
        strcat(query,"' WHERE username ='");
        strcat(query, data[2]);
        strcat(query,"';"); 
        printf("Query - %s\n", query); 
    }

    
    if (mysql_query(conn, query)) 
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    else
    {
      printf("\n \t\t Customer record updated successfully!\n");
    }

    closeDatabaseConnection(conn);

}
