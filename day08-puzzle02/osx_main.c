#include <stdio.h> // printf
#include <stdbool.h>
#include <stdlib.h> // malloc
#include <string.h> // memcpy

#define ArrayCount(Array) ((sizeof(Array) / sizeof((Array)[0])))
#define Assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }

typedef struct String
{
    char *data;
    int length;
} String;

static String
ReadFileToHeap(char *Filename)
{
    String result = {};

    void *fileHandle = fopen(Filename, "r");

    if (!fileHandle)
        return result;

    fseek(fileHandle, 0, SEEK_END);
    size_t fileSize = ftell(fileHandle);
    if (!fileSize)
        return result;

    rewind(fileHandle);
    result.data = (char *)malloc(fileSize);

    if (result.data)
    {
        size_t bytesRead = fread(result.data, 1, fileSize, fileHandle);
        if (fileSize == bytesRead)
        {
            // File read successfully
            result.length = fileSize;
        }
    }

    fclose(fileHandle);

    return (result);
}

static bool CompareChars(char *a, char *b, int length)
{
    while (length-- > 0)
    {
        if (*a == 0 && *b == 0)
            return true;
        if (*a == 0 || *b == 0)
            return false;
        if (*a++ != *b++)
            return false;
    }

    return true;
}

static bool CompareString(String a, String b)
{
    if (a.length != b.length)
        return false;

    for (int i = 0; i < a.length; i++)
    {
        if (a.data[i] != b.data[i])
            return false;
    }

    return true;
}

static bool IsNumber(char value)
{
    int n = value - '0';
    return (n >= 0 && n <= 9);
}

static bool IsAlphaNumeric(char value)
{
    return ((value >= '0' && value <= '9') ||
            (value >= 'a' && value <= 'z'));
}

static int ParseInt(char *str)
{
    int result = 0;

    char *p = str;
    while (*p != 0)
    {
        char number = *p;
        if (IsNumber(number))
        {
            result *= 10;
            result += number - '0';
            p++;
        }
        else
        {
            // NOTE(gb) Assert, return error
            return 0;
        }
    }

    return result;
}

static char *CString(String s)
{

    char *result = (char *)malloc(s.length + 1);

    for (int i = 0; i < s.length; i++)
    {
        result[i] = s.data[i];
    }
    result[s.length] = 0;

    return result;
}

enum Operation
{
    Operation_NOP,
    Operation_JMP,
    Operation_ACC,
};

typedef struct Command
{
    String str;
    enum Operation op;
    int amount;
    bool executedBefore;
} Command;

int main()
{
    String result = ReadFileToHeap("input.txt");

    Command commandList[4096];
    int commandCount = 0;

    int commandIndex = commandCount++;
    commandList[commandIndex].str.data = result.data;

    for (int i = 0; i < result.length; i++)
    {
        char ch = result.data[i];

        if (ch == '\n')
        {
            commandList[commandIndex].str.length = result.data + i - commandList[commandIndex].str.data;
            commandIndex = commandCount++;
            commandList[commandIndex].str.data = result.data + i + 1;
        }
        if (i == result.length - 1)
        {
            commandList[commandIndex].str.length = result.data + i + 1 - commandList[commandIndex].str.data;
        }
    }

    for (int f = 0; f < commandCount; ++f)
    {
        Command *it = commandList + f;

        if (CompareChars(it->str.data, "nop", 3))
        {
            it->op = Operation_NOP;
        }
        else if (CompareChars(it->str.data, "jmp", 3))
        {
            it->op = Operation_JMP;
        }
        else if (CompareChars(it->str.data, "acc", 3))
        {
            it->op = Operation_ACC;
        }

        char *p = it->str.data + 5;
        int amount = 0;

        while (IsNumber(*p))
        {
            amount *= 10;
            amount += *p - '0';
            p++;
        }

        char sign = it->str.data[4];
        if (sign == '-')
        {
            amount *= -1;
        }

        it->amount = amount;
        it->executedBefore = false;
    }

    
    int acc = 0;
    int runCount = 0;
    int currentSkip = 0;
    int commandNum = 0;
    
    int tries = 200; // safety net, not needed if the puzzle is correct.
    while (tries--)
    {

        if (commandNum > commandCount - 1)
            break;

        Command cleanList[4096] = {};
        memcpy(cleanList, commandList, commandCount * sizeof(Command));

        bool didSkip = false;
        commandNum = 0;
        currentSkip = 0;
        acc = 0;
        
        while (true)
        {

            Command *com = cleanList + commandNum;

            if (com->executedBefore)
            {
                // we failed
                printf("Repeating on commandNum %d.\n", commandNum);
                printf("Trying again...\n\n");
                runCount++;
                break;
            }

            if (commandNum > commandCount - 1)
                break;

            printf("%d: %d %d\n", commandNum, com->op, com->amount);

            com->executedBefore = true;

            // do the switcheroo

            if (currentSkip == runCount && (com->op == Operation_JMP || com->op == Operation_NOP))
            {
                if (com->op == Operation_JMP)
                {
                    com->op = Operation_NOP;
                }
                else if (com->op == Operation_NOP)
                {
                    com->op = Operation_JMP;
                }
                printf("--- CHANGED instruction %d. ---\n", commandNum);
            }

            if (com->op == Operation_JMP)
            {
                commandNum += com->amount;
                currentSkip++;
            }
            else if (com->op == Operation_ACC)
            {
                acc += com->amount;
                commandNum++;
            }
            else if (com->op == Operation_NOP)
            {
                commandNum++;
                currentSkip++;
            }
        }

        
    }

    printf("The accumulator contains %d.\n", acc);
    printf("It worked after %d tries.\n", runCount+1);
}
