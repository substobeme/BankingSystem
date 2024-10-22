/* ADMIN 0, MANAGER 1, EMPLOYEE 2, CUSTOMER 3*/
#include<time.h>
struct USER{
char username[100];
unsigned long password;
int role;
int session; // 0: Not logged in, 1: logged in
int status;// 0:Non- Active Account, 1: Active Account
};

struct ACCOUNT{
char username[100];
float balance;};


struct LOAN{
char c_username[100];
char e_username[100];
time_t time;
float loan;
int status; // 0:Accepted  1:Rejected   2:Pending
};

struct FEEDBACK{
char username[100];
char feedback[1700];
time_t tme;};

struct TRANSACTION{
char username[50];
time_t tme;
int flag; // 0: Minus  1 : Plus
float amount;
float total_amount;
};