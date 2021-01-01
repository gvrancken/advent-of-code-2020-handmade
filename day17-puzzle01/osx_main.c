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

#define START_WIDTH 8
#define START_HEIGHT 8
#define START_DEPTH 1

#define TURNS 6

#define MAX_WIDTH (START_WIDTH + (TURNS * 2) + 2)
#define MAX_HEIGHT (START_HEIGHT + (TURNS * 2) + 2)
#define MAX_DEPTH (START_DEPTH + (TURNS * 2) + 2)

#define TOTAL_CUBES (MAX_WIDTH * MAX_HEIGHT * MAX_DEPTH)

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

typedef struct v3
{
    int x;
    int y;
    int z;
} v3;

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
            result += (u64)(number - '0');
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

    if (len > 0)
    {
        for (int i = 0; i < len; ++i)
        {
            char number = str->data[i];
            if (IsNumber(number))
            {
                result *= 10;
                result += number - '0';
            }
        }
    }
    else
    {
        for (int i = 0;; ++i)
        {
            char number = str->data[i];
            if (!IsNumber(number))
                break;
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

int comp(const void *a, const void *b)
{
    return 0;
}



int PosToIndex(v3 pos)
{
    return (pos.z + TURNS) * START_WIDTH * START_HEIGHT + (pos.y + TURNS) * START_WIDTH + (pos.x + TURNS);
}

void MarkNeighbors(int active[MAX_DEPTH][MAX_HEIGHT][MAX_WIDTH], int arrayX, int arrayY, int arrayZ)
{
    for (int z = arrayZ-1; z <= arrayZ+1; ++z)
    {
        for (int y = arrayY-1; y <= arrayY+1; ++y)
        {
            for (int x = arrayX-1; x <= arrayX+1; ++x)
            {
                if (x == arrayX && y == arrayY && z == arrayZ) {
                    continue;
                }
                
                active[z][y][x] += 1;
                
            }
        }
    }
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

    int grid[MAX_DEPTH][MAX_HEIGHT][MAX_WIDTH] = {};
    int prevGrid[MAX_DEPTH][MAX_HEIGHT][MAX_WIDTH] = {};

    int numActiveNeighbors[MAX_DEPTH][MAX_HEIGHT][MAX_WIDTH] = {};

    // init
    for (int y = 0; y < lines.count; ++y)
    {
        String *line = lines.str + y;
        for (int x = 0; x < line->length; ++x)
        {
            char ch = line->data[x];
            if (ch == '#')
            {
                int z = 0;
                grid[z + TURNS][y + TURNS][x + TURNS] = 1;
            }
        }
    }

    // update
    int turn = 0;

    while (turn < 6)
    {

        printf("\n\nTURN %d\n\n", turn);
        int totalActive = 0;
        for (int z = 0; z < MAX_DEPTH; ++z)
        {
            printf("Z=%d\n\n", z);
            for (int y = 0; y < MAX_HEIGHT; ++y)
            {
                for (int x = 0; x < MAX_WIDTH; ++x)
                {
                    int isActive = grid[z][y][x];
                    printf("%c", isActive ? '#' : '.');

                    if (isActive)
                    {
                        MarkNeighbors(numActiveNeighbors, x, y, z);
                    }
                }
                printf("\n");
            }
        }

        for (int z = -(turn + 1); z < START_DEPTH + turn + 1; ++z)
        {
            printf("   Z=%d\n\n", z);
            for (int y = -(turn + 1); y < START_HEIGHT + turn + 1; ++y)
            {
                for (int x = -(turn + 1); x < START_WIDTH + turn + 1; ++x)
                {
                    int arrayZ = z + TURNS;
                    int arrayY = y + TURNS;
                    int arrayX = x + TURNS;

                    int isActive = grid[arrayZ][arrayY][arrayX];
                    int numActive = numActiveNeighbors[arrayZ][arrayY][arrayX];

                    if (isActive)
                    {
                        if (!(numActive == 2 || numActive == 3))
                        {
                            isActive = 0;
                        }
                    }
                    else
                    {
                        if (numActive == 3)
                        {
                            isActive = 1;
                        }
                    }

                    grid[arrayZ][arrayY][arrayX] = isActive;
                    totalActive += isActive;
                    printf("%c", isActive ? '#' : '.');
                }
                printf("\n");
            }
        }

        printf("total active: %d.\n", totalActive);

        memset(numActiveNeighbors, 0, TOTAL_CUBES * sizeof(int));
        turn++;
    }

    // Time
    u64 timerEnd = mach_absolute_time();
    u64 dTicks = timerEnd - timerStart;
    u64 nanoseconds = dTicks * global_nanoseconds_per_tick;
    float ms = (float)(nanoseconds * 1.0E-6);

    printf("It took %fms.\n", ms);
}
