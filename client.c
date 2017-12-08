#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 7820
int sock = 0;

void goToLandingScreen();
void goToLoginScreen();
void sendDataToServer(char*);
void goToSignUpScreen();
void goToExitScreen();
void goToChoiceScreen();
void goToMessageScreen();
void goToCustomerListScreen(char*);
void goToCustomerEditProfileListScreen();
void customerEditProfileScreen(char*, char*);
void goToProfileEditMessageScreen(char*);
void customerProfileScreen();
void goToAccountInfoScreen();
void goToDebitScreen();
void goToCreditScreen();

int main(int argc, char* argv[]) {

    struct sockaddr_in server_address;
    char choice1,choice2;
    char username[30], password[8];

    //Creating socket by calling socket function.
    sock = socket(AF_INET, SOCK_STREAM, 0);

    //Check whether the socket is created or not. If it is not created then display error message.
    if(sock < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    //Setting the server address value to 0's
    memset(&server_address, 0, sizeof(server_address));

    //Assigning Port number and address family to the socket.
    server_address.sin_port = htons(PORT);
    server_address.sin_family = AF_INET;

    //Check whether the server address is valid or not. If it is invalid then display the error message.
    if(inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        printf("Check the address\n");
        return -1;
    }

    //Check whether the socket is connected to the server or not. If it is not connected then display the error message.
    if(connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Cannot connect to the server \n");
        return -1;
    }

    //send data to socket
    write(sock, "Hello", 6);

    goToLandingScreen();

    printf("\n Inside Main\n");
    return 0;
}

void goToLoginScreen() 
{
    system("clear");
    char username[40];
    char password[20];
    printf("\n------------------------------------------------------------------------");
    printf("\n\t\t\t\tLogin\n");
    printf("------------------------------------------------------------------------\n\n");
    // printf("Login\n");
    char choice;
    do {
        printf("\nUsername : ");
        scanf("%s", username);
        printf("\nPassword : ");
        system("stty -echo");
        scanf("%s", password);
        printf("\n\n");
        system("stty echo");
        strcat(username, ",");
        strcat(username, password);
        sendDataToServer(username);
        goToCustomerListScreen(username);
        printf("Press X to go to home page\n");
        scanf(" %c", &choice);
    } while(choice != 'X');
}

void goToLandingScreen() 
{
    char choice;
    do {
        system("clear");
        printf("\n------------------------------------------------------------------------");
        printf("\n\t\t\tWelcome to SockBank\n");
        printf("------------------------------------------------------------------------\n\n");
        printf("Are you an existing user? (Y/N) \t OR  \t \t");
        printf("Press X to exit\n\n");
        scanf(" %c", &choice);
        fflush(stdin);
        
        switch(choice)
        {
            case 'Y':
                sendDataToServer("LoginCustomer");
                goToLoginScreen();
                break;
            case 'N':
                goToChoiceScreen(); 
                break;
        }

    } while(choice != 'X');
   
    printf("\n Outside Switch\n"); 
}

void goToChoiceScreen()
{
  char choice;
  system("clear");
  printf("\n Do you want to open an account with SockBank? (Y/N) \t OR \t");
  printf("Press X to exit\n\n");
  do
  {
   scanf(" %c",&choice);
   fflush(stdin);
   switch(choice)
   {
     case  'Y' :
                sendDataToServer("SignUpCustomer");
                goToSignUpScreen();
                break;
     case 'N' : 
                goToLandingScreen();
                break;
   }
  }while(choice!= 'X');
  
}

void sendDataToServer(char* message) 
{
    write(sock, message, strlen(message)+1);
}

void goToSignUpScreen()
{
   system("clear");
   char firstName[200];
   char lastName[20];
   char dob[20];
   char address[40];
   char SSN[20];
   char contactNumber[10];
   char emailId[40];
   char accountType[10];
   char password[20];
   char choice;
  
   printf("\n------------------------------------------------------------------------");
   printf("\n\t\t\t\tSignUp\n");
   printf("------------------------------------------------------------------------\n\n");
   printf("\nPlease enter the following details\n");
   printf("\nFirst Name : ");
   scanf("%s", firstName);
   printf("\nLast Name : ");
   scanf("%s", lastName);
   printf("\nDate of birth (yyyy-mm-dd) :");
   scanf("%s", dob);
   printf("\nAddress :");
   scanf(" %[^\n]",address);
   printf("\nSSN (enter NA if not applicable) :");
   scanf("%s", SSN);
   printf("\nContact Number :");
   scanf("%s",contactNumber);
   printf("\nEmail Id (this will be your username) :");
   scanf("%s",emailId);
   printf("\nAccount Type (Savings/Checking):");
   scanf("%s",accountType);
   printf("\nPassword :");
   system("stty -echo");
   scanf("%s",password);
   system("stty echo");
   strcat(firstName, ",");
   strcat(firstName, lastName);
   strcat(firstName, ",");
   strcat(firstName, dob);
   strcat(firstName, ",");
   strcat(firstName, address);
   strcat(firstName, ",");
   strcat(firstName, SSN);
   strcat(firstName, ",");
   strcat(firstName, contactNumber);
   strcat(firstName, ",");
   strcat(firstName, emailId);
   strcat(firstName, ",");
   strcat(firstName, accountType);
   strcat(firstName, ",");
   strcat(firstName, password);
   sendDataToServer(firstName);
   goToMessageScreen();
   goToLandingScreen();
}

void goToMessageScreen()
{
  system("clear");
  char choice;
  
  do
  {
   printf("\n-----------------------------------------------------------------------------------\n");
   printf("\n \t\t\t Thank you for choosing SockBank!\n");
   printf("\n \tYou will receive an email regarding further procedure in 5-7 business days.\n\n");
   printf("\n-----------------------------------------------------------------------------------\n");
   printf("\nPress X to go to homescreen\n");
   scanf(" %c",&choice);
   fflush(stdin);
  }while(choice != 'X');
}

void goToCustomerListScreen(char* user) {
    
    
    
    char choice;
    do {
        system("clear");
        printf("\n------------------------------------------------------------------------");
        printf("\n\t\t\tWelcome to SockBank\n");
        printf("------------------------------------------------------------------------\n\n");
        printf("Please choose from the below options:\n\n");
        printf("A. View Profile\n");
        printf("B. Edit Profile\n");
        printf("C. Account Information\n");
        printf("D. Money Transfer\n");
        printf("E. Debit Money\n");
        printf("F. Credit Money\n\n");
        printf("Press X to exit\n\n");
        
        scanf(" %c", &choice);
        fflush(stdin);
        switch(choice)
        {
            case 'A':
                //sendDataToServer(user);
                customerProfileScreen();
                
                break;
                
            case 'B':
                sendDataToServer("EditProfile");
                goToCustomerEditProfileListScreen(user);

                break;
            
            case 'C':
                //sendDataToServer("AccountInfo");
                goToAccountInfoScreen();
                
                break;
                
            case 'D':
                //sendDataToServer("MoneyTransfer");
                goToLoginScreen();
                
                break;
                
            case 'E':
                //sendDataToServer("MoneyTransfer");
                goToDebitScreen();
                
                break;

            case 'F':
                //sendDataToServer("MoneyTransfer");
                goToCreditScreen();
                
                break;

            case 'N':
                printf("Do you want something?\n");
                break;
        }
        
    } while(choice != 'X');
}

void customerProfileScreen() {
    
    system("clear");
    
    printf("\n------------------------------------------------------------------------");
    printf("\n\t\t\tWelcome to SockBank\n");
    printf("------------------------------------------------------------------------\n\n");
    fflush(stdin);
    
    char buffer[2048];
    memset(&buffer[0], 0, sizeof(buffer));
    int n = read(sock, buffer, sizeof(buffer));
    // write(1, buffer, n);  //values received from server
    // write(1, "\n", 1);
    
    const char s[2] = ",";
    char *token;
    
    /* get the first token */
    token = strtok(buffer, s);
    char data[10][20];
    strcpy(data[0],token);
    
    /* walk through other tokens */
    int i = 1;
    while (token != NULL) {
        //printf("%s\n", token);
        token = strtok(NULL, s);
        if (token)
            strcpy(data[i], token);
        i++;
    }
    
    char name[50];
    strcat(data[0]," ");
    strcpy(name,data[0]);
    strcat(name,data[1]);
    
    // printf("HIIIII\n");
    
    // printf("%s",data[1]);
    
    
    char choice;
    do {
        printf("Name          : %s",name);
        printf("\nDate of Birth : %s",data[2]);
        printf("\nAddress       : %s",data[3]);
        printf("\nContact Number: %s",data[4]);
        printf("\nEmail ID      : %s",data[5]);
        printf("\nAccount Number: %s",data[6]);
        printf("\nAccount Type  : %s",data[7]);
        printf("\n\n");
        printf("Press X to go to home page\n");
        scanf(" %c", &choice);
    } while(choice != 'X');
}

void goToCustomerEditProfileListScreen(char* user) {
    
    char data[20];
    char choice;
    do {
        system("clear");
        printf("\n------------------------------------------------------------------------");
        printf("\n\t\t\tWelcome to SockBank\n");
        printf("------------------------------------------------------------------------\n\n");
        printf("Please choose from the below option that you want to edit:\n\n");
        printf("A. Email ID\n");
        printf("B. Contact\n");
        printf("C. Address\n");
        printf("\n\nPress X to exit\n\n");
        
        scanf(" %c", &choice);
        fflush(stdin);
        switch(choice)
        {
            case 'A':
                //sendDataToServer(user);
                strcpy(data, "Email");
                customerEditProfileScreen(user,data);
                
                break;
                
            case 'B':
                //sendDataToServer("AccountInfo");
                strcpy(data, "Contact");
                customerEditProfileScreen(user,data);

                break;
            
            case 'C':
                //sendDataToServer("AccountInfo");
                strcpy(data, "Address");
                customerEditProfileScreen(user,data);
                
                break;
                
            case 'N':
                printf("Do you want something?\n");
                break;
        }
        
    } while(choice != 'X');
}



void customerEditProfileScreen(char* user,char* data) {

   system("clear");
   char detail[200];
   //char choice;
  
   system("clear");
   printf("\n------------------------------------------------------------------------");
   printf("\n\t\t\tWelcome to SockBank\n");
   printf("------------------------------------------------------------------------\n\n");
   printf("Please enter the updated detail:\n\n");
   if(!strcmp(data, "Email")) {
    printf("\nEmail ID : ");
    scanf("%s", detail);
    strcat(detail, ",");
    strcat(detail,data);
    strcat(detail, ",");
    strcat(detail,user);
   }
   else if(!strcmp(data, "Contact")) {
    printf("\nContact : ");
    scanf("%s", detail);
    strcat(detail, ",");
    strcat(detail,data);
    strcat(detail, ",");
    strcat(detail,user);
   }
   if(!strcmp(data, "Address")) {
    printf("\nAddress : ");
    scanf(" %[^\n]",detail);
    strcat(detail, ",");
    strcat(detail,data);
    strcat(detail, ",");
    strcat(detail,user);
   }
   sendDataToServer(detail);
   goToProfileEditMessageScreen(user);
   goToCustomerEditProfileListScreen(user);
}

void goToProfileEditMessageScreen(char* user)
{
  system("clear");
  char choice;
  do
  {
   printf("\n-----------------------------------------------------------------------------------\n");
   printf("\n \tYour profile is updated successfully. Thank you.\n\n");
   printf("\n-----------------------------------------------------------------------------------\n");
   printf("\nPress X to go to homescreen\n");
   //sleep(5);
   //goToCustomerListScreen(user);
   scanf(" %c",&choice);
   fflush(stdin);
  }while(choice != 'X');
}

void goToExitScreen()
{
    system("clear"); 
    printf("\n----------------------------------------------------------------------------\n"); 
    printf("\n\t\t\t THANK YOU! \t\t\n\n");
    printf("----------------------------------------------------------------------------\n\n"); 
}

void goToAccountInfoScreen() {

    system("clear");
    printf("\n------------------------------------------------------------------------");
    printf("\n\t\t\tWelcome to SockBank\n");
    printf("------------------------------------------------------------------------\n\n");
    printf("Account Number: \n");
    printf("Account Type  : \n");

}

void goToDebitScreen() {

    system("clear");
    printf("\n------------------------------------------------------------------------");
    printf("\n\t\t\tWelcome to SockBank\n");
    printf("------------------------------------------------------------------------\n\n");
    printf("Enter the amount to be debited: \n\n\n");
    printf("The amount is successfully debited.\n");

}

void goToCreditScreen() {
    
        system("clear");
        printf("\n------------------------------------------------------------------------");
        printf("\n\t\t\tWelcome to SockBank\n");
        printf("------------------------------------------------------------------------\n\n");
        printf("Enter the amount to be credited: \n\n\n");
        printf("The amount is successfully credited.\n");
    
    }