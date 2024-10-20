#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "structure.h"


unsigned long hash(const char *password)
{
    unsigned long crypt = 7024;
    int c;

    while ((c = *password++))
    {
        crypt = ((crypt << 7) + crypt) + c; 
    }

    return crypt;
}


void addUser()
{
    struct USER newUser;
    struct ACCOUNT newAccount;
    char username1[100];

    // Get username and password
    printf("Enter username: ");
    scanf("%s", username1);
    strcpy(newUser.username, username1);
    strcpy(newAccount.username, username1);

    newAccount.balance = 0;
    printf("Enter password: ");
    char pass[100];
    scanf("%s", pass);
    newUser.password = hash(pass);
    int r;
    printf("Give Role");
    scanf("%d",&r);
    newUser.role = r;
    newUser.status = 1;
    newUser.session=0;

    // Open the user database file
    int fd = open("user.db", O_WRONLY | O_APPEND | O_CREAT, 0666);
    int fd2 = open("account.db", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0||fd2<0)
    {
        perror("Error opening database");
        exit(1);
    }

    // Write user data to the file
    if (write(fd, &newUser, sizeof(struct USER)) < 0)
    {
        perror("Error writing to user database");
        close(fd);
        exit(1);
    }

    printf("User added successfully!\n");

    if (write(fd2, &newAccount, sizeof(struct ACCOUNT)) < 0)
    {
        perror("Error writing to account database");
        close(fd2);
        exit(1);
    }

    printf("Account added successfully!\n");

    // Close the file
    close(fd);
    close(fd2);
}

int main()
{
    addUser();
    return 0;
}
