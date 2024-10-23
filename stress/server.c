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
#include<time.h>
#define PORT 8108



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



char *view_feedback()
{
    int fd = open("feedback.db", O_RDONLY);
    if (fd == -1)
    {
        perror("System Crashed\n");
        return "Can't fetch feedback";
    }

    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Failed to lock file\n");
        close(fd);
        return "Can't fetch feedback";
    }

    struct FEEDBACK fdbk;
    char *fBuffer = malloc(50000); 
    if (fBuffer == NULL)
    {
        perror("Memory allocation failed\n");
        close(fd);
        return "Can't fetch feedback";
    }

    strcpy(fBuffer, "The feedbacks collected are:\n\n");

    while (read(fd, &fdbk, sizeof(struct FEEDBACK)) > 0)
    {
        strcat(fBuffer, "Feedback given by: ");
        strcat(fBuffer, fdbk.username);
        strcat(fBuffer, "\nFeedback: ");
        strcat(fBuffer, fdbk.feedback);
        strcat(fBuffer, "\nGiven on: ");
        strcat(fBuffer, ctime(&fdbk.tme));
        strcat(fBuffer, "\n\n");
    }

    
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("Failed to unlock file\n");
    }

    close(fd);
    return fBuffer;
}



char *view_passbook(char *t_username)
{
    int fd = open("transaction.db", O_RDONLY);
    if (fd == -1)
    {
        perror("System Crashed\n");
    }
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    while (fcntl(fd, F_SETLKW, &lock) == -1);

    struct TRANSACTION tran;
    char *Buffer=malloc(40000);
    strcpy(Buffer, "PASSBOOK OF ACCOUNT: \n");
    strcat(Buffer, t_username);
    strcat(Buffer, "\n");
    while (read(fd, &tran, sizeof(struct TRANSACTION)) > 0)
    {
        if (strcmp(tran.username, t_username) == 0)
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
    return Buffer;
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
char * money(struct USER *loginUser,float money, int flg)
{
    struct ACCOUNT User;
    struct TRANSACTION tran;
    float total_balance;
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
        char *t_balance=malloc(20); ;
        sprintf(t_balance,"%0.2f",total_balance);
        return t_balance;
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
        char *t_balance=malloc(20); ;
        sprintf(t_balance,"%0.2f",total_balance);
        return t_balance;
        
    }
    else
    {
        printf("Error\n");
    }
}

int changedetails(char* username,char* new_username,int activate, int role,int new_role)
{
    struct USER U;
    struct flock lock;
    int soff;
    int fd = open("user.db", O_RDWR);
    if (fd < 0)
    {
        perror("System Crashed/n");
        return 0;
    }
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &U, sizeof(struct USER)) > 0)
    {
        if (strcmp(username, U.username) == 0 && role == U.role)
        {   printf("Step1 of details initialized\n");
            lock.l_type = F_WRLCK;      
            lock.l_whence = SEEK_SET;   
            lock.l_start = soff;       
            lock.l_len = sizeof(struct USER); 
            lock.l_pid = getpid(); 
            strcpy(U.username,new_username);
            printf("%s\n",U.username);
            U.status=activate;
            U.role=new_role;
            printf("%d\n",U.role);
            lseek(fd, soff, SEEK_SET);
            if (write(fd, &U, sizeof(struct USER)) < 0){
                    perror("System Crashed\n");
                    lock.l_type = F_UNLCK; 
                    fcntl(fd, F_SETLK, &lock);
                    close(fd);
                    return 0;
                }
                lock.l_type = F_UNLCK; 
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return 1;
            
           
        }
    }
    close(fd);
    return 0;
}


int authenticate(struct USER *loginUser)
{
    struct USER User;
    struct flock lock;
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
            lock.l_type = F_WRLCK;      
            lock.l_whence = SEEK_SET;   
            lock.l_start = soff;       
            lock.l_len = sizeof(struct USER); 
            lock.l_pid = getpid(); 
            if (User.session == 0)
            {   
                User.session = 1;
                lseek(fd, soff, SEEK_SET);
                if (write(fd, &User, sizeof(struct USER)) < 0)
                {
                    perror("System Crashed\n");
                    lock.l_type = F_UNLCK; 
                    fcntl(fd, F_SETLK, &lock);
                    close(fd);
                    return 0;
                }
                lock.l_type = F_UNLCK; 
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return 1;
            }
            else
            {   lock.l_type = F_UNLCK; 
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return 0;
            }
        }
    }
    close(fd);
    return 0;
}

