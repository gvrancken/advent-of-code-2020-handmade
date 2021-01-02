#include <stdio.h> // printf
#include <stdbool.h>
#include <stdlib.h>         // malloc
#include <string.h>         // memcpy
#include <stdint.h>         // types
#include <mach/mach_init.h> // for time measurements
#include <mach/mach_time.h> // for time measurements

#define ArrayCount(Array) ((sizeof(Array) / sizeof((Array)[0])))
#define Assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }

#define Min(a, b) (a < b) ? a : b
#define Max(a, b) (a > b) ? a : b

#define u64 uint64_t

typedef struct String
{
    char *data;
    u64 length;
} String;

typedef struct StringArray
{
    String str[4096];
    u64 count;
} StringArray;

static String
ReadFileToHeap(char *Filename)
{
    String result = {};

    void *fileHandle = fopen(Filename, "r");

    if (!fileHandle)
        return result;

    fseek(fileHandle, 0, SEEK_END);
    u64 fileSize = (u64)ftell(fileHandle);
    if (!fileSize)
    {
        Assert(0);
        return result;
    }

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

static bool
IsNumber(char value)
{
    int n = value - '0';
    return (n >= 0 && n <= 9);
}

static void SplitString(String *input, StringArray *lines, char splitChar)
{

    u64 lineIndex = lines->count++;
    lines->str[lineIndex].data = input->data;

    for (u64 i = 0; i < input->length; i++)
    {
        char ch = input->data[i];

        if (ch == splitChar)
        {
            lines->str[lineIndex].length = (u64)(input->data + i - lines->str[lineIndex].data); // end line start new one
            lineIndex = lines->count++;
            lines->str[lineIndex].data = input->data + i + 1;
        }
    }
    lines->str[lineIndex].length = (u64)(input->data + input->length - lines->str[lineIndex].data);
}

#define For(n, count) for (int n = 0; n < count; n++)

enum Operator
{
    None,
    Add,
    Subtract,
    Multiply,
    Divide
};

typedef struct Statement
{
    enum Operator op[16];
    int values[16];
    int valueCount;
} Statement;

static u64 CalcStatement(Statement *statement) {
    u64 result = statement->values[0];
    
    for (int valueIndex=1; 
        valueIndex < statement->valueCount; 
        valueIndex++)
    {
        u64 b = statement->values[valueIndex];
        enum Operator op = statement->op[valueIndex];
        if (op == Add)
        {
            result += b;
        }
        else if (op == Multiply)
        {
            result *= b;
        }
        else if (op == Divide)
        {
            result /= b;
        }
        else if (op == Subtract)
        {
            result -= b;
        }
    }

    return result;
}

int main()
{

    String input = ReadFileToHeap("input.txt");

    // Time
    mach_timebase_info_data_t tb;
    mach_timebase_info(&tb);
    u64 global_nanoseconds_per_tick = tb.numer / tb.denom;
    u64 timerStart = mach_absolute_time();

    // Program
    StringArray lines = {};
    SplitString(&input, &lines, '\n');
    
    u64 total = 0;

    For(i, lines.count)
    {
        Statement stack[64] = {};

        String *line = lines.str + i;

        int currentStack = 0;

        For(j, line->length)
        {
            Statement *statement = stack + currentStack;
            char it = line->data[j];

            if (it == ' ')
            {
                continue;
            }
            else if (it == '(')
            {
                currentStack++;
            }
            else if (it == ')')
            {
                // resolve this to an int
                u64 result = CalcStatement(statement);
                
                printf("Result: %llu\n", result);

                statement->valueCount = 0;
                currentStack--;
                Statement *newS = stack + currentStack;
                newS->values[newS->valueCount++] = result;
            }
            else if (it == '+')
            {
                statement->op[statement->valueCount] = Add;
            }
            else if (it == '-')
            {
                statement->op[statement->valueCount] = Subtract;
            }
            else if (it == '*')
            {
                statement->op[statement->valueCount] = Multiply;
            }
            else if (it == '/')
            {
                statement->op[statement->valueCount] = Divide;
            }
            else
            {
                // seems numbers are only one digit, oh well...
                u64 num = 0;
                while (true)
                {
                    if (!IsNumber(line->data[j])) {
                        j--;
                        break;
                    }
                    num *= 10;
                    num += line->data[j] - '0';
                    j++;
                }
                
                int index = statement->valueCount++;
                statement->values[index] = num;
                
            }
        }

        Statement *statement = stack + 0;
        u64 answer = CalcStatement(statement);
        printf("Answer: %llu\n\n", answer);

        total += answer;
    }

    printf("Total: %llu\n\n", total);

    // Time
    u64 timerEnd = mach_absolute_time();
    u64 dTicks = timerEnd - timerStart;
    u64 nanoseconds = dTicks * global_nanoseconds_per_tick;
    float ms = (float)(nanoseconds * 1.0E-6);

    printf("It took %f ms.\n", ms);
}
