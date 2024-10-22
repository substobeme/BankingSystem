#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "structure.h"
#define PORT 8106

void handle_admin(int cd){
char rec[100];
char a_ack[100];
char a_ack1[100];
printf("Initiated as Admin\n");
bzero(a_ack1,sizeof(a_ack));
bzero(a_ack,sizeof(a_ack));
bzero(rec,sizeof(rec));
recv(cd,rec,sizeof(rec),0);
printf("%s\n",rec);
bzero(rec,sizeof(rec));
char a_username[100];
char a_password[100];
bzero(a_username,sizeof(a_username));
bzero(a_password,sizeof(a_password));
printf("Enter the Username: ");
scanf("%s",a_username);
printf("Enter the password: ");
scanf("%s",a_password);
send(cd,a_username,strlen(a_username)+1,0);
recv(cd,a_ack,sizeof(a_ack),0);
send(cd,a_password,strlen(a_password)+1,0);
recv(cd,a_ack1,sizeof(a_ack1),0);
int log_a;
recv(cd,&log_a,sizeof(int),0);
if(log_a==1){
printf("Logged in successfully\n");
while(1){
    printf("\n=====================================================================\n"
                    "Welcome, ADMIN %s!\n"
                    "=====================================================================\n"
                    "OPTIONS:\n"
                    "1. Add User                                                            |\n"
                    "----------------------------                                           |\n"
                    "2. Activate/Deactivate User                                            |\n"
                    "----------------------------                                           |\n"
                    "3. Change Details                                                      |\n"
                    "----------------------------                                           |\n"
                    "4. Logout                                                              |\n"
                    "=====================================================================\n"
                    "Enter your choice: ",
                    a_username);
int c_choice;
scanf("%d",&c_choice);
send(cd,&c_choice,sizeof(int),0);
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
printf("Enter username : ");
scanf("%s",u_username);
printf("Enter password: ");
scanf("%s",u_password);
printf("Enter role : ");
scanf("%d",&r_role);
printf("Confirming Inputs:\n");
printf("Username:%s\n",u_username);
printf("Password: %s\n",u_password);
printf("Role: %d\n",r_role);
send(cd,u_username,strlen(u_username),0);
recv(cd,u_ack,sizeof(u_ack),0);
send(cd,u_password,strlen(u_password),0);
recv(cd,u_ack1,sizeof(u_ack1),0);
send(cd,&r_role,sizeof(int),0);
recv(cd,u_ack2,sizeof(u_ack2),0);
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
printf("Enter the username: ");
scanf("%s",aac_username);
printf("Enter 0 to deactivate , Enter 1 to activate: ");
scanf("%d",&aa_d);
printf("Enter the role of user:");
scanf("%d",&aa_role);
send(cd,aac_username,strlen(aac_username),0);
recv(cd,aack,sizeof(aack),0);
send(cd,&aa_d,sizeof(int),0);
recv(cd,aack1,sizeof(aack1),0);
send(cd,&aa_role,sizeof(int),0);
int aact_check;
recv(cd,&aact_check,sizeof(int),0);
if(aact_check==1){ printf("Task Completed\n");}
else printf("Failed to change details\n");
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
printf("Enter the username: ");
scanf("%s",ac_username);
printf("Enter the new username: ");
scanf("%s",nac_username);
printf("Enter the role of user: ");
scanf("%d",&a_role);
printf("Enter the new role of user:");
scanf("%d",&na_role);
send(cd,ac_username,strlen(ac_username),0);
recv(cd,ack,sizeof(ack),0);
send(cd,nac_username,strlen(nac_username),0);
recv(cd,ack1,sizeof(ack1),0);
send(cd,&a_role,sizeof(int),0);
recv(cd,ack2,sizeof(ack2),0);
send(cd,&na_role,sizeof(int),0);
int act_check;
recv(cd,&act_check,sizeof(int),0);
if(act_check==1){ printf("Task Completed\n");}
else printf("Failed to change details\n");
break;


case 4:
printf("Logged out\n");
return;
default:
printf("Invalid choice\n");
break;

}
}
}
else {printf("Invalid credentials\n");}
}

