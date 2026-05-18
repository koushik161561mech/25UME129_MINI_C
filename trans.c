// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 100

struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void listRecords(FILE *fPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
int readAccountNumber(unsigned int *account, const char *prompt);
void initializeFile(FILE *fPtr);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // try to open existing file; create it if missing
    if ((cfPtr = fopen("credit.dat", "r+b")) == NULL)
    {
        if ((cfPtr = fopen("credit.dat", "w+b")) == NULL)
        {
            printf("%s: File could not be opened.\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        initializeFile(cfPtr);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // list all accounts
        case 5:
            listRecords(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    return 0;
} // end main

void initializeFile(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};

    rewind(fPtr);
    for (unsigned int i = 0; i < MAX_RECORDS; i++)
    {
        if (fwrite(&blankClient, sizeof(struct clientData), 1, fPtr) != 1)
        {
            puts("Error initializing file.");
            exit(EXIT_FAILURE);
        }
    }
}

int readAccountNumber(unsigned int *account, const char *prompt)
{
    printf("%s", prompt);
    if (scanf("%u", account) != 1)
    {
        puts("Invalid input.");
        while (getchar() != '\n' && !feof(stdin))
            ;
        return 0;
    }

    if (*account < 1 || *account > MAX_RECORDS)
    {
        printf("Account number must be between 1 and %u.\n", MAX_RECORDS);
        return 0;
    }

    return 1;
}

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    rewind(readPtr); // sets pointer to beginning of file
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                    client.acctNum, client.lastName, client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr); // fclose closes the file
    puts("accounts.txt generated.");
} // end function textFile

void listRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int found = 0;

    rewind(fPtr);
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n",
                   client.acctNum, client.lastName, client.firstName, client.balance);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No account records exist.");
    }
}

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    struct clientData client = {0, "", "", 0.0};

    if (!readAccountNumber(&account, "Enter account to update (1-100): "))
    {
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Error reading account.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    {
        printf("%-6u%-16s%-11s%10.2f\n\n", client.acctNum,
               client.lastName, client.firstName, client.balance);

        printf("%s", "Enter charge (+) or payment (-): ");
        if (scanf("%lf", &transaction) != 1)
        {
            puts("Invalid amount.");
            while (getchar() != '\n' && !feof(stdin))
                ;
            return;
        }

        client.balance += transaction;

        printf("%-6u%-16s%-11s%10.2f\n", client.acctNum,
               client.lastName, client.firstName, client.balance);

        fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    unsigned int accountNum;                        // account number
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0.0}; // blank client

    if (!readAccountNumber(&accountNum, "Enter account number to delete (1-100): "))
    {
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Error reading account.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    }
    else
    {
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        puts("Account deleted.");
    }
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number

    if (!readAccountNumber(&accountNum, "Enter new account number (1-100): "))
    {
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        puts("Error reading account.");
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
    }
    else
    {
        printf("%s", "Enter lastname, firstname, balance\n? ");
        if (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3)
        {
            puts("Invalid input.");
            while (getchar() != '\n' && !feof(stdin))
                ;
            return;
        }

        client.acctNum = accountNum;
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        puts("Account created.");
    }
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - list all accounts\n"
                 "6 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1)
    {
        while (getchar() != '\n' && !feof(stdin))
            ;
        return 0;
    }

    return menuChoice;
} // end function enterChoice