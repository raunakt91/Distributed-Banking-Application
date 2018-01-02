#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include </usr/local/include/mysql/mysql.h>
#include </usr/local/include/mysql/my_global.h>

void finish_with_error(MYSQL *con);

typedef struct 
{
    pthread_t thread_tid;      /* thread ID */
    long    thread_count;      
 }  Thread;

int login_signup(int newsockfd);



/*
 *  sockfd is the socket descriptor which is an integer value
 *  client_address is the struct used to specify a local or remote endpoint address of client.
 */
Thread *tptr;                  
socklen_t addrlen;
pthread_mutex_t mlock;
void thread(int);
struct sockaddr_in serv_addr;
int sockfd, i, nthreads;
int address_len = sizeof(serv_addr);
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{    
    nthreads = atoi(argv[argc - 1]);
    tptr = calloc(nthreads, sizeof(Thread));
    
    /*  Socket() call takes three arguments:
    *  The family of protocol used/address family
    *  Type of socket
    *  Protocol number or 0 for a given family and type
        */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*
     *  Socket call will return a socket descriptor on success which is an integer
     *  And it will return '-1' for error
     */
    if (sockfd == -1)
    {
        printf("Error calling Socket\n");
        exit(1);
    }
    /* Populating the sockaddr_in struct with the following values */
    /* Assigning the AF_INET (Internet Protocol v4) address family */
    serv_addr.sin_family = AF_INET;
    /* Populating the Server IP address with the value of the localhost IP address */
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Converting the port number received from the command line from host byte order to network byte order */
    serv_addr.sin_port = htons(8000);

    /* to set the socket options- to reuse the given port multiple times */
    int num=1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,(const char*)&num,sizeof(num))<0)
    {
        printf("setsockopt(SO_REUSEPORT) failed\n");
        exit(0);
    }
    
    /*
     *  Bind takes three arguments: - Used to bind the local endpoint parameters to the socket
     *  Socket descriptor
     *  Server Address Structure - Local endpoint in this case
     *  Size of the address
     */
    /* Returns 0 for success and -1 for failure */
    if(bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("Error binding\n");
        exit(1);
    }
    
    /* Listen call makes the socket to be a passive type, taking two arguments:
     *  Socket Descriptor
     *  number of connection that can be queued
     */
    if(listen(sockfd,5) < 0)
    {
        printf("Error listening\n");
        exit(1);
    }
    
    /* nthreads number of threads are created as specified on command line */
    for(i=0; i<nthreads; i++)
    {
        thread(i);
    }
    for ( ; ; )       
        pause();   
}             

void thread(int i)
{
    void *thread_main(void *);
    /* new threads are created to handle file transfer using pthread_create(0 call */
    pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *)(uintptr_t) i);
    return;
}

void *thread_main(void *arg)
{
     int     newsockfd;
     void    web_child(int);
     printf("Thread %d starting\n", (int) arg);
     for ( ; ; ) 
     {
         
        pthread_mutex_lock(&mlock);
         
         
         
        /* server accepts incoming conenction on sockfd and assigns it to newsockfd */
        /*   Accept takes three arguments:
          *  Socket descriptor
          *  Server Address Structure - Local endpoint in this case
          *  Size of the address
        */
        newsockfd = accept(sockfd, (struct sockaddr *) &serv_addr,(socklen_t*)&address_len);
        pthread_mutex_unlock(&mlock);
        tptr[(int) arg].thread_count++;
        
        login_signup(newsockfd);
        
        close(newsockfd);
    }
}

void finish_with_error(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
}


