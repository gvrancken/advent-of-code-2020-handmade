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
    int min = -1;
    int max = -1;
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

            min = fullInt;
            fullInt = 0;
        }
        else if (it == ' ')
        {
            if (fullInt != 0) { 
                max = fullInt;
                fullInt = 0;
            } 
        }
        else if (it == ':')
        {
            target = *((char *)result.Contents + i -1);
        }
        else if (it == '\n' || i == result.ContentsSize-1) {
            printf("%d - %d %c: %s (%d)", min, max, target, word, targetCount);  
            if (targetCount < min || targetCount > max) {
                printf(" --- ERROR!\n");
                invalidCount++;
            } else {
                validCount++;
            }
            printf("\n");

            min = -1;
            max = -1;
            letterCount = 0;
            target = -1;
            targetCount = 0;
            
        }
        else {
            // grab word
            if (target != -1) {
                word[letterCount++] = it;
                if (it == target) targetCount++;
            
            }
        }
     
    }

    printf("   Valid: %d\n   Invalid: %d\n", validCount, invalidCount);

}