#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "structure.h"
#define PORT 8009

char serverMessage[1000], clientMessage[1000], dummyBuffer[100];

void clearBuffers()
{
    bzero(serverMessage, sizeof(serverMessage));
    bzero(clientMessage, sizeof(clientMessage));
    bzero(dummyBuffer, sizeof(dummyBuffer));
}

char readBuffer[5000], writeBuffer[5000];
void clearMemory()
{
    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));
}

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

void view_passbook(struct USER *loginUser, int cd)
{
    int fd = open("transaction.db", O_RDONLY);
    if (fd == -1)
    {
        perror("System Crashed\n");
        return;
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    while (fcntl(fd, F_SETLKW, &lock) == -1)
        ;

    struct TRANSACTION tran;
    char Buffer[40000];
    bzero(Buffer, sizeof(Buffer));
    int buffer_offset = 0;
    strcpy(Buffer, "PASSBOOK OF ACCOUNT: \n");
    strcat(Buffer, loginUser->username);
    strcat(Buffer, "\n");
    while (read(fd, &tran, sizeof(struct TRANSACTION)) > 0)
    {
        if (strcmp(tran.username, loginUser->username) == 0)
        {
            char *z = (tran.flag == 1) ? "+" : "-";
            strcat(Buffer, z);
            char str[70];
            sprintf(str, "%-10.2f", tran.amount);
            strcat(Buffer, str);
            strcat(Buffer, ": Balance :");
            char str1[70];
            sprintf(str1, "%-10.2f", tran.total_amount);
            strcat(Buffer, str1);
            strcat(Buffer, "on");
            strcat(Buffer, ctime(&tran.tme));
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
    send(cd, Buffer, strlen(Buffer), 0);
    bzero(Buffer, sizeof(Buffer));
    recv(cd, Buffer, sizeof(Buffer), 0);
    bzero(Buffer, sizeof(Buffer));
}

void passbook_update(struct TRANSACTION *tran)
{
    int fd = open("transaction.db", O_WRONLY | O_APPEND);
    if (fd == -1)
    {
        perror("System Crashed\n");
        return;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = sizeof(struct TRANSACTION);
    lock.l_pid = getpid();
    while (fcntl(fd, F_SETLKW, &lock) == -1)
        ;
    if (write(fd, tran, sizeof(struct TRANSACTION)) == -1)
    {
        perror("System Issue");
    }
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &lock) == -1)
    {
        perror("System Issue");
    }
    close(fd);
}

float view_balance(struct USER *loginUser)
{
    struct ACCOUNT userAccount;
    struct flock lock;
    float balance;
    int fd = open("account.db", O_RDONLY);
    int soff;
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &userAccount, sizeof(struct ACCOUNT)) > 0)
    {
        if (strcmp(loginUser->username, userAccount.username) == 0)
        {
            lock.l_type = F_RDLCK;
            lock.l_whence = SEEK_SET;
            lock.l_start = soff;
            lock.l_len = sizeof(struct ACCOUNT);
            lock.l_pid = getpid();
            int lck = fcntl(fd, F_SETLK, &lock);
            if (lck == -1)
            {
                perror("System Crashed\n");
                close(fd);
                _exit(1);
            }
            balance = userAccount.balance;
            lock.l_type = F_UNLCK;
            lck = fcntl(fd, F_SETLK, &lock);
            if (lck == -1)
            {
                perror("System Crashed\n");
                close(fd);
                _exit(1);
            }
        }
    }
    close(fd);
    return balance;
}

