#include <stdio.h> // printf
#include <stdbool.h>
#include <stdlib.h> // malloc

#define ArrayCount(Array) ((sizeof(Array) / sizeof((Array)[0])))
#define Assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }

typedef struct debug_read_file_result_t
{
    size_t contentsSize;
    void *contents;
} debug_read_file_result_t;

static debug_read_file_result_t
ReadFileToHeap(char *Filename)
{
    debug_read_file_result_t result = {};

    void *fileHandle = fopen(Filename, "r");

    if (!fileHandle)
        return result;

    fseek(fileHandle, 0, SEEK_END);
    size_t fileSize = ftell(fileHandle);
    if (!fileSize)
        return result;

    rewind(fileHandle);
    result.contents = malloc(fileSize);

    if (result.contents)
    {
        size_t bytesRead = fread(result.contents, 1, fileSize, fileHandle);
        if (fileSize == bytesRead)
        {
            // File read successfully
            result.contentsSize = fileSize;
        }
    }

    fclose(fileHandle);

    return (result);
}

static bool CompareString(char *a, char *b)
{
    while (true)
    {
        if (*a == 0 && *b == 0)
            break;
        if (*a == 0 || *b == 0)
            return false;
        if (*a++ != *b++)
            return false;
    }

    return true;
}

typedef struct String
{
    char *data;
    int length;
} String;

int main()
{
    debug_read_file_result_t result = ReadFileToHeap("input.txt");

    char *input = (char *)result.contents;

    String passports[512] = {};
    int passportCount = 0;

    int lastOffset = 0;
    for (int i = 0; i < result.contentsSize; ++i)
    {
        char *curr = input + i;
        char *prev = input + (i - 1);

        if ((*curr == '\n' && *prev == '\n') || i == result.contentsSize - 1)
        {
            String pass;
            pass.data = input + lastOffset;
            pass.length = i - lastOffset;
            passports[passportCount++] = pass;
            lastOffset = i + 1; // skip the newlines at the start
        }
    }

    printf("Number of passport total: %d\n", passportCount);

    int validCount = 0;
    int invalidCount = 0;

    for (int i = 0; i < passportCount; ++i)
    {

        String *passport = passports + i;

        int foundCats = 0;

        printf("------------------\n");
        for (int j = 0; j < passport->length; ++j)
        {
            // printf("%c", passport->data[j]);
            char ch = passport->data[j];

            char cat[4];

            if (ch == ':')
            {
                cat[0] = passport->data[j - 3];
                cat[1] = passport->data[j - 2];
                cat[2] = passport->data[j - 1];
                cat[3] = 0;

                printf("%s\n", cat);
                if (CompareString(cat, "byr"))
                {
                    foundCats++;
                }
                else if (CompareString(cat, "iyr"))
                {
                    foundCats++;
                }
                else if (CompareString(cat, "eyr"))
                {
                    foundCats++;
                }
                else if (CompareString(cat, "hgt"))
                {
                    foundCats++;
                }
                else if (CompareString(cat, "hcl"))
                {
                    foundCats++;
                }
                else if (CompareString(cat, "ecl"))
                {
                    foundCats++;
                }
                else if (CompareString(cat, "pid"))
                {
                    foundCats++;
                }
                // else if (CompareString(cat, "cid"))
                // {
                //     foundCats++;
                // }
            }
        }
        printf("\n");
        printf("foundCats: %d\n", foundCats);
        if (foundCats < 7) {
            invalidCount++;
        } else {
            validCount++;
        }
    }

    printf("%d passports are valid\n%d are invalid.\n", validCount, invalidCount);
}