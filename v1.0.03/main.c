
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include "structure.h"
#include <time.h>

unsigned long hash(const char *password){
unsigned long crypt = 7024;
int c;
while ((c = *password++))
{crypt = ((crypt << 7) + crypt) + c; }
return crypt;
}


float view_balance(struct USER *loginUser){
struct ACCOUNT userAccount;
struct flock lock;
float balance;
int fd = open("account.db", O_RDONLY);
int soff;
while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &userAccount, sizeof(struct ACCOUNT)) > 0){
if (strcmp(loginUser->username, userAccount.username) == 0){
lock.l_type = F_RDLCK;
lock.l_whence = SEEK_SET;            
lock.l_start = soff;
lock.l_len = sizeof(struct ACCOUNT); 
lock.l_pid = getpid();
int lck= fcntl(fd, F_SETLK, &lock);
if (lck == -1){
perror("System Crashed\n");
close(fd);
_exit(1);}
balance = userAccount.balance;
lock.l_type=F_UNLCK;
lck=fcntl(fd,F_SETLK,&lock);
if(lck==-1){
perror("System Crashed\n");
close(fd);
_exit(1);
}
}}
close(fd);
return balance;
}

void passbook_update(struct TRANSACTION *tran){
int fd = open("transaction.db", O_WRONLY | O_APPEND);
if (fd == -1){
perror("System Crashed\n");
return;
}
struct flock lock;
lock.l_type = F_WRLCK;
lock.l_whence = SEEK_END;
lock.l_start = 0;
lock.l_len = sizeof(struct TRANSACTION);
lock.l_pid = getpid();
while (fcntl(fd, F_SETLKW, &lock) == -1);
if (write(fd, tran, sizeof(struct TRANSACTION)) == -1){
perror("System Issue");}
lock.l_type = F_UNLCK;
if (fcntl(fd, F_SETLK, &lock) == -1){
perror("System Issue");}
close(fd);
}

void view_passbook(struct USER *loginUser){
int fd = open("transaction.db", O_RDONLY);
if (fd == -1){
perror("System Crashed\n");
return;}
struct flock lock;
lock.l_type = F_RDLCK;    
lock.l_whence = SEEK_SET; 
lock.l_start = 0;         
lock.l_len = 0;           
lock.l_pid = getpid();    
while (fcntl(fd, F_SETLKW, &lock) == -1);
struct TRANSACTION tran;
printf("PASSBOOK OF ACCOUNT: %s\n",loginUser->username);
while (read(fd, &tran, sizeof(struct TRANSACTION)) > 0){
if (strcmp(tran.username, loginUser->username) == 0){
if (tran.flag == 1){printf("+");}
else{printf("-");}
printf("%-10.2f : Balance : %-10.2f :\t On %s", tran.amount,tran.total_amount,ctime(&tran.tme));}}
lock.l_type = F_UNLCK;
fcntl(fd, F_SETLKW, &lock);
close(fd);
}




int transaction_in_db(int fd, struct ACCOUNT *userAccount,int offset,float amount)
{
struct flock lock;
lock.l_type = F_WRLCK;               
lock.l_whence = SEEK_SET;            
lock.l_start = offset;               
lock.l_len = sizeof(struct ACCOUNT); 
lock.l_pid = getpid();               
int ls=fcntl(fd, F_SETLK, &lock);
if (ls == -1)
{perror("System Crash\n");
close(fd);
_exit(1);}
int ss = lseek(fd, offset, SEEK_SET);
if (ss == -1){
perror("System Crash\n");
close(fd);
_exit(1);}
struct ACCOUNT User;
read(fd, &User, sizeof(struct ACCOUNT));
float total_amount = userAccount->balance + amount;
strcpy(User.username, userAccount->username);
User.balance = userAccount->balance;
if (total_amount >= 0){User.balance += amount;}
else{printf("ACCOUNT OUT OF BALANCE"); return 1;}
lseek(fd, -1 * sizeof(struct ACCOUNT), SEEK_CUR);
int ws = write(fd, &User, sizeof(struct ACCOUNT));
if (ws < 0){
perror("System Crash\n");
close(fd);
_exit(1);}
lock.l_type = F_UNLCK;
int u = fcntl(fd, F_SETLK, &lock);
if (u == -1){
perror("System Crash\n");
close(fd);
_exit(1);}
return 0;
close(fd);
}


