#include <stdio.h>          // printf

#define ArrayCount(Array) ( (sizeof(Array) / sizeof((Array)[0])) )
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

typedef struct debug_read_file_result_t
{
    size_t ContentsSize;
    void *Contents;
} debug_read_file_result_t;

inline int StringLength(char *str) {
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
ReadFileToStack(char *Filename, void *mem)
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
            Result.Contents = mem;
           
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


int main()
{
    char mem[1024] = {};
    debug_read_file_result_t result = ReadFileToStack("input.txt", mem);

    char *p = (char *)result.Contents;

    int intArr[256] = {};
    int intCount = 0;

    int fullInt = 0;
    for (int i = 0; i < result.ContentsSize; i++)
    {
        Assert(intCount < ArrayCount(intArr));

        char it = *((char *)result.Contents + i);
        int itInt = it - '0';
        if (it != '\n') {
            fullInt = fullInt * 10 + itInt;
        } else {
            intArr[intCount++] = fullInt;
            fullInt = 0;
        }

        p++;
    }

    int target = 2020;
    int *a = intArr;
    while (*a != 0) {
        int *b = a+1;
        while (*b != 0) {
            int *c = b+1;
            while (*c != 0) {
                int result = (*b)+(*a)+(*c);
                if (result == target) {
                    printf("%d + %d + %d = %d\n", *a, *b, *c, target);
                    printf("%d * %d * %d = %d\n", *a, *b, *c, (*b) * (*a) * (*c));
                }
                c++;
            }
            b++;
        }
        a++;
    }

}