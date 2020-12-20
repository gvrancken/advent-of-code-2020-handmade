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

typedef struct String_t
{
    char *data;
    int length;
} String_t;

typedef struct Passport_t
{
    String_t str;
    int row;
    int seat;
    int seatID;
} Passport_t;

static char *CString(String_t s)
{

    char *result = (char *)malloc(s.length + 1);

    for (int i = 0; i < s.length; i++)
    {
        result[i] = s.data[i];
    }
    result[s.length] = 0;

    return result;
}

int main()
{
    debug_read_file_result_t result = ReadFileToHeap("input.txt");

    String_t input;
    input.data = result.contents;
    input.length = result.contentsSize;

    String_t groups[1024];
    int groupCount = 0;
    groups[0].data = input.data;

    for (int i = 0; i < input.length; i++)
    {
        char ch = input.data[i];
        char prev_ch = input.data[i - 1];

        if (ch == '\n' && prev_ch == '\n')
        {
            String_t *s = groups + groupCount;
            s->length = input.data + i - 1 - s->data;
            groupCount++;
            groups[groupCount].data = input.data + i + 1;
        }
    }

    groups[groupCount].length = input.data + input.length - groups[groupCount].data;
    groupCount++;

    printf("number of groups: %d\n", groupCount);

#if 0
    for (int i = 0; i < groupCount; ++i)
    {
        printf("%s\nlength: %d\n", CString(groups[i]), groups[i].length);
    }
#endif

    int totalChecked = 0;

    for (int i = 0; i < groupCount; ++i)
    {
        String_t it = groups[i];
        printf("----------------\n");
        printf("%s\n\n", CString(it));

        int numChecked = 0;
        int questions[26] = {};
        int numPeople = 1;

        for (int c = 0; c < it.length; ++c)
        {
            char ch = it.data[c];
            if (ch == '\n') {
                numPeople++;
            } else {
                questions[ch - 'a'] += 1;
            }
        }

        printf("numPeople: %d\n", numPeople);

        for (int n = 0; n < ArrayCount(questions); ++n)
        {
            printf("%c: %d\n", n + 97, questions[n]);
            if (questions[n] == numPeople)
            {
                numChecked++;
            }
        }

        

        printf("numChecked: %d\n", numChecked);
        totalChecked += numChecked;
    }

    printf("---------------\n");
    printf("Total checked: %d\n", totalChecked);
}