void transaction_update(struct ACCOUNT *userAccount, float amount){
int fd = open("account.db", O_RDWR);
if (fd < 0){
perror("System Crashed\n");
return;
}
struct ACCOUNT User;
int soff;
while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct ACCOUNT)) > 0){
if (strcmp(userAccount->username, User.username) == 0){
transaction_in_db(fd,userAccount,soff, amount);
break;}}
close(fd);
}


void money(struct USER *loginUser,int flg){
struct ACCOUNT User;
struct TRANSACTION tran;
float money;
float total_balance;
printf("Enter amount: ");
scanf("%f", &money);
strcpy(User.username, loginUser->username);
User.balance = view_balance(loginUser);
time_t t;
if (flg==1&&money > 0){
transaction_update(&User, money);
total_balance = view_balance(loginUser);
time(&t);
strcpy(tran.username, loginUser->username);
tran.amount = money;
tran.tme=t;
tran.flag = 1;
tran.total_amount = total_balance;
passbook_update(&tran);
printf("Deposited successfully on %s\n New balance: %.f\n",ctime(&t), total_balance);}
else if(flg==0&&money>0){
transaction_update(&User, -money);
total_balance = view_balance(loginUser);
time(&t);
strcpy(tran.username, loginUser->username);
tran.amount = money;
tran.tme=t;
tran.flag = 0;
tran.total_amount = total_balance;
passbook_update(&tran);
printf("Withdrawed successfully on %s\n New balance: %.f\n",ctime(&t), total_balance);}
else{
printf("You have tried to deposit invalid amount!\n");}
}


void change_password(struct USER *loginUser){
struct USER User;
int soff;
char new_password[100];
printf("New Password:");
scanf("%s",new_password);
int fd = open("user.db", O_RDWR);
if (fd < 0){perror("System Crashed/n");}
while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct USER)) > 0){
if (strcmp(loginUser->username, User.username) == 0){  
User.password=hash(new_password);
lseek(fd, soff, SEEK_SET);
if (write(fd, &User, sizeof(struct USER)) < 0){
perror("System Crashed\n");
close(fd);
return;
}
}}
close(fd);}