void logout(struct USER *loginUser)
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

int transfer(struct USER *loginUser,char *receiver, float amount)
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
    sls = fcntl(sender_fd, F_SETLKW, &sender_lock);
    lseek(sender_fd, s_off, SEEK_SET);
    read(sender_fd, &senderAccount, sizeof(struct ACCOUNT));
    if (amount > senderAccount.balance)
    {
        sender_lock.l_type = F_UNLCK;
        fcntl(sender_fd, F_SETLKW, &sender_lock);
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
        fcntl(sender_fd, F_SETLKW, &sender_lock);
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
        rls = fcntl(receiver_fd, F_SETLKW, &receiver_lock);
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
        fcntl(receiver_fd, F_SETLKW, &receiver_lock);
        close(receiver_fd);
    }
    return 0;
}

int  loan(struct USER *loginUser, float amount) {
    struct LOAN loanAccount, readAccount;
    struct flock lock;
    int ls, wb;
    time_t t;
    int soff;

    int lfd = open("loan.db", O_WRONLY | O_APPEND);
    if (lfd < 0) {
        perror("System Crashed\n");
        return 0;
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
    ls = fcntl(lfd, F_SETLKW, &lock);
    if (ls == -1) {
        perror("System Crashed\n");
        close(lfd);
        return 0;
    }

    lseek(lfd, 0, SEEK_END);
    wb = write(lfd, &loanAccount, sizeof(struct LOAN));
    if (wb < 0) {
        perror("System crashed\n");
        lock.l_type = F_UNLCK;
        fcntl(lfd, F_SETLKW, &lock);
        close(lfd);
        return 0;
    }

    lock.l_type = F_UNLCK;
    fcntl(lfd, F_SETLK, &lock);
    close(lfd);

    printf("Job done\n");
    return 1;
}


char *view_loans_manager() {
    struct LOAN readAccount;
    int lfd = open("loan.db", O_RDONLY);
    if (lfd < 0) {
        perror("Error opening loan database\n");
        return "FAILED";
    }

    char *loan_buffer= malloc(50000);
    strcpy(loan_buffer, "The loan requests are:\n");

    while (read(lfd, &readAccount, sizeof(struct LOAN)) > 0) {
      
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
            sprintf(buf, "The loan amount requested: %-10.2f at %s by %s, is of status %s\n",
                    readAccount.loan, ctime(&readAccount.time), readAccount.c_username, stat1);
            
            strcat(loan_buffer, buf);
        
    }
    close(lfd);
    return loan_buffer;
}

int assignloan(char* c_username,char* e_username)
{
    struct LOAN LU;
    struct flock lock;
    int soff;
    int fd = open("loan.db", O_RDWR);
    if (fd < 0)
    {
        perror("System Crashed/n");
        return 0;
    }
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &LU, sizeof(struct LOAN)) > 0)
    {
        if (strcmp(c_username, LU.c_username) == 0 &&  LU.status==2)
        {   printf("Step1 of loan initialized\n");
            lock.l_type = F_WRLCK;      
            lock.l_whence = SEEK_SET;   
            lock.l_start = soff;       
            lock.l_len = sizeof(struct LOAN); 
            lock.l_pid = getpid(); 
            srand(time(0));
            strcpy(LU.e_username,e_username);
            printf("%s\n",LU.e_username);
            LU.status=3;
            LU.id=rand()%501;
            lseek(fd, soff, SEEK_SET);
            if (write(fd, &LU, sizeof(struct LOAN)) < 0){
                    perror("System Crashed\n");
                    lock.l_type = F_UNLCK; 
                    fcntl(fd, F_SETLK, &lock);
                    close(fd);
                    return 0;
                }
                lock.l_type = F_UNLCK; 
                fcntl(fd, F_SETLK, &lock);
                close(fd);
                return 1;
            
           
        }
    }
    close(fd);
    return 0;
}


