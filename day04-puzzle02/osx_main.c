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

        bool isInvalid = false;

        int foundCats = 0;

        printf("------------------\n");
        for (int j = 0; j < passport->length; ++j)
        {

            char ch = passport->data[j];

            char cat[4];

            if (ch == ':')
            {
                // get three letter code before colon
                cat[0] = passport->data[j - 3];
                cat[1] = passport->data[j - 2];
                cat[2] = passport->data[j - 1];
                cat[3] = 0;

                printf("%s: ", cat);

                j++; // skip this colon char to get to the value

                if (CompareString(cat, "byr"))
                {
                    foundCats++;

                    char value[5]; // make a c-string

                    for (int n = 0; n < 4; n++)
                    {
                        char ch = passport->data[j++];
                        value[n] = ch;
                    }
                    value[4] = 0;
                    int number = ParseInt(value);
                    printf("%d", number);
                    if (number < 1920 || number > 2002)
                    {
                        isInvalid = true;
                        printf(" --- INVALID! --- ");
                        break;
                    }
                }
                else if (CompareString(cat, "iyr"))
                {
                    foundCats++;

                    char value[5]; // make a c-string

                    for (int n = 0; n < 4; n++)
                    {
                        char ch = passport->data[j++];
                        value[n] = ch;
                    }
                    value[4] = 0;
                    int number = ParseInt(value);
                    printf("%d", number);
                    if (number < 2010 || number > 2020)
                    {
                        isInvalid = true;
                        printf(" --- INVALID! --- ");
                        break;
                    }
                }
                else if (CompareString(cat, "eyr"))
                {
                    foundCats++;

                    char value[5]; // make a c-string

                    for (int n = 0; n < 4; n++)
                    {
                        char ch = passport->data[j++];
                        value[n] = ch;
                    }
                    value[4] = 0;
                    int number = ParseInt(value);

                    printf("%d", number);
                    if (number < 2020 || number > 2030)
                    {
                        isInvalid = true;
                        printf(" --- INVALID! --- ");
                        break;
                    }
                }
                else if (CompareString(cat, "hgt"))
                {
                    foundCats++;

                    char value[10] = {}; // make a c-string
                    int n = 0;

                    while (n < ArrayCount(value))
                    {
                        char ch = passport->data[j++];
                        if (ch == ' ' || ch == '\n')
                            break;
                        value[n++] = ch;
                    }
                    value[n] = 0;

                    printf("%s", value);

                    if (CompareString(value + (n - 2), "cm"))
                    {
                        if (n != 5)
                        {
                            isInvalid = true;
                            printf(" --- INVALID HEIGHT! --- ");
                            break;
                        }
                        char number[4] = {};
                        number[0] = value[0];
                        number[1] = value[1];
                        number[2] = value[2];
                        number[3] = 0;
                        int num = ParseInt(number);
                        if (num < 150 || num > 193)
                        {
                            isInvalid = true;
                            printf(" --- INVALID HEIGHT! --- ");
                            break;
                        }
                    }
                    else if (CompareString(value + (n - 2), "in"))
                    {
                        if (n != 4)
                        {
                            isInvalid = true;
                            printf(" --- INVALID HEIGHT! --- ");
                            break;
                        }
                        char number[3] = {};
                        number[0] = value[0];
                        number[1] = value[1];
                        number[2] = 0;
                        int num = ParseInt(number);
                        if (num < 59 || num > 76)
                        {
                            isInvalid = true;
                            printf(" --- INVALID HEIGHT! --- ");
                            break;
                        }
                    }
                    else
                    {
                        isInvalid = true;
                        printf(" --- INVALID UNIT! --- ");
                        break;
                    }
                }
                else if (CompareString(cat, "hcl"))
                {
                    foundCats++;

                    char value[10] = {}; // make a c-string
                    int n = 0;

                    while (n < ArrayCount(value))
                    {
                        char ch = passport->data[j++];
                        if (ch == ' ' || ch == '\n')
                            break;
                        value[n++] = ch;
                    }
                    value[n] = 0;

                    printf("%s", value);

                    if (n != 7) {
                        printf(" --- INVALID LENGTH! --- ");
                        isInvalid = true;
                        
                    }
                    if (value[0] != '#') {
                        printf(" --- INVALID MISSING HASH! --- ");
                        isInvalid = true;
                    }
                    
                    for (int s=1; s<7; ++s) {
                        if (!IsAlphaNumeric(value[s])) {
                            printf(" --- INVALID ALPHANUMERICS! --- ");
                            isInvalid = true;
                        }
                    }



                   

                }
                else if (CompareString(cat, "ecl"))
                {
                    foundCats++;

                    char value[256] = {}; // make a c-string
                    int n = 0;

                    while (n < ArrayCount(value))
                    {
                        char ch = passport->data[j++];
                        if (ch == ' ' || ch == '\n')
                            break;
                        value[n++] = ch;
                    }
                    value[n] = 0;

                    printf("%s", value);

                    if (!(CompareString(value, "amb") ||
                          CompareString(value, "blu") ||
                          CompareString(value, "brn") ||
                          CompareString(value, "gry") ||
                          CompareString(value, "grn") ||
                          CompareString(value, "hzl") ||
                          CompareString(value, "oth")))
                    {
                        isInvalid = true;
                        printf(" --- INVALID OPTION! --- ");
                        break;
                    }
                }
                else if (CompareString(cat, "pid"))
                {
                    foundCats++;

                    char value[256] = {}; // make a c-string
                    int valueCount = 0;

                    while (valueCount < ArrayCount(value))
                    {
                        char ch = passport->data[j++];
                        if (ch == ' ' || ch == '\n')
                            break;
                        value[valueCount++] = ch;
                    }
                    value[valueCount] = 0;
                    printf("%s", value);

                    if (valueCount != 9)
                    {
                        isInvalid = true;
                        printf(" --- INVALID LENGTH! --- ");
                        break;
                    }
                }
#if 0
                else if (CompareString(cat, "cid"))
                {
                    foundCats++;
                }
#endif

                printf("\n");

                if (isInvalid) {
                    printf(" --- INVALID --- ");
                    break;
                }
            }
        }
        printf("\n");
        // printf("foundCats: %d\n", foundCats);
        if (!isInvalid && foundCats < 7)
        {
            printf(" --- INVALID NUMBER OF CATEGORIES! --- \n");
            isInvalid = true;
        }

        if (isInvalid)
        {
            invalidCount++;
        }
        else
        {
            validCount++;
        }
    }

    printf("%d passports are valid\n%d are invalid.\n", validCount, invalidCount);
}