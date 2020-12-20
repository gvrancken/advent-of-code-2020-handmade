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
    size_t ContentsSize;
    void *Contents;
} debug_read_file_result_t;

inline int StringLength(char *str)
{
    int count = 0;
    while (*str++)
    {
        count++;
    }

    return count;
}

static void
CatStrings(char *srcA, size_t lenA, char *srcB, size_t lenB, char *dest)
{
    for (int i = 0; i < lenA; i++)
    {
        *dest++ = *srcA++;
    }

    for (int i = 0; i < lenB; i++)
    {
        *dest++ = *srcB++;
    }

    *dest++ = 0; // add 0 terminator
}

static debug_read_file_result_t
ReadFileToHeap(char *Filename)
{
    debug_read_file_result_t Result = {};

    char fullPathFileName[1024];

    FILE *FileHandle = fopen(Filename, "r");

    if (FileHandle != 0)
    {
        fseek(FileHandle, 0, SEEK_END);
        size_t FileSize = ftell(FileHandle);
        if (FileSize)
        {
            rewind(FileHandle);
            Result.Contents = malloc(FileSize);

            if (Result.Contents)
            {
                size_t BytesRead = fread(Result.Contents, 1, FileSize, FileHandle);
                if (FileSize == BytesRead)
                {
                    // File read successfully
                    Result.ContentsSize = FileSize;
                }
            }
        }
        fclose(FileHandle);
    }

    return (Result);
}

static bool IsNumeric(char val)
{
    char it = val - '0';
    return (it >= 0 && it <= 9);
}

int main()
{

    debug_read_file_result_t result = ReadFileToHeap("input.txt");


    int intArr[256] = {};
    int intCount = 0;

    int fullInt = 0;
    int posA = -1;
    int posB = -1;
    char target = -1;

    char word[256];
    int letterCount = 0;
    int targetCount = 0;
    int validCount = 0;
    int invalidCount = 0;

    for (int i = 0; i < result.ContentsSize; i++)
    {
        Assert(intCount < ArrayCount(intArr));

        char it = *((char *)result.Contents + i);

        if (IsNumeric(it))
        {
            int itInt = it - '0';
            fullInt = fullInt * 10 + itInt;
        }
        else if (it == '-')
        {
            posA = fullInt;
            fullInt = 0;
        }
        else if (it == ' ')
        {
            if (fullInt != 0) { 
                posB = fullInt;
                fullInt = 0;
            } 
        }
        else if (it == ':')
        {
            target = *((char *)result.Contents + i -1);
        }
        else if (it == '\n' || i == result.ContentsSize-1) {
            printf("%d - %d %c: %s (%d)", posA, posB, target, word, targetCount);  
            if (targetCount != 1) {
                printf(" --- ERROR!");
                invalidCount++;
            } else {
                validCount++;
            }
            printf("\n");

            posA = -1;
            posB = -1;
            letterCount = 0;
            target = -1;
            targetCount = 0;
            
        }
        else {
            // grab word if we already have a target letter
            if (target != -1) {
                word[letterCount++] = it;
                if (letterCount == posA || letterCount == posB) {
                    if (it == target) targetCount++;
                }
                
            }
        }
     
    }

    printf("   Valid: %d\n   Invalid: %d\n", validCount, invalidCount);

}