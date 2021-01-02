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
#define START_DIM4 1

#define TURNS 6

#define MAX_WIDTH (START_WIDTH + TURNS * 2)
#define MAX_HEIGHT (START_HEIGHT + TURNS * 2)
#define MAX_DEPTH (START_DEPTH + TURNS * 2)
#define MAX_DIM4 (START_DIM4 + TURNS * 2)

#define TOTAL_CUBES MAX_WIDTH *MAX_HEIGHT *MAX_DEPTH *MAX_DIM4

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

void MarkNeighbors(int *active, int index)
{
    for (int w = -1; w <= 1; ++w)
    {
        for (int z = -1; z <= 1; ++z)
        {
            for (int y = -1; y <= 1; ++y)
            {
                for (int x = -1; x <= 1; ++x)
                {
                    int offset = (w * MAX_DEPTH * MAX_HEIGHT * MAX_WIDTH) + z * (MAX_HEIGHT * MAX_WIDTH) + (y * MAX_WIDTH) + x;
                    if (offset != 0)
                    {
                        active[index + offset] += 1;
                    }
                }
            }
        }
    }
}

static inline int gridToPos(int x, int y, int z, int w)
{
    return (w * MAX_DEPTH * MAX_WIDTH * MAX_HEIGHT) + (z * MAX_WIDTH * MAX_HEIGHT) + (y * MAX_WIDTH) + x;
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

    // int grid[MAX_DEPTH][MAX_HEIGHT][MAX_WIDTH] = {};
    int grid[TOTAL_CUBES] = {};
    int numActiveNeighbors[TOTAL_CUBES] = {};

    // init
    int *row = grid + (TURNS * MAX_DEPTH * MAX_WIDTH * MAX_HEIGHT) + (TURNS * MAX_WIDTH * MAX_HEIGHT) + (TURNS * MAX_WIDTH) + TURNS;

    for (int y = 0; y < lines.count; ++y)
    {
        int *pixel = row;
        String *line = lines.str + y;

        for (int x = 0; x < line->length; ++x)
        {

            char ch = line->data[x];
            if (ch == '#')
            {
                // since coords will be negative as well, offset them so the
                // highest negative is still a positive array index.
                *pixel = 1;
            }
            pixel++;
        }

        row += MAX_WIDTH;
    }

    // update
    int turn = 1;

    while (turn <= 6)
    {

        // printf("\n\nTURN %d\n\n", turn);
        int totalActive = 0;

        for (int w = 0; w < MAX_DIM4; ++w)
        {
            for (int z = 0; z < MAX_DEPTH; ++z)
            {
                // printf("Z=%d\n\n", z);
                for (int y = 0; y < MAX_HEIGHT; ++y)
                {
                    for (int x = 0; x < MAX_WIDTH; ++x)
                    {
                        int pos = gridToPos(x, y, z, w);
                        int isActive = grid[pos];
                        // printf("%c", isActive ? '#' : '.');

                        if (isActive)
                        {
                            MarkNeighbors(numActiveNeighbors, pos);
                        }
                    }
                    // printf("\n");
                }
            }
        }

        for (int w = -turn; w < START_DIM4 + turn; ++w)
        {
            for (int z = -turn; z < START_DEPTH + turn; ++z)
            {
                // printf("   Z=%d\n\n", z);
                for (int y = -turn; y < START_HEIGHT + turn; ++y)
                {
                    for (int x = -turn; x < START_WIDTH + turn; ++x)
                    {
                        int arrayX = x + TURNS;
                        int arrayY = y + TURNS;
                        int arrayZ = z + TURNS;
                        int arrayW = w + TURNS;

                        int pos = gridToPos(arrayX, arrayY, arrayZ, arrayW);
                        int isActive = grid[pos];
                        int numActive = numActiveNeighbors[pos];

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

                        grid[pos] = isActive;
                        totalActive += isActive;
                        // printf("%c", isActive ? '#' : '.');
                    }
                    // printf("\n");
                }
            }
        }

        printf("Turn %d, total active: %d.\n", turn, totalActive);

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
