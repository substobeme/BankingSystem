#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "structure.h"
#define PORT 8009

char readBuffer[5000], writeBuffer[5000];
void clearMemory()
{
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
}

void view_passbook(int cd)
{
    char Buffer[40000];
    bzero(Buffer, sizeof(Buffer));
    int bs = recv(cd, Buffer, sizeof(Buffer) - 1, 0);
    if (bs < 0)
    {
        perror("Error receiving data");
        return;
    }
    printf("%s", Buffer);
    send(cd, Buffer, sizeof(Buffer), 0);
}

void money(int cd, int flg)
{
    float money;
    float total_balance;
    printf("Enter amount:  ");
    scanf("%f", &money);
    time_t t;
    send(cd, &money, sizeof(float), 0);
    if (flg == 1 && money > 0)
    {
        time(&t);
        recv(cd, &total_balance, sizeof(float), 0);
        printf("Deposited Succesfully on %s\n New Balance: %f\n", ctime(&t), total_balance);
    }
    else if (flg == 0 && money > 0)
    {
        recv(cd, &total_balance, sizeof(float), 0);
        time(&t);
        printf("Withdrawn Succesfully on %s\n New Balance: %f\n", ctime(&t), total_balance);
    }
    else
        printf("You have tried to deposit invalid amount!\n");
}


void handle_admin(int cd,char* username,char* password)
{
   
    int log1;
    clearMemory();
    fflush(stdin);
    fflush(stdout);
    send(cd, username, strlen(username)+1, 0); 
    send(cd, password, strlen(password)+1, 0);  
    recv(cd, &log1, sizeof(int), 0);
    
    if (log1 == 1)
    {
        printf("Logged in successfully!\n");
        while (1)
        {
            int ch;
            // Display admin menu on client side
            printf("\n=====================================================================\n"
                   "OPTIONS:\n"
                   "1. Add User\n"
                   "2. Deactivate/Activate Accounts\n"
                   "3. Employee/Manager Details\n"
                   "4. Logout\n"
                   "=====================================================================\n"
                   "Enter your choice: ");
            scanf("%d", &ch);

            // Send choice to server
            send(cd, &ch, sizeof(int), 0);
            
            switch (ch)
            {
            case 1:
                char username[100];
                char password[100];
                int role,success;
                printf("Enter username: ");
                scanf("%s",username);
                printf("Enter Password: ");
                scanf("%s",password);
                printf("Assign Role: ");
                fflush(stdin);
                fflush(stdout);
                clearMemory();
                send(cd,username,sizeof(username),0);
                send(cd,password,sizeof(password),0);
                send(cd,role,sizeof(int),0);
                recv(cd,success,sizeof(int),0);
                break;
            case 2:
                printf("Deactivate/Activate Accounts option selected.\n");
                // Perform Account management here on client side
                break;
            case 3:
                printf("Employee/Manager Details option selected.\n");
                // Perform detail retrieval/display here on client side
                break;
            case 4:
                printf("Logging out...\n");
                return;
            default:
                printf("Invalid Choice. Try again.\n");
                break;
            }
        }
    }
    else
    {
        printf("Invalid credentials. Please try again.\n");
    }
}