int login_signup(int newsockfd)
{
    //Connecting to database.....
    MYSQL *con = mysql_init(NULL);
    if (con == NULL)
    {
        fprintf(stderr, "mysql_init() failed\n");
        exit(1);
    }
    
    if (mysql_real_connect(con, "localhost", "root", "root","CMPE_207", 0, NULL, 0) == NULL)
    {
        finish_with_error(con);
    }
    //........................
    
    char LoginDetails[20];
    char signupdetails[500];
    int flag;
    recv(newsockfd, &flag, sizeof(int), 0);
    
    if(flag == 1)
    {
        recv(newsockfd,LoginDetails,sizeof(LoginDetails),0);
        char *LoginID = strtok(LoginDetails," ");
        char *Password = strtok(NULL,"\n");
        printf("\nLoginID: %s\nPassword: %s\n",LoginID,Password);
        
        char query[300];
        sprintf(query,"SELECT Role FROM Login WHERE Login_ID = '%s' AND Password = '%s'",LoginID,Password);
        if (mysql_query(con, query))
        {
            printf("\nin query\n");
            finish_with_error(con);
        }
        
        MYSQL_RES *result = mysql_store_result(con);
        if (result == NULL)
        {
            finish_with_error(con);
        }
        
        int num_fields = mysql_num_fields(result);
        //printf("Fields-%d\n",num_fields);
        MYSQL_ROW row;
        char* send_buf[10];
        int count = 0;
        while ((row = mysql_fetch_row(result)))
        {
            count = 1;
            for(int i = 0; i < num_fields; i++)
            {
                send_buf[i] = row[i];
                //printf("%s",row[i]);
                printf("\n%s LOGIN AUTHENTICATED\n", send_buf[i]);
                send(newsockfd, send_buf[i], sizeof(send_buf), 0);
            }
        }
        if (count == 0)
        {
            char message[] = "INVALID";
            send(newsockfd,message,sizeof(message),0);
            login_signup(newsockfd);
        }
        char option[5];
        while(recv(newsockfd,option,sizeof(option),0)>0)
        {
            printf("\nOption received - %s for user - %s\n", option, LoginID);
            if(strcmp(send_buf[0],"CUSTOMER") == 0)
            {
                if(strcmp(option,"1") == 0)
                {
                    char query[300];
                    sprintf(query,"SELECT * FROM LOGIN WHERE Login_ID = '%s'",LoginID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    
                    int num_fields = mysql_num_fields(result);
                    MYSQL_ROW row;
                    char* send_buf[10];
                    char send_buffer[100];
                    bzero(send_buffer,sizeof(send_buffer));
                    printf("Login_ID\tEmail_ID\tPassword\tRole\t\tFull_Name\tPhone_Number\tAddress\n");
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            printf("%s\t",send_buf[i]);
                            if(i != 0)
                            {
                                strcat(send_buffer,"\t");
                            }
                            if(i == 1)
                            {
                                strcat(send_buffer,"\t");
                            }
                            strcat(send_buffer,send_buf[i]);
                        }
                        send(newsockfd, send_buffer, sizeof(send_buffer), 0);
                        memset(send_buf, 0, sizeof(send_buf));
                        memset(&send_buffer[0], 0, sizeof(send_buffer));
                        //char send_buffer[100] = {0};
                        //bzero(send_buffer,100);
                        printf("\n");
                    }
                    //mysql_close(con);
                }
                
                if(strcmp(option,"2") == 0)
                {
                    char input[20];
                    //char query[300];                 *******1
                    char out[]="Profile has been updated";
                    char id[5];
                    recv(newsockfd,id,sizeof(id),0);
                    recv(newsockfd,input,sizeof(input),0);
                    printf("Received -\t%s",input);
                    if(strcmp(id,"1") == 0)
                    {
                        sprintf(query,"UPDATE Login SET Email_ID='%s' WHERE Login_ID = '%s'",input,LoginID);
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                    }
                    if(strcmp(id,"2") == 0)
                    {
                        sprintf(query,"UPDATE Login SET Password='%s' WHERE Login_ID = '%s'",input,LoginID);
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                    }
                    if(strcmp(id,"3") == 0)
                    {
                        sprintf(query,"UPDATE Login SET Phone_Number='%s' WHERE Login_ID = '%s'",input,LoginID);
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                    }
                    if(strcmp(id,"4") == 0)
                    {
                        sprintf(query,"UPDATE Login SET Address='%s' WHERE Login_ID = '%s'",input,LoginID);
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                    }
                    send(newsockfd,out,sizeof(out),0);
                }
                
                if(strcmp(option,"3") == 0)
                {
                    char query[300];
                    sprintf(query,"SELECT Account_Balance FROM Accounts WHERE Login_ID = '%s'",LoginID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    
                    int num_fields = mysql_num_fields(result);
                    MYSQL_ROW row;
                    char* send_buf[10];
                    char send_buffer[100];
                    bzero(send_buffer,sizeof(send_buffer));
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            printf("%s\t",send_buf[i]);
                            strcat(send_buffer,send_buf[i]);
                        }
                        send(newsockfd, send_buffer, sizeof(send_buffer), 0);
                        // memset(send_buf, 0, sizeof(send_buf));
                        // memset(&send_buffer[0], 0, sizeof(send_buffer));
                        //char send_buffer[100] = {0};
                        
                        printf("\n");
                    }
                    //mysql_close(con);
                }
                if(strcmp(option,"4") == 0)
                {
                    char ID[100], amount[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    recv(newsockfd,amount,sizeof(amount),0);
                    printf("ID - %s ---- Amount - $%s\n",ID,amount);
                    if (strcmp(LoginID,ID) != 0)
                    {
                        char query[3000];
                        sprintf(query,"SELECT Account_Balance FROM Accounts WHERE Login_ID = '%s'",LoginID);
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                        
                        MYSQL_RES *result = mysql_store_result(con);
                        if (result == NULL)
                        {
                            finish_with_error(con);
                        }
                        char* send_buf[10];
                        float initial1;
                        float deduct = atof(amount);
                        int num_fields = mysql_num_fields(result);
                        MYSQL_ROW row;
                        while ((row = mysql_fetch_row(result)))
                        {
                            for(int i = 0; i < num_fields; i++)
                            {
                                send_buf[i] = row[i];
                                initial1 = atof(send_buf[i]);
                            }
                        }
                        printf("Current balance - %f\n",initial1);
                        printf("Amount to be deducted - %f\n",deduct);
                        printf("Updated balance - %f\n",initial1-deduct);
                        float final = initial1-deduct;
                        
                        if(final >= 0)
                        {
                            sprintf(query,"UPDATE Accounts SET Account_Balance='%f' WHERE Login_ID = '%s'",(initial1-deduct),LoginID);
                            if (mysql_query(con, query))
                            {
                                printf("\nin query\n");
                                finish_with_error(con);
                            }
                            
                            sprintf(query,"SELECT Account_Balance FROM Accounts WHERE Login_ID = '%s'",ID);
                            if (mysql_query(con, query))
                            {
                                printf("\nin query\n");
                                finish_with_error(con);
                            }
                            
                            result = mysql_store_result(con);
                            if (result == NULL)
                            {
                                finish_with_error(con);
                            }
                            float initial2;
                            num_fields = mysql_num_fields(result);
                            while ((row = mysql_fetch_row(result)))
                            {
                                for(int i = 0; i < num_fields; i++)
                                {
                                    send_buf[i] = row[i];
                                    printf("***%s***\n",send_buf[i]);
                                    initial2 = atof(send_buf[i]);
                                }
                            }
                            printf("Receiver balance - %f\n",initial2);
                            sprintf(query,"UPDATE Accounts SET Account_Balance='%f' WHERE Login_ID = '%s'",(initial2+deduct),ID);
                            if (mysql_query(con, query))
                            {
                                printf("\nin query\n");
                                finish_with_error(con);
                            }
                            sprintf(query,"INSERT INTO Acc_History VALUES ('%s','%s','%f')",LoginID,ID,deduct);
                            if (mysql_query(con, query))
                            {
                                printf("\nin query\n");
                                finish_with_error(con);
                            }
                            char message[]="SUCCESSFUL";
                            send(newsockfd,message,sizeof(message),0);
                            memset(&message[0], 0, sizeof(message));
                        }
                        else
                        {
                            char message[]="UNSUCCESSFUL";
                            send(newsockfd,message,sizeof(message),0);
                            memset(&message[0], 0, sizeof(message));
                        }
                        memset(send_buf, 0, sizeof(send_buf));
                        
                        //mysql_free_result(result);
                        //mysql_close(con);
                    }
                    else
                    {
                        char message[]="INVALID ID";
                        send(newsockfd,message,sizeof(message),0);
                        memset(&message[0], 0, sizeof(message));
                    }
                }
                if(strcmp(option,"5") == 0)
                {
                    char amt[5];
                    //printf("1");
                    recv(newsockfd,amt,sizeof(amt),0);
                    //printf("2");
                    char* send_buf[10];
                    char query[300];
                    sprintf(query,"SELECT Account_Balance FROM Accounts WHERE Login_ID = '%s'",LoginID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    float initial3;
                    float deduct1 = atof(amt);
                    int num_fields = mysql_num_fields(result);
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            initial3 = atof(send_buf[i]);
                        }
                    }
                    float final = initial3-deduct1;
                    printf("Current balance - %f\n",initial3);
                    printf("Amount to be deducted - %f\n",deduct1);
                    printf("Updated balance - %f\n",final);
                    if(final >= 0)
                    {
                        sprintf(query,"UPDATE Accounts SET Account_Balance='%f' WHERE Login_ID = '%s'",(final),LoginID);
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                    }
                    else
                    {
                        char message[] = "INSUFFICIENT BALANCE";
                        send(newsockfd,message,sizeof(message),0);
                    }
                    memset(&amt[0], 0, sizeof(amt));
                    //memset(&send_buf, 0, sizeof(send_buf));
                    //mysql_free_result(result);
                    //mysql_close(con);
                }
                
                if(strcmp(option,"6") == 0)
                {
                    printf("\nSent transactions\n");
                    char query[300];
                    char* send_buf[10];
                    char send_buffer1[1000];
                    bzero(send_buffer1,sizeof(send_buffer1));
                    char type_sent[]="Sent";
                    sprintf(query,"SELECT * FROM Acc_History WHERE Sender = '%s' or Customer = '%s'",LoginID,LoginID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    int num_fields = mysql_num_fields(result);
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            printf("%s\t",send_buf[i]);
                            if(i != 0)
                            {
                                strcat(send_buffer1,"\t\t");
                            }
                            strcat(send_buffer1,send_buf[i]);
                        }
                        printf("\n");
                        strcat(send_buffer1,"\n");
                    }
                    send(newsockfd, send_buffer1, sizeof(send_buffer1), 0);
                }
                if(strcmp(option,"7") == 0)
                {
                    login_signup(newsockfd);
                }
                
            }
            if(strcmp(send_buf[0],"TELLER") == 0)
            {
                if(strcmp(option,"1") == 0)
                {
                    recv(newsockfd,signupdetails,sizeof(signupdetails),0);
                    printf("\nThe signup details are - %s\n\n", signupdetails);
                    char login[100], email[100], role[100], password[100], fname[100], pnumber[100], address[100];
                    char s[2] = " ";
                    recv(newsockfd,fname,sizeof(fname),0);
                    strcpy(email, strtok(signupdetails, s));
                    strcpy(password, strtok(NULL, s));
                    strcpy(pnumber, strtok(NULL, s));
                    
                    strcpy(role,"CUSTOMER");
                    recv(newsockfd,address,sizeof(address),0);
                    
                    strcpy(login,"C");
                    char initial[100];
                    int x;
                    srand (time(NULL));
                    x = (rand()%2999)+7000;
                    sprintf(initial, "%d", x);
                    strcat(login,initial);
                    
                    printf("\nCredentials received from client are:\n\nLogin_ID : %s \nEmail_ID : %s \nPassword : %s \nRole : %s\nName : %s\nPhone :  %s\nAddress :  %s",login,email,password,role,fname,pnumber,address);
                    char statement[1000];
                    sprintf(statement, "INSERT INTO Login VALUES('%s','%s','%s','%s','%s','%s','%s')", login,email,password,role,fname,pnumber,address);
                    if (mysql_query(con, statement))
                    {
                        finish_with_error(con);
                    }
                    char query1[300];
                    sprintf(query1, "INSERT INTO Accounts VALUES('%s' , '0')", login);
                    if (mysql_query(con, query1))
                    {
                        finish_with_error(con);
                    }
                    char command[300];
                    sprintf(command, "echo 'Your request for new Account has been received. Please note the Login ID %s for future correspondence. You can now login to your account. ' | mail -s 'Confirmation Email' %s", login, email);
                    system(command);
                    char message[]="Account Created";
                    send(newsockfd,message,sizeof(message),0);
                }
                
                if(strcmp(option,"2") == 0)
                {
                    char ID[100], amount[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    recv(newsockfd,amount,sizeof(amount),0);
                    printf("ID - %s ---- Amount - $%s\n",ID,amount);
                    char query[300];
                    sprintf(query,"SELECT Account_Balance FROM Accounts WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    float initial1;
                    float add = atof(amount);
                    int num_fields = mysql_num_fields(result);
                    char* send_buf[10];
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            initial1 = atof(send_buf[i]);
                        }
                    }
                    printf("Current balance - %f\n",initial1);
                    printf("Amount to be deducted - %f\n",add);
                    printf("Updated balance - %f\n",initial1+add);
                    sprintf(query,"UPDATE Accounts SET Account_Balance='%f' WHERE Login_ID = '%s'",(initial1+add),ID);
                    char message[] = "SUCCESSFUL";
                    send(newsockfd,message,sizeof(message),0);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    sprintf(query,"INSERT INTO Acc_History VALUES ('%s','%s','%f')",LoginID,ID,add);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    //mysql_close(con);
                }
                
                if(strcmp(option,"3") == 0)
                {
                    char ID[100], amount[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    recv(newsockfd,amount,sizeof(amount),0);
                    printf("ID - %s ---- Amount - $%s\n",ID,amount);
                    char query[300];
                    sprintf(query,"SELECT Account_Balance FROM Accounts WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    float initial1;
                    float deduct = atof(amount);
                    int num_fields = mysql_num_fields(result);
                    char* send_buf[10];
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            initial1 = atof(send_buf[i]);
                        }
                    }
                    printf("Current balance - %f\n",initial1);
                    printf("Amount to be deducted - %f\n",deduct);
                    printf("Updated balance - %f\n",initial1-deduct);
                    sprintf(query,"UPDATE Accounts SET Account_Balance='%f' WHERE Login_ID = '%s'",(initial1-deduct),ID);
                    char message[] = "SUCCESSFUL";
                    send(newsockfd,message,sizeof(message),0);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    sprintf(query,"INSERT INTO Acc_History VALUES ('%s','%s','%f')",ID,LoginID,deduct);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                }
                
                if(strcmp(option,"4") == 0)
                {
                    char ID[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    char query[300];
                    sprintf(query,"SELECT * FROM LOGIN WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    
                    int num_fields = mysql_num_fields(result);
                    MYSQL_ROW row;
                    char* send_buf[10];
                    char send_buffer[100];
                    printf("Login_ID\tEmail_ID\tPassword\tRole\t\tFull_Name\tPhone_Number\tAddress\n");
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            printf("%s\t",send_buf[i]);
                            if(i != 0)
                            {
                                strcat(send_buffer,"\t");
                            }
                            if(i == 1)
                            {
                                strcat(send_buffer,"\t");
                            }
                            strcat(send_buffer,send_buf[i]);
                        }
                        send(newsockfd, send_buffer, sizeof(send_buffer), 0);
                        //memset(send_buf, 0, sizeof(send_buf));
                        memset(&send_buffer[0], 0, sizeof(send_buffer));
                        //char send_buffer[100] = {0};
                        //bzero(send_buffer,100);
                        printf("\n");
                    }
                }
                
                if(strcmp(option,"5") == 0)
                {
                    char ID[100], transfer[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    
                    char query[300];
                    sprintf(query,"SELECT Email_ID FROM Login WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    int num_fields = mysql_num_fields(result);
                    char* send_buf[100];
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            strcpy(transfer,send_buf[i]);
                        }
                    }
                    printf("Email ID fetched from the Database is %s", transfer);
                    
                    char command[10000];
                    
                    sprintf(command, "echo 'your request for new cheque book has been successfully confirmed' | mail -s 'Confirmation Email' %s", transfer);
                    system(command);
                    send(newsockfd,transfer,sizeof(transfer),0);
                    
                }
                if(strcmp(option,"6") == 0)
                {
                    char query2[300];
                    sprintf(query2,"SELECT * FROM Validate_login");
                    if (mysql_query(con, query2))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result2 = mysql_store_result(con);
                    if (result2 == NULL)
                    {
                        finish_with_error(con);
                    }
                    
                    int num_fields2 = mysql_num_fields(result2);
                    MYSQL_ROW row2;
                    char* send_buf2[10];
                    char* test[5] = {0};
                    char* test1[5] = {0};
                    int j = 0;
                    int m = 0;
                    char send_buffer2[1000];
                    printf("Login_ID\tEmail_ID\tPassword\tRole\t\tFull_Name\tPhone_Number\tAddress\n");
                    while ((row2 = mysql_fetch_row(result2)))
                    {
                        for(int i = 0; i < num_fields2; i++)
                        {
                            send_buf2[i] = row2[i];
                            test[j] = send_buf2[0];
                            test1[m] = send_buf2[1];
                            
                            printf("%s\t",send_buf2[i]);
                            fflush(stdout);
                            if(i != 0)
                            {
                                strcat(send_buffer2,"\t");
                            }
                            if(i == 1)
                            {
                                strcat(send_buffer2,"\t");
                            }
                            strcat(send_buffer2,send_buf2[i]);
                        }
                        j++;
                        m++;
                        strcat(send_buffer2,"\n");
                    }
                    printf("email - %s",test1[0]);
                    fflush(stdout);
                    send(newsockfd, send_buffer2, sizeof(send_buffer2), 0);
                    char approval[5];
                    recv(newsockfd,approval,sizeof(approval),0);
                    if(strcmp(approval,"Y")==0)
                    {
                        char query[30000];
                        sprintf(query,"INSERT INTO Login SELECT * FROM Validate_login");
                        if (mysql_query(con, query))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                        
                        char query2[30000];
                        sprintf(query2,"DELETE FROM Validate_login");
                        if (mysql_query(con, query2))
                        {
                            printf("\nin query\n");
                            finish_with_error(con);
                        }
                        int k = 0;
                        while(test[k])
                        {
                            if(strcmp(test[k],"0") != 0)
                            {
                                char query3[30000];
                                sprintf(query3,"INSERT INTO Accounts VALUES('%s','0')",test[k]);
                                if (mysql_query(con, query3))
                                {
                                    printf("\nin query\n");
                                    finish_with_error(con);
                                }
                                
                                char command[10000];
                                sprintf(command, "echo 'Congratulations! Your account has been approved. You will now be able to login using the Login ID sent to you in previous email communications.' | mail -s 'Account Approval Email' %s ", test1[k]);
                                system(command);
                            }
                            k++;
                        }
                    }
                    
                }
                if(strcmp(option,"7") == 0)
                {
                    login_signup(newsockfd);
                }
            }
            if(strcmp(send_buf[0],"ADMIN") == 0)
            {
                if(strcmp(option,"1") == 0)
                {
                    char ID[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    char query[300];
                    sprintf(query,"SELECT * FROM LOGIN WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    
                    int num_fields = mysql_num_fields(result);
                    MYSQL_ROW row;
                    char* send_buf[10];
                    char send_buffer[100];
                    printf("Login_ID\tEmail_ID\tPassword\tRole\t\tFull_Name\tPhone_Number\tAddress\n");
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            printf("%s\t",send_buf[i]);
                            if(i != 0)
                            {
                                strcat(send_buffer,"\t");
                            }
                            if(i == 1)
                            {
                                strcat(send_buffer,"\t");
                            }
                            strcat(send_buffer,send_buf[i]);
                        }
                        send(newsockfd, send_buffer, sizeof(send_buffer), 0);
                        //memset(send_buf, 0, sizeof(send_buf));
                        memset(&send_buffer[0], 0, sizeof(send_buffer));
                        //char send_buffer[100] = {0};
                        //bzero(send_buffer,100);
                        printf("\n");
                    }
                }
                
                if(strcmp(option,"2") == 0)
                {
                    recv(newsockfd,signupdetails,sizeof(signupdetails),0);
                    
                    printf("\nThe signup details are - %s\n\n", signupdetails);
                    
                    char login[100], email[100], role[100], password[100], fname[100], pnumber[100], address[100];
                    char s[2] = " ";
                    
                    recv(newsockfd,fname,sizeof(fname),0);
                    
                    strcpy(login, strtok(signupdetails, s));
                    strcpy(email, strtok(NULL, s));
                    strcpy(password, strtok(NULL, s));
                    // strcpy(role, strtok(NULL, s));
                    strcpy(pnumber, strtok(NULL, s));
                    strcpy(role, "TELLER");
                    
                    recv(newsockfd,address,sizeof(address),0);
                    printf("\nCredentials received from client are:\n\nLogin_ID : %s \nEmail_ID : %s \nPassword : %s \nRole : %s\n name: %s\n phone %s\n address %s",login,email,password,role,fname,pnumber,address);
                    char statement[1000];
                    sprintf(statement, "INSERT INTO Login VALUES('%s','%s','%s','%s','%s','%s','%s')", login,email,password,role,fname,pnumber,address);
                    if (mysql_query(con, statement))
                    {
                        finish_with_error(con);
                    }
                }
                
                if(strcmp(option,"3") == 0)
                {
                    char ID[100];
                    char email[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    
                    
                    char query[300];
                    sprintf(query,"SELECT Email_ID FROM Login WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    int num_fields = mysql_num_fields(result);
                    char* send_buf[100];
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            strcpy(email,send_buf[i]);
                        }
                    }
                    printf("Email ID fetched from the Database is %s", email);
                    
                    
                    char query1[300];
                    sprintf(query1,"DELETE FROM Login WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query1))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    send(newsockfd,email,sizeof(email),0);
                }
                
                if(strcmp(option,"4") == 0)
                {
                    char ID[100];
                    char email[100];
                    recv(newsockfd,ID,sizeof(ID),0);
                    
                    
                    char query[300];
                    sprintf(query,"SELECT Email_ID FROM Login WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    MYSQL_RES *result = mysql_store_result(con);
                    if (result == NULL)
                    {
                        finish_with_error(con);
                    }
                    int num_fields = mysql_num_fields(result);
                    char* send_buf[100];
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(result)))
                    {
                        for(int i = 0; i < num_fields; i++)
                        {
                            send_buf[i] = row[i];
                            strcpy(email,send_buf[i]);
                        }
                    }
                    printf("Email ID fetched from the Database is %s", email);
                    
                    
                    char query1[300];
                    sprintf(query1,"DELETE FROM Login WHERE Login_ID = '%s'",ID);
                    if (mysql_query(con, query1))
                    {
                        printf("\nin query\n");
                        finish_with_error(con);
                    }
                    send(newsockfd,email,sizeof(email),0);
                }
                if(strcmp(option,"5") == 0)
                {
                    login_signup(newsockfd);
                }
            }
        }
        mysql_free_result(result);
    }
    else if(flag == 2)
    {
        recv(newsockfd,signupdetails,sizeof(signupdetails),0);
        printf("\nThe signup details are - %s\n\n", signupdetails);
        char login[100], email[100], role[100], password[100], fname[100], pnumber[100], address[100];
        char s[2] = " ";
        recv(newsockfd,fname,sizeof(fname),0);
        strcpy(email, strtok(signupdetails, s));
        strcpy(password, strtok(NULL, s));
        strcpy(pnumber, strtok(NULL, s));
        
        strcpy(role,"CUSTOMER");
        recv(newsockfd,address,sizeof(address),0);
        
        strcpy(login,"C");
        char initial[100];
        int x;
        srand (time(NULL));
        x = (rand()%2999)+7000;
        //initial = itoa(x);
        sprintf(initial, "%d", x);
        strcat(login,initial);
        printf("\nCredentials received from client are:\n\nLogin_ID : %s \nEmail_ID : %s \nPassword : %s \nRole : %s\nName : %s\nPhone :  %s\nAddress :  %s",login,email,password,role,fname,pnumber,address);
        char statement[1000];
        sprintf(statement, "INSERT INTO Validate_login VALUES('%s','%s','%s','%s','%s','%s','%s')", login,email,password,role,fname,pnumber,address);
        if (mysql_query(con, statement))
        {
            finish_with_error(con);
        }
        char command[10000];
        
        sprintf(command, "echo 'Your request for new Account has been received. Please note the Login ID %s for future correspondence. A Teller or Admin will review the profile information and you will receive another email regarding your account update. ' | mail -s 'Confirmation Email' %s", login, email);
        system(command);
        send(newsockfd,email,sizeof(email),0);
        login_signup(newsockfd);
    }
    mysql_close(con);
    return 0;
}