void handle_user(int cd){

char rec[100];
char a_ack[100];
char a_ack1[100];
printf("Initiated as User\n");
bzero(a_ack1,sizeof(a_ack));
bzero(a_ack,sizeof(a_ack));
bzero(rec,sizeof(rec));
recv(cd,rec,sizeof(rec),0);
printf("%s\n",rec);
bzero(rec,sizeof(rec));
char c_username[100];
char c_password[100];
bzero(c_username,sizeof(c_username));
bzero(c_password,sizeof(c_password));
printf("Enter the Username: ");
scanf("%s",c_username);
printf("Enter the password: ");
scanf("%s",c_password);
send(cd,c_username,strlen(c_username)+1,0);
recv(cd,a_ack,sizeof(a_ack),0);
send(cd,c_password,strlen(c_password)+1,0);
recv(cd,a_ack1,sizeof(a_ack1),0);
int log_c;
recv(cd,&log_c,sizeof(int),0);
if(log_c==1){
printf("Logged in Successfully\n");
while(1){
            printf("\n=====================================================================\n"
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
                    c_username);
int c_choice;
scanf("%d",&c_choice);
send(cd,&c_choice,sizeof(int),0);
switch(c_choice){
case 1:
time_t ct;
time(&ct);
char c_b[100];
bzero(c_b,sizeof(c_b));
recv(cd,c_b,sizeof(c_b),0);
printf("Account: %s\n ",c_username);
printf("Balance: %s\n",c_b);
printf("Viewed on: %s\n",ctime(&ct));
break;

case 2:
char d_money[100];
bzero(d_money,sizeof(d_money));
printf("Enter amount to deposit:");
scanf("%s",d_money);
send(cd,d_money,sizeof(d_money),0);
printf("Deposit initiated\n");
char rd_money[100];
bzero(rd_money,sizeof(rd_money));
recv(cd,rd_money,sizeof(rd_money),0);
usleep(5);
printf("Deposit successful\n");
printf("New Balance: %s\n",rd_money);
break;

case 3:
char w_money[100];
bzero(w_money,sizeof(w_money));
printf("Enter amount to withdraw:");
scanf("%s",w_money);
send(cd,w_money,sizeof(w_money),0);
printf("Withdrawl initiated\n");
char rw_money[100];
bzero(rw_money,sizeof(rw_money));
recv(cd,rw_money,sizeof(rw_money),0);
usleep(5);
printf("Withdrawl successful\n");
printf("New Balance: %s\n",rw_money);
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
printf("Enter account name to transfer to: \n");
scanf("%s",cs_username);
printf("Enter amount to transfer: \n");
scanf("%s",cs_amount);
send(cd,cs_username,strlen(cs_username)+1,0);
recv(cd,ack,sizeof(ack),0);
printf("Transfering .....\n");
send(cd,cs_amount,strlen(cs_amount)+1,0);
recv(cd,ack1,sizeof(ack1),0);
int c_check;
recv(cd,&c_check,sizeof(int),0);
if(c_check==0){printf("Transfer Succesfull\n");}
else printf("Failed transfer\n ");
break;




case 5:
char l_amount[100];
bzero(l_amount,sizeof(l_amount));
printf("Enter the loan amount: ");
scanf("%s",l_amount);
send(cd,l_amount,strlen(l_amount)+1,0);
int l_lcheck;
recv(cd,&l_lcheck,sizeof(int),0);
if(l_lcheck==1) printf("Loan successfuly applied\n");
else printf("Loan application failed\n");
break;

case 6:
char view_loan_buffer[50000];
bzero(view_loan_buffer,sizeof(view_loan_buffer));
recv(cd,view_loan_buffer,sizeof(view_loan_buffer),0);
usleep(10);
printf("%s\n",view_loan_buffer);
usleep(5);
break;

case 7:
char inputbuffer[5000];
char feedback[5000];
bzero(feedback,sizeof(feedback));
bzero(inputbuffer,sizeof(inputbuffer));
printf("Please give your feedback:");
getchar(); 
fgets(feedback, sizeof(feedback), stdin); 
strcpy(inputbuffer,feedback);
printf("Your feedback registered %s\n",inputbuffer);
send(cd,inputbuffer,sizeof(inputbuffer),0);
usleep(10);
int f_check;
recv(cd,&f_check,sizeof(int),0);
if (f_check==1) printf("Feedback Taken successfully\n");
else printf("Failed to take feedback\n");
break;

case 8:
char s_passbk[40000];
bzero(s_passbk,sizeof(s_passbk));
recv(cd,s_passbk,sizeof(s_passbk),0);
usleep(10);
printf("%s\n",s_passbk);
usleep(5);
break;





case 9:
printf("Logged out\n");
return;
default:
printf("Invalid choice\n");
break;

}
}


}
else{
    printf("Invalid Credentials\n");
}


}
void handle_employee(int cd){
char rec[100];
char a_ack[100];
char a_ack1[100];
printf("Initiated as Employee\n");
bzero(a_ack1,sizeof(a_ack));
bzero(a_ack,sizeof(a_ack));
bzero(rec,sizeof(rec));
recv(cd,rec,sizeof(rec),0);
printf("%s\n",rec);
bzero(rec,sizeof(rec));
char e_username[100];
char e_password[100];
bzero(e_username,sizeof(e_username));
bzero(e_password,sizeof(e_password));
printf("Enter the Username: ");
scanf("%s",e_username);
printf("Enter the password: ");
scanf("%s",e_password);
send(cd,e_username,strlen(e_username)+1,0);
recv(cd,a_ack,sizeof(a_ack),0);
send(cd,e_password,strlen(e_password)+1,0);
recv(cd,a_ack1,sizeof(a_ack1),0);
int log_e;
recv(cd,&log_e,sizeof(int),0);
if(log_e==1){
printf("Logged in successfully\n");
while(1){
printf(           "\n=====================================================================\n"
                    "Welcome, Employee %s!\n"
                    "=====================================================================\n"
                    "OPTIONS:\n"
                    "1. Add Customer                                                        |\n"
                    "----------------------------                                           |\n"
                    "2. Activate/Deactivate Customer                                        |\n"
                    "----------------------------                                           |\n"
                    "3. View Passbook                                                       |\n"
                    "----------------------------                                           |\n"
                    "4. Approve loan                                                        |\n"
                    "----------------------------                                           |\n"
                    "5. Logout                                                              |\n"
                    "=====================================================================\n"
                    "Enter your choice: ",
                    e_username);
int c_choice;
scanf("%d",&c_choice);
send(cd,&c_choice,sizeof(int),0);
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
printf("Enter username : ");
scanf("%s",u_username);
printf("Enter password: ");
scanf("%s",u_password);
printf("Confirming Inputs:\n");
printf("Username:%s\n",u_username);
printf("Password: %s\n",u_password);
send(cd,u_username,strlen(u_username),0);
recv(cd,u_ack,sizeof(u_ack),0);
send(cd,u_password,strlen(u_password),0);
recv(cd,u_ack1,sizeof(u_ack1),0);
printf("New User Added\n");
break;


case 2:
char ac_username[100];
char ack[100];
int a_d;
char ack1[100];
bzero(ac_username,sizeof(ac_username));
bzero(ack,sizeof(ack));
bzero(ack1,sizeof(ack1));
printf("Enter the username: ");
scanf("%s",ac_username);
printf("Enter 0 to deactivate , Enter 1 to activate: ");
scanf("%d",&a_d);
send(cd,ac_username,strlen(ac_username),0);
recv(cd,ack,sizeof(ack),0);
send(cd,&a_d,sizeof(int),0);
int act_check;
recv(cd,&act_check,sizeof(int),0);
if(act_check==1){ printf("Task Completed\n");}
else printf("Failed to change details\n");
break;

case 3:
char pss_username[100];
bzero(pss_username,sizeof(pss_username));
printf("Enter the username of the passbook holder : ");
scanf("%s",pss_username);
send(cd,pss_username,strlen(pss_username),0);
printf("Requesting Information:\n");
char s1_passbk[40000];
bzero(s1_passbk,sizeof(s1_passbk));
recv(cd,s1_passbk,sizeof(s1_passbk),0);
usleep(10);
printf("%s\n",s1_passbk);
usleep(5);
break;


case 5:
printf("Logged out\n");
return;
default:
printf("Invalid choice\n");
break;

}


}
}
else {printf("Invalid credentials\n");}
}