void loan(struct USER *loginUser,int flag){
float amount;
struct LOAN loanAccount,readAccount;
struct flock lock;
int ls,wb;
time_t t;
int soff;
if(flag==1){
printf("Request the Loan Amount: ");
scanf("%f", &amount);
int lfd = open("loan.db", O_WRONLY | O_APPEND);
if (lfd < 0){
perror("System Crashed\n");
return;}
time(&t);
strcpy(loanAccount.c_username, loginUser->username);
loanAccount.loan = amount;
loanAccount.status = 2;
loanAccount.time=t;
lock.l_type = F_WRLCK;
lock.l_whence = SEEK_END;
lock.l_start = 0;
lock.l_len = sizeof(struct ACCOUNT);
lock.l_pid = getpid();
ls = fcntl(lfd, F_SETLK, &lock);
if (ls == -1){
perror("System Crashed\n");
close(lfd);
return;}
lseek(lfd, 0, SEEK_END);
wb = write(lfd, &loanAccount, sizeof(struct LOAN));
if (wb < 0){
perror("System crashed\n");
lock.l_type = F_UNLCK;
fcntl(lfd, F_SETLK, &lock);
close(lfd);
return;}
lock.l_type = F_UNLCK;
fcntl(lfd, F_SETLK, &lock);
close(lfd);
printf("Loan Successfully applied for\n");
}
else if(flag==2){
int lfd = open("loan.db", O_RDONLY);
if (lfd < 0){
perror("System Crashed\n");
return;}
while ((soff = lseek(lfd, 0, SEEK_CUR)) != -1 && read(lfd, &readAccount, sizeof(struct LOAN)) > 0){
if(strcmp(loginUser->username,readAccount.c_username)==0){
int stat=readAccount.status;
char stat1[100];
if(stat==0){ strcpy(stat1,"Accepted");} else if(stat==1){strcpy(stat1,"Rejected");} else {strcpy(stat1,"Pending"); }
printf("The loan amount requested : %-10.2f at %s is of status %s\n",readAccount.loan,ctime(&readAccount.time),stat1);
}
else{
continue;
}
}
close(lfd);}
else printf("Invalid");
}
void transfer(struct USER *loginUser){
char receiver[100];
struct TRANSACTION senderRecord, receiverRecord;
struct ACCOUNT senderAccount, receiverAccount;
int s_off, r_off;
struct flock sender_lock, receiver_lock;
int sls, rls;
int sender_fd, receiver_fd;
float amount;
time_t t;
printf("Account name of receiver: ");
scanf("%s", receiver);
printf("Amount to transfer: ");
scanf("%f", &amount);
sender_fd = open("account.db", O_RDWR);
receiver_fd = open("account.db", O_RDWR);
while (read(sender_fd, &senderAccount, sizeof(struct ACCOUNT)) > 0){
if (strcmp(loginUser->username, senderAccount.username) == 0){
s_off = lseek(sender_fd, -1 * sizeof(struct ACCOUNT), SEEK_CUR);
break;}}
sender_lock.l_type = F_WRLCK;
sender_lock.l_whence = SEEK_SET;
sender_lock.l_start = s_off;
sender_lock.l_len = sizeof(struct ACCOUNT);
sender_lock.l_pid = getpid();
sls = fcntl(sender_fd, F_SETLK, &sender_lock);
lseek(sender_fd, s_off, SEEK_SET);
read(sender_fd, &senderAccount, sizeof(struct ACCOUNT));
if (amount > senderAccount.balance){
printf("Transfer not supported due to insufficient balance");
close(sender_fd);
close(receiver_fd);}
else{
lseek(sender_fd, s_off, SEEK_SET);
float new_amount = senderAccount.balance - amount;
senderAccount.balance = new_amount;
strcpy(senderRecord.username, loginUser->username);
time(&t);
senderRecord.amount = amount;
senderRecord.flag = 0;
senderRecord.tme=t;
senderRecord.total_amount = senderAccount.balance;
passbook_update(&senderRecord);
lseek(sender_fd, s_off, SEEK_SET);
write(sender_fd, &senderAccount, sizeof(struct ACCOUNT));
sender_lock.l_type = F_UNLCK;
fcntl(sender_fd, F_SETLK, &sender_lock);
close(sender_fd);
while (read(receiver_fd, &receiverAccount, sizeof(struct ACCOUNT)) > 0){
if (strcmp(receiver, receiverAccount.username) == 0){
r_off = lseek(receiver_fd, -1 * sizeof(struct ACCOUNT), SEEK_CUR);
break;}}
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
receiverRecord.tme=t;
receiverRecord.flag= 1;
receiverRecord.total_amount = receiverAccount.balance;
passbook_update(&receiverRecord);
lseek(receiver_fd, r_off, SEEK_SET);
write(receiver_fd, &receiverAccount, sizeof(struct ACCOUNT));
receiver_lock.l_type = F_UNLCK;
fcntl(receiver_fd, F_SETLK, &receiver_lock);
close(receiver_fd);}
printf("Transfered Successfully\n");
}

void feedback(struct USER *loginUser,int flag){
char feedback[1700];
int feedback_fd, ws;
struct FEEDBACK user;
struct flock lock;
time_t t;
if(flag==1){
printf("Enter Feedback:");
fflush(stdout);
long long rb= read(STDIN_FILENO, feedback, sizeof(feedback));
feedback[rb - 1] = '\0';
strcpy(user.username, loginUser->username);
strcpy(user.feedback, feedback);
time(&t);
user.tme=t;
feedback_fd = open("feedback.db", O_WRONLY | O_APPEND);
if (feedback_fd == -1){
perror("System Crashed/n");
return;
}
lock.l_type = F_WRLCK;
lock.l_whence = SEEK_END;
lock.l_start = 0;
lock.l_len = sizeof(struct FEEDBACK);
lock.l_pid = getpid();
fcntl(feedback_fd, F_SETLK, &lock);
ws = write(feedback_fd, &user, sizeof(struct FEEDBACK));
if (ws == -1){
perror("System Crashed\n");
}
lock.l_type = F_UNLCK;
fcntl(feedback_fd, F_SETLK, &lock);
close(feedback_fd);
}
else if(flag==2){
feedback_fd = open("feedback.db", O_RDONLY);
if (feedback_fd < 0) {
perror("System Crashed\n");
return;
}
printf("Your Feedback entries:\n");
while (read(feedback_fd, &user, sizeof(struct FEEDBACK)) > 0) {
printf("Username: %s\n", user.username);
printf("Feedback: %s\n", user.feedback);
printf("Time of submission:%s\n",ctime(&user.tme));
}
close(feedback_fd);
}
else printf("Too stupid use a simple system smh\n");
}




