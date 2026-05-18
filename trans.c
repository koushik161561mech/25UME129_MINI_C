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
#define DATA_FILE "credit.dat"
#define OUTPUT_FILE "accounts.txt"
#define RECORD_SIZE sizeof(struct clientData)
#define RECORDS_FILE_SIZE (MAX_RECORDS * RECORD_SIZE)

struct clientData
{
    unsigned int acctNum; // account number
    char lastName[LAST_NAME_SIZE];    // account last name
    char firstName[FIRST_NAME_SIZE];   // account first name
    double balance;       // account balance
};                        // end structure clientData

static struct clientData accounts[MAX_RECORDS];
static const struct clientData blankClient = {0, "", "", 0.0};

// prototypes
unsigned int enterChoice(void);
void printHeader(void);
int readAccountNumber(unsigned int *account, const char *prompt);
int readDouble(double *value, const char *prompt);
int readRecord(unsigned int account, struct clientData *client);
int writeRecord(FILE *fPtr, unsigned int account, const struct clientData *client);
int ensureFileSize(FILE *fPtr);
void loadRecords(FILE *fPtr);
void textFile(void);
void listRecords(void);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    if ((cfPtr = fopen(DATA_FILE, "r+b")) == NULL)
    {
        if ((cfPtr = fopen(DATA_FILE, "w+b")) == NULL)
        {
            fprintf(stderr, "%s: File could not be opened.\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (setvbuf(cfPtr, NULL, _IOFBF, 8192) != 0)
    {
        fputs("Warning: failed to set file buffer.\n", stderr);
    }

    if (!ensureFileSize(cfPtr))
    {
        fputs("Could not initialize or extend data file.\n", stderr);
        fclose(cfPtr);
        exit(EXIT_FAILURE);
    }

    loadRecords(cfPtr);

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

int ensureFileSize(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};
    long currentSize;

    if (fseek(fPtr, 0, SEEK_END) != 0)
    {
        return 0;
    }

    currentSize = ftell(fPtr);
    if (currentSize < 0)
    {
        return 0;
    }

    if (currentSize == RECORDS_FILE_SIZE)
    {
        return 1;
    }

    if (currentSize > RECORDS_FILE_SIZE)
    {
        return 1; // preserve larger files without truncating
    }

    if ((currentSize % RECORD_SIZE) != 0)
    {
        return 0;
    }

    if (fseek(fPtr, currentSize, SEEK_SET) != 0)
    {
        return 0;
    }

    while (currentSize < RECORDS_FILE_SIZE)
    {
        if (fwrite(&blankClient, RECORD_SIZE, 1, fPtr) != 1)
        {
            return 0;
        }
        currentSize += RECORD_SIZE;
    }

    fflush(fPtr);
    return 1;
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

static inline long recordOffset(unsigned int account)
{
    return (long)(account - 1) * RECORD_SIZE;
}

int readRecord(unsigned int account, struct clientData *client)
{
    if (account < 1 || account > MAX_RECORDS)
    {
        return 0;
    }

    *client = accounts[account - 1];
    return 1;
}

int writeRecord(FILE *fPtr, unsigned int account, const struct clientData *client)
{
    if (account < 1 || account > MAX_RECORDS)
    {
        return 0;
    }

    accounts[account - 1] = *client;

    if (fseek(fPtr, recordOffset(account), SEEK_SET) != 0)
    {
        return 0;
    }

    if (fwrite(client, RECORD_SIZE, 1, fPtr) != 1)
    {
        return 0;
    }

    fflush(fPtr);
    return 1;
}

void loadRecords(FILE *fPtr)
{
    rewind(fPtr);
    if (fread(accounts, RECORD_SIZE, MAX_RECORDS, fPtr) != MAX_RECORDS)
    {
        for (unsigned int i = 0; i < MAX_RECORDS; ++i)
        {
            accounts[i] = blankClient;
        }
    }
}

void printHeader(void)
{
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
}

void textFile(void)
{
    FILE *writePtr;

    if ((writePtr = fopen(OUTPUT_FILE, "w")) == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    for (unsigned int i = 0; i < MAX_RECORDS; ++i)
    {
        if (accounts[i].acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                    accounts[i].acctNum, accounts[i].lastName, accounts[i].firstName,
                    accounts[i].balance);
        }
    }

    fclose(writePtr);
    puts("accounts.txt generated.");
}

void listRecords(void)
{
    int found = 0;

    printHeader();

    for (unsigned int i = 0; i < MAX_RECORDS; ++i)
    {
        if (accounts[i].acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n",
                   accounts[i].acctNum, accounts[i].lastName, accounts[i].firstName, accounts[i].balance);
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
