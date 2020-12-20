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

typedef struct slope
{
    int right;
    int down;
    int treeCount;
} slope;

int main()
{

    slope slopes[5] = {
        {1, 1}, {3, 1}, {5, 1}, {7, 1}, {1, 2}};

    debug_read_file_result_t result = ReadFileToHeap("input.txt");

 
    int pitch = 0;

    char *p = result.contents;
    while (*p++ != '\n')
    {
        pitch++;
    }
    pitch += 1; // the end of line is part of the line if we dont split there

    printf("pitch: %d\n", pitch);

    char *bytes = (char *)result.contents;

    for (int slopeIndex = 0;
         slopeIndex < ArrayCount(slopes);
         slopeIndex++)
    {
        slope *slopePtr = slopes + slopeIndex;
        int freeCount = 0;
        int treeCount = 0;

        int targetCol = 0;
        int nextRow = 0;

        for (int i = 0; i < result.contentsSize; ++i)
        {
            int row = i / pitch;
            // only walk through actual number chars, not the newline char (pitch-1)
            for (int col = 0; col < pitch - 1; ++col)
            {
                int pixel = row * pitch + col;

                char *c = bytes + pixel;
                if (row == nextRow && col == targetCol)
                {
                    if (*c == '.')
                    {
                        freeCount++;
                    }
                    else
                    {
                        treeCount++;
                    }
                    targetCol = (targetCol + slopePtr->right) % (pitch - 1);
                    nextRow += slopePtr->down;
                }
            }

           
        }

        slopePtr->treeCount = treeCount;        
        printf("treeCount: %d (slope %d,%d)\n", slopePtr->treeCount, slopePtr->right, slopePtr->down);
    }

    long long slopedMultiplied = slopes[0].treeCount;

    for (int slopeIndex = 1;
         slopeIndex < ArrayCount(slopes);
         slopeIndex++)
    {
        slope *slopePtr = slopes + slopeIndex;
        slopedMultiplied *= slopePtr->treeCount;
    }

    printf("multiplied treeCount: %lld\n", slopedMultiplied);


}