void handle_user(int cd)
{
    char username[100];
    char password[100];
    clearMemory();
    recv(cd, readBuffer, sizeof(readBuffer), 0);
    printf("%s", readBuffer);
    scanf("%s", username);
    send(cd, username, strlen(username), 0);
    clearMemory();
    recv(cd, readBuffer, sizeof(readBuffer), 0);
    printf("%s", readBuffer);
    scanf("%s", password);
    send(cd, password, strlen(password), 0);
    int log1;
    recv(cd, &log1, sizeof(int), 0);
    if (log1 == 1)
    {
        printf("Logged in\n");
        while (1)
        {
            fflush(stdout);
            clearMemory();
            recv(cd, readBuffer, sizeof(readBuffer), 0);
            int ch;
            printf("%s", readBuffer);
            scanf("%d", &ch);
            send(cd, &ch, sizeof(int), 0);
            clearMemory();
            switch (ch)
            {
            case 1:
                float balance;
                recv(cd, &balance, sizeof(float), 0);
                time_t t;
                time(&t);
                printf("Account Owner:%s\n", username);
                printf("Balance :%f\n", balance);
                printf("Viewed on %s", ctime(&t));
                break;
            case 2:
                money(cd, 1);
                break;
            case 3:
                money(cd, 0);
                break;
            case 4:
                char username[100];
                printf("Enter username to transfer to: ");
                scanf("%s", username);
                char buffer[100];
                float amount;
                printf("Enter amount to transfer: ");
                scanf("%f", &amount);
                send(cd, username, strlen(username) + 1, 0);
                recv(cd, writeBuffer, sizeof(writeBuffer), 0);
                sprintf(buffer, "%.2f", amount);
                send(cd, buffer, strlen(buffer) + 1, 0);
                recv(cd, writeBuffer, sizeof(writeBuffer), 0);
                int res;
                recv(cd, &res, sizeof(int), 0);
                if (res == 0)
                    printf("Transfer Successful\n");
                else
                    printf("Transfer Failed\n");
                break;
            case 5:
                float amount1;
                char buffer1[500];
                recv(cd, buffer1, sizeof(buffer), 0);
                printf("%s", buffer1);
                scanf("%f", &amount1);

                send(cd, &amount1, sizeof(float), 0);

                recv(cd, buffer1, sizeof(buffer), 0);
                printf("%s", buffer1);
                break;
            case 6:
                char buffer2[5000];
                memset(buffer2, 0, sizeof(buffer2));
                recv(cd, buffer2, sizeof(buffer2), 0);
                printf("%s", buffer2);
                send(cd, writeBuffer, sizeof(writeBuffer), 0);
                break;
            case 7:
                char feedback[1700];
                printf("Enter your feedback: ");
                getchar();
                clearMemory();
                fgets(feedback, sizeof(feedback), stdin);
                send(cd, feedback, strlen(feedback) + 1, 0);
                recv(cd, writeBuffer, sizeof(writeBuffer), 0);
                printf("%s\n", writeBuffer);
                clearMemory();
                send(cd, writeBuffer, sizeof(writeBuffer), 0);
                break;
            case 8:
                char Buffer[40000];
                view_passbook(cd);
                break;
            case 9:
                printf("User Logged out\n");
                return;
                break;
            default:
                printf("Invalid Choice\n");
                break;
            }
        }
    }
    else
        printf("Invalid Credentials\n");
    return;
}


void server_handler(int cd)
{
    printf("Connection established\n");
    while (1)
    {
        int readB, writeB;
        int choice;
        clearMemory();
        recv(cd, readBuffer, sizeof(readBuffer), 0);
        printf("%s", readBuffer);
        scanf("%d", &choice);
        send(cd, &choice, sizeof(int), 0);
        clearMemory();
        switch (choice)
        {
        case 1:
            handle_user(cd);
            break;
        case 4:
            char username[100];
            char password[100];
            printf("Enter username: ");
            scanf("%s",username);
            printf("Enter Password: ");
            scanf("%s",password);
            handle_admin(cd,username,password);
            break;
        case 5:
            return;
        default:
            break;
        }
    }
}

int main()
{
    struct sockaddr_in server_address;
    int cd, cs;
    cd = socket(AF_INET, SOCK_STREAM, 0);
    if (cd == -1)
    {
        perror("Connection Failed");
        _exit(0);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);
    cs = connect(cd, (void *)&server_address, sizeof(server_address));
    if (cs == -1)
    {
        perror("Server connection failed");
        close(cd);
        _exit(0);
    }
    server_handler(cd);
    close(cs);
    return 0;
}
