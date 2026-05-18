// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 100
#define LAST_NAME_SIZE 15
#define FIRST_NAME_SIZE 10
#define INPUT_BUFFER_SIZE 128

struct clientData
{
    unsigned int acctNum; // account number
    char lastName[LAST_NAME_SIZE];    // account last name
    char firstName[FIRST_NAME_SIZE];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void printHeader(void);
int readAccountNumber(unsigned int *account, const char *prompt);
int readDouble(double *value, const char *prompt);
int readRecord(FILE *fPtr, unsigned int account, struct clientData *client);
int writeRecord(FILE *fPtr, unsigned int account, const struct clientData *client);
void initializeFile(FILE *fPtr);
void clearInput(void);
void textFile(FILE *readPtr);
void listRecords(FILE *fPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    if ((cfPtr = fopen("credit.dat", "r+b")) == NULL)
    {
        if ((cfPtr = fopen("credit.dat", "w+b")) == NULL)
        {
            fprintf(stderr, "%s: File could not be opened.\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        initializeFile(cfPtr);
    }

    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 5:
            listRecords(cfPtr);
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }

    fclose(cfPtr);
    return 0;
}

void initializeFile(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};

    rewind(fPtr);
    for (unsigned int i = 0; i < MAX_RECORDS; ++i)
    {
        if (fwrite(&blankClient, sizeof(struct clientData), 1, fPtr) != 1)
        {
            fputs("Error initializing file.\n", stderr);
            exit(EXIT_FAILURE);
        }
    }
    fflush(fPtr);
}

void clearInput(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

int readAccountNumber(unsigned int *account, const char *prompt)
{
    char buffer[INPUT_BUFFER_SIZE];
    char *endptr;
    unsigned long value;

    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        puts("Input error.");
        return 0;
    }

    value = strtoul(buffer, &endptr, 10);
    if (endptr == buffer || (*endptr != '\n' && *endptr != '\0'))
    {
        puts("Invalid input.");
        return 0;
    }

    if (value < 1 || value > MAX_RECORDS)
    {
        printf("Account number must be between 1 and %u.\n", MAX_RECORDS);
        return 0;
    }

    *account = (unsigned int)value;
    return 1;
}

int readDouble(double *value, const char *prompt)
{
    char buffer[INPUT_BUFFER_SIZE];
    char *endptr;

    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        puts("Input error.");
        return 0;
    }

    *value = strtod(buffer, &endptr);
    if (endptr == buffer || (*endptr != '\n' && *endptr != '\0'))
    {
        puts("Invalid number.");
        return 0;
    }

    return 1;
}

long recordOffset(unsigned int account)
{
    return (long)(account - 1) * sizeof(struct clientData);
}

int readRecord(FILE *fPtr, unsigned int account, struct clientData *client)
{
    if (fseek(fPtr, recordOffset(account), SEEK_SET) != 0)
    {
        return 0;
    }

    return fread(client, sizeof(struct clientData), 1, fPtr) == 1;
}

int writeRecord(FILE *fPtr, unsigned int account, const struct clientData *client)
{
    if (fseek(fPtr, recordOffset(account), SEEK_SET) != 0)
    {
        return 0;
    }

    if (fwrite(client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        return 0;
    }

    fflush(fPtr);
    return 1;
}

void printHeader(void)
{
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    rewind(readPtr);
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

    fclose(writePtr);
    puts("accounts.txt generated.");
}

void listRecords(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int found = 0;

    rewind(fPtr);
    printHeader();

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

void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    if (!readAccountNumber(&account, "Enter account to update (1-100): "))
    {
        return;
    }

    if (!readRecord(fPtr, account, &client))
    {
        puts("Error reading the account record.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("%-6u%-16s%-11s%10.2f\n\n",
           client.acctNum, client.lastName, client.firstName, client.balance);

    if (!readDouble(&transaction, "Enter charge (+) or payment (-): "))
    {
        return;
    }

    client.balance += transaction;

    if (!writeRecord(fPtr, account, &client))
    {
        puts("Error updating the account.");
        return;
    }

    printf("Updated account: %-6u%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);
}

void deleteRecord(FILE *fPtr)
{
    unsigned int accountNum;
    struct clientData client = {0, "", "", 0.0};
    struct clientData blankClient = {0, "", "", 0.0};

    if (!readAccountNumber(&accountNum, "Enter account number to delete (1-100): "))
    {
        return;
    }

    if (!readRecord(fPtr, accountNum, &client))
    {
        puts("Error reading the account record.");
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
        return;
    }

    if (!writeRecord(fPtr, accountNum, &blankClient))
    {
        puts("Error deleting the account.");
        return;
    }

    puts("Account deleted.");
}

void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;
    char input[INPUT_BUFFER_SIZE];

    if (!readAccountNumber(&accountNum, "Enter new account number (1-100): "))
    {
        return;
    }

    if (!readRecord(fPtr, accountNum, &client))
    {
        puts("Error reading the account record.");
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
        return;
    }

    printf("Enter lastname, firstname, balance\n? ");
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        puts("Invalid input.");
        return;
    }

    if (sscanf(input, "%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3)
    {
        puts("Invalid input.");
        return;
    }

    client.acctNum = accountNum;
    if (!writeRecord(fPtr, accountNum, &client))
    {
        puts("Error writing the account record.");
        return;
    }

    puts("Account created.");
}

unsigned int enterChoice(void)
{
    char buffer[INPUT_BUFFER_SIZE];
    char *endptr;
    unsigned long value;

    printf("\nEnter your choice\n"
           "1 - store a formatted text file of accounts called\n"
           "    \"accounts.txt\" for printing\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - list all accounts\n"
           "6 - end program\n? ");

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        return 0;
    }

    value = strtoul(buffer, &endptr, 10);
    if (endptr == buffer || (*endptr != '\n' && *endptr != '\0'))
    {
        return 0;
    }

    return (unsigned int)value;
}