int decideloan(char* c_username,int id,int decision)
{   struct USER UL;
    struct LOAN LU;
    struct flock lock;
    float amnt;
    int soff;
    int fd = open("loan.db", O_RDWR);
    if (fd < 0)
    {
        perror("System Crashed/n");
        return 0;
    }
    while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &LU, sizeof(struct LOAN)) > 0)
    {
        if (LU.id==id && LU.status==3)
        {   printf("Step1 of loan decision\n");
            lock.l_type = F_WRLCK;      
            lock.l_whence = SEEK_SET;   
            lock.l_start = soff;       
            lock.l_len = sizeof(struct LOAN); 
            lock.l_pid = getpid(); 
            srand(time(0));
            printf("%s\n",LU.e_username);
            LU.status=decision;
            LU.id=rand()%501;
            amnt= LU.loan;
            lseek(fd, soff, SEEK_SET);
            if (write(fd, &LU, sizeof(struct LOAN)) < 0){
                    perror("System Crashed\n");
                    lock.l_type = F_UNLCK; 
                    fcntl(fd, F_SETLK, &lock);
                    close(fd);
                    return 0;
            }
            lock.l_type = F_UNLCK; 
            fcntl(fd, F_SETLK, &lock);
            close(fd);
            if(decision==0){
            strcpy(UL.username,c_username);
            money(&UL,amnt,1);
            }
            return 1;
            
           
        }
    }
    close(fd);
    return 0;
}


char *view_loans_employee(char* username) {
    struct LOAN readAccount;
    int lfd = open("loan.db", O_RDONLY);
    if (lfd < 0) {
        perror("Error opening loan database\n");
        return "FAILED";
    }

    char *loan_buffer= malloc(50000);
    strcpy(loan_buffer, "The loan requests are:\n");
    int foundLoan = 0; 

    while (read(lfd, &readAccount, sizeof(struct LOAN)) > 0) {
        if (strcmp(username, readAccount.e_username) == 0) {
            foundLoan = 1; 
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
            sprintf(buf, "The loan amount requested: %-10.2f at %s by %s of ID %d,is of status %s\n",
                    readAccount.loan, ctime(&readAccount.time), readAccount.c_username,readAccount.id,stat1);
            
            strcat(loan_buffer, buf);
        }
    }
     if (foundLoan) {
        close(lfd);
        return loan_buffer;
    } else {
         close(lfd);
         return "No loan applications assigned to you.\n";
    }
    close(lfd);
}
    
    



 char *view_loans(char* username) {
    struct LOAN readAccount;
    int lfd = open("loan.db", O_RDONLY);
    if (lfd < 0) {
        perror("Error opening loan database\n");
        return "FAILED";
    }

    char *loan_buffer= malloc(50000);
    strcpy(loan_buffer, "The loan requests are:\n");
    int foundLoan = 0; 

    while (read(lfd, &readAccount, sizeof(struct LOAN)) > 0) {
        if (strcmp(username, readAccount.c_username) == 0) {
            foundLoan = 1; 
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
            
            strcat(loan_buffer, buf);
        }
    }

    if (foundLoan) {
        close(lfd);
        return loan_buffer;
    } else {
         close(lfd);
         return "No loan applications found for your account.\n";
    }
    close(lfd);
}

int handle_feedback(char*username,char* feedback) {
    struct FEEDBACK userFeedback;
    strcpy(userFeedback.username, username); 
    strcpy(userFeedback.feedback,feedback);
    time(&userFeedback.tme);
    int feedback_fd = open("feedback.db", O_WRONLY | O_APPEND);
    if (feedback_fd == -1) {
        perror("Error opening feedback database");
        close(feedback_fd);
        return 0;
    }
    if (write(feedback_fd, &userFeedback, sizeof(struct FEEDBACK)) == -1) {
        perror("Error writing feedback");
        close(feedback_fd);
        return 0;
    } else {
        close(feedback_fd);
        return 1;
    }
    return 1;
}



void add_User(char* username,char* password,int role)
{
    struct USER newUser;
    struct ACCOUNT newAccount;
    strcpy(newUser.username, username);
    strcpy(newAccount.username, username);

    newAccount.balance = 0;
    newUser.password = hash(password);
    newUser.role = role;
    newUser.status = 1;
    newUser.session=0;
    int fd = open("user.db", O_WRONLY | O_APPEND | O_CREAT, 0666);
    int fd2 = open("account.db", O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0||fd2<0)
    {
        perror("Error opening database");
        exit(1);
    }


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

    close(fd);
    close(fd2);
}