int transaction_in_db(int fd, struct ACCOUNT *userAccount, int offset, float amount)
{
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = sizeof(struct ACCOUNT);
    lock.l_pid = getpid();
    int ls = fcntl(fd, F_SETLK, &lock);
    if (ls == -1)
    {
        perror("System Crash\n");
        close(fd);
        _exit(1);
    }
    int ss = lseek(fd, offset, SEEK_SET);
    if (ss == -1)
    {
        perror("System Crash\n");
        close(fd);
        _exit(1);
    }
    struct ACCOUNT User;
    read(fd, &User, sizeof(struct ACCOUNT));
    float total_amount = userAccount->balance + amount;
    strcpy(User.username, userAccount->username);
    User.balance = userAccount->balance;
    if (total_amount >= 0)
    {
        User.balance += amount;
    }
    else
    {
        perror("ACCOUNT OUT OF BALANCE");
        return 1;
    }
    lseek(fd, -1 * sizeof(struct ACCOUNT), SEEK_CUR);
    int ws = write(fd, &User, sizeof(struct ACCOUNT));
    if (ws < 0)
    {
        perror("System Crash\n");
        close(fd);
        _exit(1);
    }
    lock.l_type = F_UNLCK;
    int u = fcntl(fd, F_SETLK, &lock);
    if (u == -1)
    {
        perror("System Crash\n");
        close(fd);
        _exit(1);
    }
    return 0;
    close(fd);
}
void handle_feedback(int cd) {
    struct FEEDBACK userFeedback;
    char buffer[500];
    recv(cd, userFeedback.feedback, sizeof(userFeedback.feedback), 0);
    strcpy(userFeedback.username, "username_from_session"); // Replace with actual logic
    time(&userFeedback.tme);
    int feedback_fd = open("feedback.db", O_WRONLY | O_APPEND);
    if (feedback_fd == -1) {
        perror("Error opening feedback database");
        return;
    }
    if (write(feedback_fd, &userFeedback, sizeof(struct FEEDBACK)) == -1) {
        perror("Error writing feedback");
    } else {
        strcpy(buffer, "Feedback submitted successfully.\n");
        send(cd, buffer, strlen(buffer) + 1, 0); 
    }

    close(feedback_fd);
    recv(cd, writeBuffer, sizeof(writeBuffer), 0);
}

void transaction_update(struct ACCOUNT *userAccount, float amount)
{
    int fd = open("account.db", O_RDWR);
    if (fd < 0)
    {
        perror("System Crashed\n");
        return;
    }
    struct ACCOUNT User;
    int soff;
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct ACCOUNT)) > 0)
    {
        if (strcmp(userAccount->username, User.username) == 0)
        {
            transaction_in_db(fd, userAccount, soff, amount);
            break;
        }
    }
    close(fd);
}


void money(struct USER *loginUser, int flg, int cd)
{
    struct ACCOUNT User;
    struct TRANSACTION tran;
    float money;
    float total_balance;
    recv(cd, &money, sizeof(float), 0);
    strcpy(User.username, loginUser->username);
    User.balance = view_balance(loginUser);
    time_t t;
    if (flg == 1 && money > 0)
    {
        transaction_update(&User, money);
        total_balance = view_balance(loginUser);
        time(&t);
        strcpy(tran.username, loginUser->username);
        tran.amount = money;
        tran.tme = t;
        tran.flag = 1;
        tran.total_amount = total_balance;
        passbook_update(&tran);
        send(cd, &total_balance, sizeof(float), 0);
    }
    else if (flg == 0 && money > 0)
    {
        transaction_update(&User, -money);
        total_balance = view_balance(loginUser);
        time(&t);
        strcpy(tran.username, loginUser->username);
        tran.amount = money;
        tran.tme = t;
        tran.flag = 0;
        tran.total_amount = total_balance;
        passbook_update(&tran);
        send(cd, &total_balance, sizeof(float), 0);
    }
    else
    {
        printf("Error\n");
    }
}

