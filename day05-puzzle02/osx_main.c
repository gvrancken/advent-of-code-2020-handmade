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

typedef struct Passport_t {
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

int comp (const void *a, const void *b) 
{
    Passport_t *p1 = (Passport_t *)a;
    Passport_t *p2 = (Passport_t *)b;

    int s = p1->seatID;
    int f = p2->seatID;
    if (f > s) return -1;
    if (f < s) return 1;
    return 0;
}

int main()
{
    debug_read_file_result_t result = ReadFileToHeap("input.txt");

    String_t input = {
        result.contents,
        result.contentsSize};

    Passport_t passes[1024] = {};
    int passCount = 0;

    passes[0].str.data = input.data;

    for (int i = 0; i < input.length; ++i)
    {

        char ch = input.data[i];
        if (ch == '\n')
        {
            passes[passCount].str.length = (input.data + i) - passes[passCount].str.data;
            passCount++;
            passes[passCount].str.data = input.data + i + 1;
        }
    }
    
    // end the last one (eof in stead of newline).
    passes[passCount].str.length = input.data + input.length - passes[passCount].str.data;
    passCount++;

    printf("number of passes: %d\n", passCount);

#if 0
    for (int i=0; i<passCount; ++i) {
        printf("%s\n", CString(passes[i]));
    }
#endif

    int highest = 0;

    for (int i = 0; i < passCount; ++i)
    {

        int minRow = 0;
        int maxRow = 127;

        int minSeat = 0;
        int maxSeat = 7;

        Passport_t *pass = passes + i;
        
        printf("pass: %s\n", CString(pass->str));

        for (int j = 0; j < pass->str.length; ++j)
        {
            char ch = pass->str.data[j];
            if (ch == 'F')
            {
                maxRow = minRow + ((maxRow-minRow) >> 1);
            }
            else if (ch == 'B')
            {
                minRow = maxRow - ((maxRow-minRow) >> 1);
            }
            else if (ch == 'L')
            {
                maxSeat = minSeat + ((maxSeat-minSeat) >> 1);
            }
            else if (ch == 'R')
            {
                minSeat = maxSeat - ((maxSeat-minSeat) >> 1);
            }

            // printf("%c --> min: %d, max: %d... seat min: %d max: %d\n", ch, minRow, maxRow, minSeat, maxSeat);
        }

        int seatId = minRow*8+minSeat;
        // printf("row: %d, seat: %d. Seat ID = %d\n", minRow, minSeat, seatId);

        if (seatId > highest) highest = seatId;
        
        pass->row = minRow;
        pass->seat = minSeat;
        pass->seatID = seatId;
        
    }

    qsort(passes, passCount, sizeof(*passes), comp);

    // NOTE(gb): check where there is more than 1 space between. this is the missing seat.
    for (int i=1; i<passCount-1; i++) {
        Passport_t *curr = passes + i;
        Passport_t *next = passes + (i+1);

        if (next->seatID - curr->seatID != 1 && curr->seatID < 127*8 && curr->seatID > 8) {
            printf("Seat id %d is next to %d --- id %d is missing!\n", curr->seatID, next->seatID, next->seatID-1);
        }
        
    }

}