int authenticate(struct USER *loginUser){
struct USER User;
int soff;
int fd = open("user.db", O_RDWR);
if (fd < 0){
perror("System Crashed/n");
return 0;
}
while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct USER)) > 0){
if (strcmp(loginUser->username, User.username) == 0 && loginUser->password==User.password &&loginUser->role == User.role&& User.status==1){            
if (User.session == 0){
User.session = 1;
lseek(fd, soff, SEEK_SET);
if (write(fd, &User, sizeof(struct USER)) < 0){
perror("System Crashed\n");
close(fd);
return 0;}
close(fd);
return 1; }
else{
close(fd);
return 0;}}}
close(fd);
return 0;
}

void logout(struct USER *loginUser){
struct USER User;
int soff;
int fd = open("user.db", O_RDWR);
while ((soff = lseek(fd, 0, SEEK_CUR)) != -1 && read(fd, &User, sizeof(struct USER)) > 0){
if (strcmp(loginUser->username,User.username) == 0 &&loginUser->password == User.password &&loginUser->role == User.role){
User.session = 0;
lseek(fd, soff, SEEK_SET);
if (write(fd,&User,sizeof(struct USER))< 0)
{perror("System Crashed\n");
_exit(1);}
}}
close(fd);
printf("User Logged out\n");
}








void login_user(struct USER *loginUser){
char username[100];
char password[100];
int choice;
printf("Enter username: ");
scanf("%s", username);
strcpy(loginUser->username,username);
printf("Enter password: ");
scanf("%s", password);
loginUser->password = hash(password);
loginUser->role = 3;
int log = authenticate(loginUser);
if (log == 1){
while (1){
printf("\n=====================================================================\n");
printf("\nWelcome, USER  %s!\nOPTIONS:\n", loginUser->username);
printf("1. View Account Balance                                                |\n");
printf("----------------------------                                           |\n");
printf("2. Deposit Money                                                       |\n");
printf("----------------------------                                           |\n");
printf("3. Withdraw Money                                                      |\n");
printf("----------------------------                                           |\n");
printf("4. Transfer Funds                                                      |\n");
printf("----------------------------                                           |\n");
printf("5. Loan Applications & Status                                          |\n");
printf("----------------------------                                           |\n");
printf("6. Change Password                                                     |\n");
printf("----------------------------                                           |\n");
printf("7. Feedback                                                            |\n");
printf("----------------------------                                           |\n");
printf("8. View Passbook                                                       |\n");
printf("----------------------------                                           |\n");
printf("9. Logout                                                              |\n");
printf("\n=====================================================================\n");
printf("Enter your choice:[");
scanf("%d", &choice);
switch (choice)
{case 1:
float balance = view_balance(loginUser);
time_t t;
time(&t);
printf("Account Owner:%s\n",username);
printf("Viewed on: %s", ctime(&t)); 
printf("Your account balance is: %.2f\n", balance);
break;
case 2:
money(loginUser,1);
break;
case 3:
money(loginUser,0);
break;
case 4:
transfer(loginUser);
break;
case 5:
int c;
printf("Press 1 to apply for loan , Press 2 to view loan status\n");
scanf("%d",&c);
loan(loginUser,c);
break;
case 6:
change_password(loginUser);
break;
case 7:
int i;
printf("Press 1 to add feedback, press 2 to view all feedback");
scanf("%d",&i);
feedback(loginUser,i);
break;
case 8:
view_passbook(loginUser);
break;
case 9:
logout(loginUser);
return;
default:
printf("Invalid choice!\n");}}}
else{
printf("Invalid credentials!\n");
}}



int main()
{
int choice;
struct USER loginUser;
struct ACCOUNT userAccount;
while (1){
printf("=====================================================================\n");
printf("\t WELCOME TO BANKING PORTAL \t\n");
printf("=====================================================================\n");
printf("\t MENU                                                       |\n");
printf("1. Customer Login                                                   |\n");
printf("----------------------------                                        |\n");
printf("2. Employee Login                                                   |\n");
printf("----------------------------                                        |\n");
printf("3. Manager  Login                                                   |\n");
printf("----------------------------                                        |\n");
printf("4. Admin  Login                                                     |\n");
printf("----------------------------                                        |\n");
printf("5. Exit                                                             |\n");
printf("=====================================================================\n");
printf("Please Make a Choice: [");
scanf("%d", &choice);
switch (choice){
case 1:
login_user(&loginUser);
break;
case 5:
printf("Thank you for using our service\n");
exit(0);
default:
printf("Invalid choice!\n");}}
 return 0;
}