int transfer(struct USER *loginUser, int cd, char *receiver, float amount)
{
    struct TRANSACTION senderRecord, receiverRecord;
    struct ACCOUNT senderAccount, receiverAccount;
    int s_off, r_off;
    struct flock sender_lock, receiver_lock;
    int sls, rls;
    int sender_fd, receiver_fd;
    time_t t;
    sender_fd = open("account.db", O_RDWR);
    receiver_fd = open("account.db", O_RDWR);
    while (read(sender_fd, &senderAccount, sizeof(struct ACCOUNT)) > 0)
    {
        if (strcmp(loginUser->username, senderAccount.username) == 0)
        {
            s_off = lseek(sender_fd, -1 * sizeof(struct ACCOUNT), SEEK_CUR);
            break;
        }
    }
    sender_lock.l_type = F_WRLCK;
    sender_lock.l_whence = SEEK_SET;
    sender_lock.l_start = s_off;
    sender_lock.l_len = sizeof(struct ACCOUNT);
    sender_lock.l_pid = getpid();
    sls = fcntl(sender_fd, F_SETLK, &sender_lock);
    lseek(sender_fd, s_off, SEEK_SET);
    read(sender_fd, &senderAccount, sizeof(struct ACCOUNT));
    if (amount > senderAccount.balance)
    {
        sender_lock.l_type = F_UNLCK;
        fcntl(sender_fd, F_SETLK, &sender_lock);
        close(sender_fd);
        close(receiver_fd);
        return 1;
    }
    else
    {
        lseek(sender_fd, s_off, SEEK_SET);
        float new_amount = senderAccount.balance - amount;
        senderAccount.balance = new_amount;
        strcpy(senderRecord.username, loginUser->username);
        time(&t);
        senderRecord.amount = amount;
        senderRecord.flag = 0;
        senderRecord.tme = t;
        senderRecord.total_amount = senderAccount.balance;
        passbook_update(&senderRecord);
        lseek(sender_fd, s_off, SEEK_SET);
        write(sender_fd, &senderAccount, sizeof(struct ACCOUNT));
        sender_lock.l_type = F_UNLCK;
        fcntl(sender_fd, F_SETLK, &sender_lock);
        close(sender_fd);
        while (read(receiver_fd, &receiverAccount, sizeof(struct ACCOUNT)) > 0)
        {
            if (strcmp(receiver, receiverAccount.username) == 0)
            {
                r_off = lseek(receiver_fd, -1 * sizeof(struct ACCOUNT), SEEK_CUR);
                break;
            }
        }
        receiver_lock.l_type = F_WRLCK;
        receiver_lock.l_whence = SEEK_SET;
        receiver_lock.l_start = r_off;
        receiver_lock.l_len = sizeof(struct ACCOUNT);
        receiver_lock.l_pid = getpid();
        rls = fcntl(receiver_fd, F_SETLK, &receiver_lock);
        lseek(receiver_fd, r_off, SEEK_SET);
        read(receiver_fd, &receiverAccount, sizeof(struct ACCOUNT));
        receiverAccount.balance += amount;
        strcpy(receiverRecord.username, receiver);
        receiverRecord.amount = amount;
        receiverRecord.tme = t;
        receiverRecord.flag = 1;
        receiverRecord.total_amount = receiverAccount.balance;
        passbook_update(&receiverRecord);
        lseek(receiver_fd, r_off, SEEK_SET);
        write(receiver_fd, &receiverAccount, sizeof(struct ACCOUNT));
        receiver_lock.l_type = F_UNLCK;
        fcntl(receiver_fd, F_SETLK, &receiver_lock);
        close(receiver_fd);
    }
    return 0;
}
void loan(struct USER *loginUser, int cd) {
    float amount;
    struct LOAN loanAccount, readAccount;
    struct flock lock;
    int ls, wb;
    time_t t;
    int soff;
    send(cd, "Request the Loan Amount:", 25, 0); 
    recv(cd, &amount, sizeof(float), 0); 

    int lfd = open("loan.db", O_WRONLY | O_APPEND);
    if (lfd < 0) {
        perror("System Crashed\n");
        return;
    }

    time(&t);
    strcpy(loanAccount.c_username, loginUser->username);
    loanAccount.loan = amount;
    loanAccount.status = 2; 
    loanAccount.time = t;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = 0; 
    lock.l_pid = getpid();
    ls = fcntl(lfd, F_SETLK, &lock);
    if (ls == -1) {
        perror("System Crashed\n");
        close(lfd);
        return;
    }

    lseek(lfd, 0, SEEK_END);
    wb = write(lfd, &loanAccount, sizeof(struct LOAN));
    if (wb < 0) {
        perror("System crashed\n");
        lock.l_type = F_UNLCK;
        fcntl(lfd, F_SETLK, &lock);
        close(lfd);
        return;
    }

    lock.l_type = F_UNLCK;
    fcntl(lfd, F_SETLK, &lock);
    close(lfd);


    send(cd, "Loan Successfully applied for\n", 32, 0);
}