void login_manager(struct USER *loginU,int cd){
    char sent[100];
    char a_ack[100];
    char a_ack1[100];
    bzero(sent,sizeof(sent));
    bzero(a_ack1,sizeof(a_ack1));
    bzero(a_ack,sizeof(a_ack));
    sprintf(sent,"succesfully conected\n");
    send(cd,sent,sizeof(sent),0);
    bzero(sent,sizeof(sent));
    char m_username[100];
    char m_password[100];
    bzero(m_username,sizeof(m_username));
    bzero(m_password,sizeof(m_password));
    recv(cd,m_username,sizeof(m_username),0);
    strcpy(a_ack,"Acknowledgement");
    send(cd,a_ack,sizeof(a_ack),0);
    recv(cd,m_password,sizeof(m_username),0);
    strcpy(loginU->username, m_username);
    strcpy(a_ack1,"Acknowledgement");
    loginU->password = hash(m_password);
    loginU->role = 1;
    int log_m=authenticate(loginU);
    send(cd,&log_m,sizeof(int),0);
    printf("sending detail\n");
    if(log_m==1){
    while(1){
    int c_choice;
    recv(cd,&c_choice,sizeof(int),0);
    switch(c_choice){

   
   case 1:
    char* loanReader= view_loans_manager();
    send(cd,loanReader,strlen(loanReader)+1,0);
    printf("Job done");
    break;
   
   case 2:
   char l_username[100];
   char lack[100];
   char el_username[100];
   char lack1[100];
   bzero(l_username,sizeof(l_username));
   bzero(el_username,sizeof(el_username));
   bzero(lack,sizeof(lack));
   bzero(lack1,sizeof(lack1));
   recv(cd,l_username,sizeof(l_username),0);
   strcpy(lack,"Acknowledgement");
   send(cd,lack,sizeof(lack),0);
   recv(cd,el_username,sizeof(el_username),0);
    printf("Doing job\n");
    int lact_check=assignloan(l_username,el_username);
    send(cd,&lact_check,sizeof(int),0);
    printf("Job Done\n");
    break;

    case 3:
    char p_username[100];
    char pack[100];
    int prom;
    char pack1[100];
    bzero(p_username,sizeof(p_username));
    bzero(pack,sizeof(pack));
    bzero(pack1,sizeof(pack1));
    recv(cd,p_username,sizeof(p_username),0);
    strcpy(pack,"Acknowledgement");
    send(cd,pack,sizeof(pack),0);
    recv(cd,&prom,sizeof(int),0);
    printf("Doing job\n");
    int pact_check=changedetails(p_username,p_username,1,2,prom);
    send(cd,&pact_check,sizeof(int),0);
    printf("Job Done\n");
    break;
    
    case 4:
    char ac_username[100];
    char ack[100];
    int a_d;
    char ack1[100];
    bzero(ac_username,sizeof(ac_username));
    bzero(ack,sizeof(ack));
    bzero(ack1,sizeof(ack1));
    recv(cd,ac_username,sizeof(ac_username),0);
    strcpy(ack,"Acknowledgement");
    send(cd,ack,sizeof(ack),0);
    recv(cd,&a_d,sizeof(int),0);
    printf("Doing job\n");
    int act_check=changedetails(ac_username,ac_username,a_d,2,2);
    send(cd,&act_check,sizeof(int),0);
    printf("Job Done\n");
    break;
    
    case 5:
    char* feedReader= view_feedback();
    send(cd,feedReader,strlen(feedReader)+1,0);
    printf("Job done");
    break;
    
    case 6:
    logout(loginU);
    return;
    default:
    printf("Invalid choice\n");
    break;
    }



    }
    }
    
    
}
void login_employee(struct USER *loginU,int cd){
    char sent[100];
    char a_ack[100];
    char a_ack1[100];
    bzero(sent,sizeof(sent));
    bzero(a_ack1,sizeof(a_ack1));
    bzero(a_ack,sizeof(a_ack));
    sprintf(sent,"succesfully conected\n");
    send(cd,sent,sizeof(sent),0);
    bzero(sent,sizeof(sent));
    char e_username[100];
    char e_password[100];
    bzero(e_username,sizeof(e_username));
    bzero(e_password,sizeof(e_password));
    recv(cd,e_username,sizeof(e_username),0);
    strcpy(a_ack,"Acknowledgement");
    send(cd,a_ack,sizeof(a_ack),0);
    recv(cd,e_password,sizeof(e_username),0);
    strcpy(loginU->username, e_username);
    strcpy(a_ack1,"Acknowledgement");
    loginU->password = hash(e_password);
    loginU->role = 2;
    int log_e=authenticate(loginU);
    send(cd,&log_e,sizeof(int),0);
    printf("sending detail\n");
    if(log_e==1){
    while(1){
    int c_choice;
    recv(cd,&c_choice,sizeof(int),0);
    switch(c_choice){


   case 1:
    char u_username[100];
    char u_ack[100];
    char u_password[100];
    char u_ack1[100];
    bzero(u_username,sizeof(u_username));
    bzero(u_ack,sizeof(u_ack));
    bzero(u_password,sizeof(u_password));
    bzero(u_ack1,sizeof(u_ack1));
    recv(cd,u_username,sizeof(u_username),0);
    printf("1st step of admin\n");
    strcpy(u_ack,"Acknowledgement");
    send(cd,u_ack,sizeof(u_ack),0);
    recv(cd,u_password,sizeof(u_password),0);
    printf("2nd step of admin\n");
    strcpy(u_ack1,"Acknowledgement");
    send(cd,u_ack1,sizeof(u_ack1),0);
    add_User(u_username,u_password,3);
    printf("Job done\n");
    break;
    
    case 2:
    char ac_username[100];
    char ack[100];
    int a_d;
    char ack1[100];
    bzero(ac_username,sizeof(ac_username));
    bzero(ack,sizeof(ack));
    bzero(ack1,sizeof(ack1));
    recv(cd,ac_username,sizeof(ac_username),0);
    strcpy(ack,"Acknowledgement");
    send(cd,ack,sizeof(ack),0);
    recv(cd,&a_d,sizeof(int),0);
    printf("Doing job\n");
    int act_check=changedetails(ac_username,ac_username,a_d,3,3);
    send(cd,&act_check,sizeof(int),0);
    printf("Job Done\n");
    break;
    
   case 3:
   char pss_username[100];
   bzero(pss_username,sizeof(pss_username));
   recv(cd,pss_username,sizeof(pss_username),0);
   char *s_passbk= view_passbook(pss_username);
   send(cd,s_passbk,strlen(s_passbk)+1,0);
   printf("Job done\n");
   break;

   case 4:
   char d_username[100];
   char dack[100];
   int id;
   int decision;
   char dack1[100];
   bzero(d_username,sizeof(d_username));
   bzero(dack,sizeof(dack));
   bzero(dack1,sizeof(dack1));
   recv(cd,d_username,sizeof(d_username),0);
   strcpy(dack,"Acknowledgement");
   send(cd,dack,sizeof(dack),0);
   recv(cd,&id,sizeof(int),0);
   strcpy(dack1,"Acknowledgement");
   send(cd,dack1,sizeof(dack1),0);
   recv(cd,&decision,sizeof(int),0);
    printf("Doing job\n");
    int dact_check=decideloan(d_username,id,decision);
    send(cd,&dact_check,sizeof(int),0);
    printf("Job Done\n");
    break;

   case 5:
    char* ELReader= view_loans_employee(loginU->username);
    send(cd,ELReader,strlen(ELReader)+1,0);
    printf("Job done");
    break;

    case 6:
    logout(loginU);
    return;
    default:
    printf("Invalid choice\n");
    break;
    }



    }
    }
    
    
}

