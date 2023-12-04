#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_SLEEP 5
#define MAX_DEPOSIT_AMOUNT 100
#define MAX_WITHDRAW_AMOUNT 50


void DearOldDad(int *BankAccount, int *Turn);
void PoorStudent(int *BankAccount, int *Turn);

int main(){
  int ShmID;
  int *BankAccount;
  int *Turn;
  pid_t pid;
  int status;

  //create a shared memory for BankAccount and Turn
  ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
    perror("shmget");
    exit(1);
  }

  BankAccount = (int *) shmat(ShmID, NULL, 0);
  Turn = BankAccount + 1; //second int in the shared memory is Turn

  //initialise the variables to 0
  *BankAccount = 0;
  *Turn = 0;

  pid = fork();
  if (pid < 0){
    perror("fork");
    exit(1);
  }
  else if (pid == 0){
    //Child process
    PoorStudent(BankAccount, Turn);
  }
  else{
    //Parent process
    DearOldDad(BankAccount, Turn);

    //wait for child process to finish
    wait(&status);

    //detach and remove shared memory
    shmdt((void *)BankAccount);
    shmctl(ShmID, IPC_RMID, NULL);
  }

  return 0;

}

void DearOldDad(int *BankAccount, int *Turn){
  //Parent Process
  int account, i;

  for (i = 0; i <25; i++){
    //sleep for a random amout of time between 0-5s
    sleep(rand() & (MAX_SLEEP + 1));

    account = *BankAccount;

    while (*Turn != 0); //busy-waiting for Turn to be 0

    if (account <= 100){
      //randomly generate a balance between 0-100
      int balance = rand() & (MAX_DEPOSIT_AMOUNT + 1);

      if (balance % 2 == 0){
        //if the random balance is even, deposit it to the account
        *BankAccount += balance;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, *BankAccount);
      }
      else{
        //if the random balance is odd
        printf("Dear old Dad: Doesn't have any money to give\n");
      }

      account = *BankAccount;
      *Turn = 1;
    }
    else{
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
    }
  }
}

void PoorStudent(int *BankAccount, int *Turn){
  //Child Process
  int account, i;

  for (i = 0; i < 25; i ++){
    //sleep for a random amount of time between 0-5s
    sleep(rand() & (MAX_SLEEP + 1));

    account = *BankAccount;

    while (*Turn != 1); //busy-waiting for Turn to be 1

    //randomly generate a balance between 0-50
    int balance = rand() % (MAX_WITHDRAW_AMOUNT + 1);
    printf("Poor Student needs $%d\n", balance);

    if (balance <= account){
      *BankAccount -= balance;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, *BankAccount);
    }
    else{
      printf("Poor Student: Not Enough Cash ($%d)\n", account);
    }

    account = *BankAccount;
    *Turn = 0;
  }
  
}