void handle_manager(int cd){
char rec[100];
char a_ack[100];
char a_ack1[100];
printf("Initiated as Employee\n");
bzero(a_ack1,sizeof(a_ack));
bzero(a_ack,sizeof(a_ack));
bzero(rec,sizeof(rec));
recv(cd,rec,sizeof(rec),0);
printf("%s\n",rec);
bzero(rec,sizeof(rec));
char m_username[100];
char m_password[100];
bzero(m_username,sizeof(m_username));
bzero(m_password,sizeof(m_password));
printf("Enter the Username: ");
scanf("%s",m_username);
printf("Enter the password: ");
scanf("%s",m_password);
send(cd,m_username,strlen(m_username)+1,0);
recv(cd,a_ack,sizeof(a_ack),0);
send(cd,m_password,strlen(m_password)+1,0);
recv(cd,a_ack1,sizeof(a_ack1),0);
int log_m;
recv(cd,&log_m,sizeof(int),0);
if(log_m==1){
printf("Logged in successfully\n");
while(1){
    printf(       "\n=====================================================================\n"
                    "Welcome, ADMIN %s!\n"
                    "=====================================================================\n"
                    "OPTIONS:\n"
                    "1. View Feedback                                                       |\n"
                    "----------------------------                                           |\n"
                    "2. Assign Loan                                                         |\n"
                    "----------------------------                                           |\n"
                    "3. Promotion                                                           |\n"
                    "----------------------------                                           |\n"
                    "4. Activate/Deactivate Employee                                        |\n"
                    "----------------------------                                           |\n"
                    "5. Logout                                                              |\n"
                    "=====================================================================\n"
                    "Enter your choice: ",
                    m_username);
int c_choice;
scanf("%d",&c_choice);
send(cd,&c_choice,sizeof(int),0);
switch(c_choice){
case 5:
printf("Logged out\n");
return;
default:
printf("Invalid choice\n");
break;

}
}
}
else {printf("Invalid credentials\n");}
}

void server_handler(int cd){

    struct USER loginUser;
    struct ACCOUNT userAccount;
    printf("Server has connected to the server!\n");
    while(1){
    printf("=====================================================================\n"
            "\t WELCOME TO BANKING PORTAL \t\n"
            "=====================================================================|\n"
            "MENU                                                                 |\n"
            "1. Customer Login                                                    |\n"
            "----------------------------                                         |\n"
            "2. Employee Login                                                    |\n"
            "---------------------------                                          |\n"
            "3. Manager  Login                                                    |\n"
            "----------------------------                                         |\n"
            "4. Admin  Login                                                      |\n"
            "----------------------------                                         |\n"
            "5. Exit                                                              |\n"
            "=====================================================================|\n"
            "Please Make a Choice:[ ");
            int choice;
            scanf("%d",&choice);
            send(cd,&choice,sizeof(choice),0);
            switch(choice){
            case 1:
            handle_user(cd);
            break;
            case 2:
            handle_employee(cd);
            break;
            case 3:
            handle_manager(cd);
            break;
            case 4:
            handle_admin(cd);
            break;
            case 5:
            printf("Thanks for using our service");
            return;

            default:
            printf("Invalid Choice\n");
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