void login_admin(struct USER *loginU,int cd){
    char sent[100];
    char a_ack[100];
    char a_ack1[100];
    bzero(sent,sizeof(sent));
    bzero(a_ack1,sizeof(a_ack1));
    bzero(a_ack,sizeof(a_ack));
    sprintf(sent,"succesfully conected\n");
    send(cd,sent,sizeof(sent),0);
    bzero(sent,sizeof(sent));
    char a_username[100];
    char a_password[100];
    bzero(a_username,sizeof(a_username));
    bzero(a_password,sizeof(a_password));
    recv(cd,a_username,sizeof(a_username),0);
    strcpy(a_ack,"Acknowledgement");
    send(cd,a_ack,sizeof(a_ack),0);
    recv(cd,a_password,sizeof(a_username),0);
    strcpy(loginU->username, a_username);
    strcpy(a_ack1,"Acknowledgement");
    loginU->password = hash(a_password);
    loginU->role = 0;
    int log_a=authenticate(loginU);
    send(cd,&log_a,sizeof(int),0);
    printf("sending detail\n");
    if(log_a==1){
    while(1){
    int c_choice;
    recv(cd,&c_choice,sizeof(int),0);
    printf("receiving choice\n");
    switch(c_choice){

    case 1:
    char u_username[100];
    char u_ack[100];
    char u_password[100];
    char u_ack1[100];
    int  r_role;
    char u_ack2[100];
    bzero(u_username,sizeof(u_username));
    bzero(u_ack,sizeof(u_ack));
    bzero(u_password,sizeof(u_password));
    bzero(u_ack1,sizeof(u_ack1));
    bzero(u_ack2,sizeof(u_ack2));
    recv(cd,u_username,sizeof(u_username),0);
    printf("1st step of admin\n");
    strcpy(u_ack,"Acknowledgement");
    send(cd,u_ack,sizeof(u_ack),0);
    recv(cd,u_password,sizeof(u_password),0);
    printf("2nd step of admin\n");
    strcpy(u_ack1,"Acknowledgement");
    send(cd,u_ack1,sizeof(u_ack1),0);
    recv(cd,&r_role,sizeof(int),0);
    printf("3rd step of admin\n");
    strcpy(u_ack2,"Acknowledgement");
    send(cd,u_ack2,sizeof(u_ack2),0);
    add_User(u_username,u_password,r_role);
    printf("Job done\n");
    break;
    
    case 2:
    char aac_username[100];
    char aack[100];
    int aa_d;
    int aa_role;
    char aack1[100];
    bzero(aac_username,sizeof(aac_username));
    bzero(aack,sizeof(aack));
    bzero(aack1,sizeof(aack1));
    recv(cd,aac_username,sizeof(aac_username),0);
    strcpy(aack,"Acknowledgement");
    send(cd,aack,sizeof(aack),0);
    recv(cd,&aa_d,sizeof(int),0);
    strcpy(aack1,"Acknowledgement");
    send(cd,aack1,sizeof(aack1),0);
    recv(cd,&aa_role,sizeof(int),0);
    printf("Doing job\n");
    int aact_check=changedetails(aac_username,aac_username,aa_d,aa_role,aa_role);
    send(cd,&aact_check,sizeof(int),0);
    printf("Job Done\n");
    break;

   case 3:
    char ac_username[100];
    char ack[100];
    char nac_username[100];
    int a_role;
    int na_role;
    char ack1[100];
    char ack2[100];
    bzero(ac_username,sizeof(ac_username));
    bzero(nac_username,sizeof(nac_username));
    bzero(ack,sizeof(ack));
    bzero(ack1,sizeof(ack1));
    bzero(ack2,sizeof(ack2));
    recv(cd,ac_username,sizeof(ac_username),0);
    strcpy(ack,"Acknowledgement");
    send(cd,ack,sizeof(ack),0);
    recv(cd,nac_username,sizeof(nac_username),0);
    strcpy(ack1,"Acknowledgement");
    send(cd,ack1,sizeof(ack1),0);
    recv(cd,&a_role,sizeof(int),0);
    strcpy(ack2,"Acknowledgement");
    send(cd,ack2,sizeof(ack2),0);
    recv(cd,&na_role,sizeof(int),0);
    printf("Doing job\n");
    int act_check=changedetails(ac_username,nac_username,1,a_role,na_role);
    send(cd,&act_check,sizeof(int),0);
    printf("Job Done\n");
    break;
   




    case 4:
    logout(loginU);
    return;
    default:
    printf("Invalid choice\n");
    break;
    }



    }
    }
    
    
}