void view_loans(struct USER *loginUser, int cd) {
    struct LOAN readAccount;
    int lfd = open("loan.db", O_RDONLY);
    if (lfd < 0) {
        perror("Error opening loan database\n");
        return;
    }

    char buffer[5000];
    strcpy(buffer, "The loan requests are:\n");
    int foundLoan = 0; 

    while (read(lfd, &readAccount, sizeof(struct LOAN)) > 0) {
        if (strcmp(loginUser->username, readAccount.c_username) == 0) {
            foundLoan = 1; // Mark that we found at least one loan
            int stat = readAccount.status;
            char stat1[100];
            if (stat == 0) {
                strcpy(stat1, "Accepted");
            } else if (stat == 1) {
                strcpy(stat1, "Rejected");
            } else {
                strcpy(stat1, "Pending");
            }

            char buf[600];
            sprintf(buf, "The loan amount requested: %-10.2f at %s is of status %s\n",
                    readAccount.loan, ctime(&readAccount.time), stat1);
            strcat(buffer, buf);
        }
    }

    if (foundLoan) {
        send(cd, buffer, strlen(buffer), 0);
    } else {
        const char *noLoansMessage = "No loan applications found for your account.\n";
        send(cd, noLoansMessage, strlen(noLoansMessage), 0);
    }
    recv(cd, writeBuffer, sizeof(writeBuffer), 0);
    close(lfd);
}


int authenticate(struct USER *loginUser)
{
    struct USER User;
    int soff;
    int fd = open("user.db", O_RDWR);
    if (fd < 0)
    {
        perror("System Crashed/n");
        return 0;
    }
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct USER)) > 0)
    {
        if (strcmp(loginUser->username, User.username) == 0 && loginUser->password == User.password && loginUser->role == User.role && User.status == 1)
        {
            if (User.session == 0)
            {
                User.session = 1;
                lseek(fd, soff, SEEK_SET);
                if (write(fd, &User, sizeof(struct USER)) < 0)
                {
                    perror("System Crashed\n");
                    close(fd);
                    return 0;
                }
                close(fd);
                return 1;
            }
            else
            {
                close(fd);
                return 0;
            }
        }
    }
    close(fd);
    return 0;
}

void logout(struct USER *loginUser, int cd)
{
    struct USER User;
    int soff;
    int fd = open("user.db", O_RDWR);
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct USER)) > 0)
    {
        if (strcmp(loginUser->username, User.username) == 0 && loginUser->password == User.password && loginUser->role == User.role)
        {
            User.session = 0;
            lseek(fd, soff, SEEK_SET);
            if (write(fd, &User, sizeof(struct USER)) < 0)
            {
                perror("System Crashed\n");
                _exit(1);
            }
        }
    }
    close(fd);
}

void login_admin(struct USER *loginUser, int cd)
{
    char username[100];
    char password[100];
    
    clearMemory();
    recv(cd, username, sizeof(username), 0);
    strcpy(loginUser->username, username);
    
    recv(cd, password, sizeof(password), 0);
    loginUser->password = hash(password);
    loginUser->role = 0;
    int log = authenticate(loginUser);
    send(cd, &log, sizeof(int), 0);
    
    if (log == 1)
    {
        int choice;
        while (1)
        {
            // Wait for the client's choice
            recv(cd, &choice, sizeof(int), 0);
            
            switch (choice)
            {
            case 1:
                // Handle adding user on server-side (if needed)
                break;
            case 2:
                // Handle account deactivation/activation on server-side (if needed)
                break;
            case 3:
                // Handle retrieving employee/manager details on server-side (if needed)
                break;
            case 4:
                // Logout user
                logout(loginUser, cd);
                return;
            default:
                // Handle invalid choice
                break;
            }
        }
    }
}

