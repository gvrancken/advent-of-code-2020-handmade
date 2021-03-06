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
    int length;
} String;

typedef struct StringArray
{
    String str[4096];
    int count;
} StringArray;


typedef struct Range {
    int min;
    int max;
} Range;

typedef struct Category {
    Range ranges[2];
} Category;



static String
ReadFileToHeap(char *Filename)
{
    String result = {};

    void *fileHandle = fopen(Filename, "r");

    if (!fileHandle)
        return result;

    fseek(fileHandle, 0, SEEK_END);
    size_t fileSize = ftell(fileHandle);
    if (!fileSize) {
        Assert("File not found.");
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
CompareChars(char *a, char *b, int length)
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

static bool
CompareString(String a, String b)
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

static bool
IsNumber(char value)
{
    int n = value - '0';
    return (n >= 0 && n <= 9);
}

static bool
IsAlphaNumeric(char value)
{
    return ((value >= '0' && value <= '9') ||
            (value >= 'a' && value <= 'z'));
}

static u64
ParseU64FromChars(char *str)
{

    u64 result = 0;

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
            // NOTE(gb) We return the number until the non-number char
            return result;
        }
    }

    return result;
}

static int
ParseInt(String *str, int len)
{
    int result = 0;

    if (len > 0) {
        for (int i = 0; i < len; ++i)
        {
            char number = str->data[i];
            if (IsNumber(number))
            {
                result *= 10;
                result += number - '0';
            }
        }
    } else {
        for (int i = 0;; ++i) {
            char number = str->data[i];
            if (!IsNumber(number)) break;
            result *= 10;
            result += number - '0';
        }

    }

    return result;
}

static char *
CString(String s)
{

    char *result = (char *)malloc(s.length + 1);

    for (int i = 0; i < s.length; i++)
    {
        result[i] = s.data[i];
    }
    result[s.length] = 0;

    return result;
}


static void SplitString(String *input, StringArray *lines, char splitChar)
{

    int lineIndex = lines->count++;
    lines->str[lineIndex].data = input->data;

    for (int i = 0; i < input->length; i++)
    {
        char ch = input->data[i];

        if (ch == splitChar)
        {
            lines->str[lineIndex].length = input->data + i - lines->str[lineIndex].data; // end line start new one
            lineIndex = lines->count++;
            lines->str[lineIndex].data = input->data + i + 1;
        }
    }
    lines->str[lineIndex].length = input->data + input->length - lines->str[lineIndex].data;
}

int comp(const void *a, const void *b)
{
    return 0;
}

typedef struct Ticket {
    int values[64];
} Ticket;

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

    Category categories[64];
    int categoryCount = 0;
    
    int i=0;
    for (; i < lines.count; i++) {
        String *line = lines.str + i;

        if (line->length == 0) break; // end of categories

        Category *category = categories + categoryCount++;

        int len = 0;
        while (true) {
            char ch = line->data[len++];
            if (ch == ':') break;
        }

        String catVal = {line->data + len+1, line->length - (len+1)};

        Range ranges[2] = {};

        StringArray rangesStr = {};
        SplitString(&catVal, &rangesStr, ' ');
        // NOTE(gb) skip the word 'or'
        rangesStr.str[1] = rangesStr.str[2];


        for (int n=0; n<2; n++) {
            StringArray minMax = {};
            SplitString(rangesStr.str + n, &minMax, '-');

            ranges[n].min = ParseInt(minMax.str, minMax.str[0].length);
            ranges[n].max = ParseInt(minMax.str + 1, minMax.str[1].length);
        
        }

        category->ranges[0] = ranges[0];
        category->ranges[1] = ranges[1];
    }
    
    i+=2; // skip white lines and header

    Ticket tickets[256] = {};
    int ticketCount = 0;

    for (; i<lines.count; i++) {
        String *line = lines.str + i;

        if (line->length == 0) {  // end of my ticket
            i+=1; // skip header
            continue;
        }

        Ticket *myT = tickets + ticketCount++;

        StringArray valuesStr = {};
        SplitString(line, &valuesStr, ',');
        
        for (int n=0; n < valuesStr.count; n++) {
            myT->values[n] = ParseInt(valuesStr.str + n, valuesStr.str[n].length);
        }
    }

    // search other tickets
    int total = 0;
    for (int ticketIndex=1; ticketIndex < ticketCount; ticketIndex++) {
        Ticket *ticket = tickets + ticketIndex;

        
        for (int valIndex=0; valIndex < categoryCount; valIndex++) {
            int val = ticket->values[valIndex];
            bool valid = false;
            for (int catIndex=0; catIndex<categoryCount; catIndex++) {
                Category *cat = categories + catIndex;
                if ((val >= cat->ranges[0].min &&
                    val <= cat->ranges[0].max) ||
                    (val >= cat->ranges[1].min &&
                    val <= cat->ranges[1].max)) {
                        valid = true;
                    }
            }
            if (!valid) {
                printf("Invalid: %d (ticket [%d])\n", val, ticketIndex);
                total += val;
            }
        }
    
    }
    printf("Answer: %d.\n", total);

    // Time
    u64 timerEnd = mach_absolute_time();
    u64 dTicks = timerEnd - timerStart;
    u64 nanoseconds = dTicks * global_nanoseconds_per_tick;
    float ms = (float)nanoseconds * 1.0E-6;

    printf("It took %fms.\n", ms);
}