void login_user(struct USER *loginU,int cd){
    char sent[100];
    char a_ack[100];
    char a_ack1[100];
    bzero(sent,sizeof(sent));
    bzero(a_ack1,sizeof(a_ack1));
    bzero(a_ack,sizeof(a_ack));
    sprintf(sent,"succesfully conected\n");
    send(cd,sent,sizeof(sent),0);
    bzero(sent,sizeof(sent));
    char c_username[100];
    char c_password[100];
    bzero(c_username,sizeof(c_username));
    bzero(c_password,sizeof(c_password));
    recv(cd,c_username,sizeof(c_username),0);
    strcpy(a_ack,"Acknowledgement");
    send(cd,a_ack,sizeof(a_ack),0);
    recv(cd,c_password,sizeof(c_username),0);
    strcpy(loginU->username, c_username);
    strcpy(a_ack1,"Acknowledgement");
    loginU->password = hash(c_password);
    loginU->role = 3;
    int log_c=authenticate(loginU);
    send(cd,&log_c,sizeof(int),0);
    printf("sending detail\n");
    if(log_c==1){
    while(1){
    int c_choice;
    recv(cd,&c_choice,sizeof(int),0);
    switch(c_choice){
    case 1:
    float balance = view_balance(loginU);
    char c_b[100];
    bzero(c_b,sizeof(c_b));
    sprintf(c_b,"%0.2f",balance);
    send(cd, &c_b, strlen(c_b)+1, 0);
    break;

   case 2:
   char d_money[100];
   bzero(d_money,sizeof(d_money));
   recv(cd,d_money,sizeof(d_money),0);
   float deposit;
   deposit=atof(d_money);
   char *rd_money=money(loginU,deposit,1);
   send(cd,rd_money,strlen(rd_money)+1,0);
   usleep(5);
   break;
   
   case 3:
   char w_money[100];
   bzero(w_money,sizeof(w_money));
   recv(cd,w_money,sizeof(w_money),0);
   float withdrawl;
   withdrawl=atof(w_money);
   char *rw_money=money(loginU,withdrawl,0);
   send(cd,rw_money,strlen(rw_money)+1,0);
   usleep(5);
   break;

   case 4:
  char cs_username[100];
  char ack[100];
  char cs_amount[100];
   char ack1[100];
   bzero(cs_username,sizeof(cs_username));
   bzero(cs_amount,sizeof(cs_amount));
   bzero(ack,sizeof(ack));
   bzero(ack,sizeof(ack));
   recv(cd,cs_username,sizeof(cs_username),0);
   strcpy(ack,"acknowledged");
   send(cd,ack,sizeof(ack),0);
   printf("Received username\n");

   recv(cd,cs_amount,sizeof(cs_amount),0);
   strcpy(ack1,"acknowledged");
   send(cd,ack1,sizeof(ack1),0);
   printf("Received amount\n");
   float transfer_amt;
   transfer_amt = atof(cs_amount);
   int c_check = transfer(loginU,cs_username,transfer_amt);
   send(cd,&c_check,sizeof(int),0);
   printf("sent the data\n");
   printf("Job Done\n");
   break;
   
   case 5:
   char l_amount[100];
   float l_lamount;
   bzero(l_amount,sizeof(l_amount));
   recv(cd,l_amount,sizeof(l_amount),0);
   l_lamount= atof(l_amount);
   int l_lcheck = loan(loginU,l_lamount);
   send(cd,&l_lcheck,sizeof(int),0);
   break;

   
  case 6:
  char *view_loan_buffer=view_loans(loginU->username);
  send(cd,view_loan_buffer,strlen(view_loan_buffer)+1,0);
  printf("View job done\n");
  break;

  case 7:
  char feedback[5000];
  bzero(feedback,sizeof(feedback));
  recv(cd,feedback,sizeof(feedback),0);
  printf("Receiving Data\n");
  printf("1step for feedback\n");
  int f_check= handle_feedback(loginU->username,feedback);
  send(cd,&f_check,sizeof(int),0);
  break;
   case 8:
   char *s_passbk= view_passbook(loginU->username);
   send(cd,s_passbk,strlen(s_passbk)+1,0);
   printf("Job done\n");
   break;
   
   case 9:
    logout(loginU);
    return;
    default:
    break;
    }



    }
    }
    
}


void client_handler(int cd){
struct USER loginUser;
struct ACCOUNT userAccount;
printf("Client has connected to the server!\n");
while(1){
int choice;
char mack[100];
bzero(mack,sizeof(mack));
recv(cd,&choice,sizeof(choice),0);
strcpy(mack,"Acknowledgement");
send(cd,mack,sizeof(mack),0);
bzero(mack,sizeof(mack));
usleep(5);
switch(choice){
    case 1:
    login_user(&loginUser,cd);
    break;
    
    case 2:
    login_employee(&loginUser,cd);
    break;

    case 3:
    login_manager(&loginUser,cd);
    break;

    case 4:
    login_admin(&loginUser,cd);
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