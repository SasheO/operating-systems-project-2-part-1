#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

void ChildProcess(int*, sem_t*);
void  ParentProcess(int*, sem_t*);
void  DepositMoney();

int  main(int  argc, char *argv[]){
  int    BankAcctID;
  int    *BankAcctPtr;
  pid_t  pid;
  sem_t *mutex;

  // provision shared memory for BankAcctID, see any errors in allocating shared memory
  BankAcctID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  if (BankAcctID < 0) {
      printf("*** shmget error ***\n");
      exit(1);
  }

    /* create, initialize semaphore */
  if ((mutex = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }

  // set BankAcctPtr pointer to the shared memory provisioned
  BankAcctPtr = (int *) shmat(BankAcctID, NULL, 0);
  if (*BankAcctPtr == -1) {
      printf("*** shmat error ***\n");
      exit(1);
  }
  

  *BankAcctPtr = 0; // initialize value to 0

  printf("Orig Bank Account = %d\n", *BankAcctPtr);

  pid = fork();
  if (pid < 0) { 
      printf("*** fork error ***\n");
      exit(1);
  }
  else if (pid == 0) {
      ChildProcess(BankAcctPtr, mutex);
      // exit(0);
  }
  else{
    ParentProcess(BankAcctPtr, mutex);
  }

  return 0;
}


void  ChildProcess(int  *BankAcctPtr, sem_t *mutex){
  srandom(time(NULL));
  int sleep_time, account, balance_needed, indx;
  for (indx=0; indx < 25; indx ++){ // loop 25 times as per the lab prompt
    sleep_time = random()%5+1;
    sleep(sleep_time); // sleep random amount of time between 1-5 seconds
    sem_wait(mutex); // acquire lock/semaphore
    account = *BankAcctPtr; // copy the in BankAccount to a local variable account
    // student tries to withdraw random amount of money if they have enough in their bank account
    balance_needed = random()%50; 
    printf("Poor Student needs $%d\n", balance_needed);
    if (balance_needed<=account){
      account -= balance_needed;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance_needed, account);
    }
    else{
      printf("Poor Student: Not Enough Cash ($%d)\n", account );
    }

    * BankAcctPtr = account; // update shared variable BankAcctPtr with locat account variable
    sem_post(mutex); // release semaphore
  }
}

void  ParentProcess(int * BankAcctPtr, sem_t *mutex){
  srandom(time(NULL));
  int sleep_time, indx;
  int * accountPTR;
  int account; // local variable for bank account amount
  for (indx=0; indx < 25; indx ++){  // loop 25 times as per the lab prompt
    accountPTR = &account; // accountPTR pointer holds the address of account integer (effectively points to accout)
    sleep_time = random()%5+1;
    sleep(sleep_time); // sleep random amount of time between 1-5 seconds
    sem_wait(mutex); // acquire lock/semaphore
    printf("Dear Old Dad: Attempting to Check Balance\n");
    account = *BankAcctPtr; // copy the in BankAccount to a local variable account
    if (account <= 100){
      DepositMoney(accountPTR); // edits account variable using its pointer
      *BankAcctPtr = account; // update shared variable BankAcctPtr with locat account variable
    }
    else{
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
    }
    sem_post(mutex); // release semaphore
  }
}

void DepositMoney(int *accountPTR){
  /*
  Takes in int accountPTR pointer, generates a number <100.
  if random number is even, it modifies the value in accountPTR and prints message.
  otherwise prints message
  */
  srandom(time(NULL));
  int deposit_amount = random()%100;
  if (deposit_amount%2==0){
    * accountPTR += deposit_amount;
    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", deposit_amount, *accountPTR);
  }
  else{
    printf("Dear old Dad: Doesn't have any money to give\n");
  }
}