void login_user(struct USER *loginUser, int cd)
{
    char username[1000];
    char password[1000];
    int choice;
    char UserName[] = "Enter username: ";
    send(cd, UserName, strlen(UserName), 0);
    recv(cd, username, sizeof(username), 0);
    strcpy(loginUser->username, username);
    char Password[] = "Enter password: ";
    send(cd, Password, strlen(Password), 0);
    recv(cd, password, sizeof(password), 0);
    loginUser->password = hash(password);
    loginUser->role = 3;
    int log = authenticate(loginUser);
    printf("%d", log);
    send(cd, &log, sizeof(int), 0);
    if (log == 1)
    {
        while (1)
        {
            char CustomerMenu[2000];
            sprintf(CustomerMenu,
                    "\n=====================================================================\n"
                    "Welcome, USER %s!\n"
                    "=====================================================================\n"
                    "OPTIONS:\n"
                    "1. View Account Balance                                                |\n"
                    "----------------------------                                           |\n"
                    "2. Deposit Money                                                       |\n"
                    "----------------------------                                           |\n"
                    "3. Withdraw Money                                                      |\n"
                    "----------------------------                                           |\n"
                    "4. Transfer Funds                                                      |\n"
                    "----------------------------                                           |\n"
                    "5. Loan Applications                                                   |\n"
                    "----------------------------                                           |\n"
                    "6. Loan Status                                                         |\n"
                    "----------------------------                                           |\n"
                    "7. Feedback                                                            |\n"
                    "----------------------------                                           |\n"
                    "8. View Passbook                                                       |\n"
                    "----------------------------                                           |\n"
                    "9. Logout                                                              |\n"
                    "=====================================================================\n"
                    "Enter your choice: ",
                    loginUser->username);
            send(cd, CustomerMenu, strlen(CustomerMenu), 0);
            recv(cd, &choice, sizeof(int), 0);
            switch (choice)
            {
            case 1:
                float balance = view_balance(loginUser);
                send(cd, &balance, sizeof(float), 0);
                break;
            case 2:
                money(loginUser, 1, cd);
                break;
            case 3:
                money(loginUser, 0, cd);
                break;
            case 4:
                char username[100];
                float amount;
                int res;
                char buffer[100];
                recv(cd, username, sizeof(username), 0);
                send(cd, writeBuffer, sizeof(writeBuffer), 0);
                recv(cd, buffer, sizeof(buffer), 0);
                send(cd, writeBuffer, sizeof(writeBuffer), 0);
                amount = atof(buffer);
                printf("Sending username: %s\n", username);
                printf("Sending amount: %.2f\n", amount);
                res = transfer(loginUser, cd, username, amount);
                send(cd, &res, sizeof(int), 0);
                break;
            case 5:
                loan(loginUser,cd);
                break;
            case 6:
                view_loans(loginUser,cd);
                break;
            case 7:
                int i;
                handle_feedback(cd);
                break;
            case 8:
                view_passbook(loginUser, cd);
                break;
            case 9:
                logout(loginUser, cd);
                return;
            default:
                break;
            }
        }
    }
}


void client_handler(int cd)
{
    struct USER loginUser;
    struct ACCOUNT userAccount;
    printf("Client has connected to the server!\n");
    while (1)
    {
        char MainMenu[] =
            "=====================================================================\n"
            "\t WELCOME TO BANKING PORTAL \t\n"
            "=====================================================================\n"
            "MENU                                                                |\n"
            "1. Customer Login                                                   |\n"
            "----------------------------                                        |\n"
            "2. Employee Login                                                   |\n"
            "---------------------------                                         |\n"
            "3. Manager  Login                                                   |\n"
            "----------------------------                                        |\n"
            "4. Admin  Login                                                     |\n"
            "----------------------------                                        |\n"
            "5. Exit                                                             |\n"
            "=====================================================================\n"
            "Please Make a Choice: ";
        int serverMB, clientMB;
        int choice;
        serverMB = send(cd, MainMenu, strlen(MainMenu), 0);
        clientMB = recv(cd, &choice, sizeof(int), 0);
        switch (choice)
        {
        case 1:
            login_user(&loginUser, cd);
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            login_admin(&loginUser,cd);
            break;
        case 5:
            printf("A client has disconnected\n");
            return;
        default:
            break;
        }
    }
}

int main()
{
    struct sockaddr_in server_address, client_address;
    int sd, sbs, sckls, cd;
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("Socket Creation Failed");
        _exit(0);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);
    sbs = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (sbs == -1)
    {
        perror("Binding failed!");
        _exit(0);
    }
    sckls = listen(sd, 50);
    if (sckls == -1)
    {
        perror("System Crashed\n");
        close(sd);
        _exit(0);
    }
    while (1)
    {
        int client_addrsize = sizeof(client_address);
        cd = accept(sd, (struct sockaddr *)&client_address, &client_addrsize);
        if (cd == -1)
        {
            perror("Client System Crashed\n");
            close(sd);
        }
        else
        {
            if (!fork())
            {
                client_handler(cd);
                close(cd);
                _exit(0);
            }
            else
            {
                close(cd);
            }
        }
    }
    close(sd);
    return 0;
}
