#include <stdio.h> // printf
#include <stdbool.h>
#include <stdlib.h> // malloc
#include <string.h> // memcpy
#include <stdint.h> // types

#define ArrayCount(Array) ((sizeof(Array) / sizeof((Array)[0])))
#define Assert(Expression) \
    if (!(Expression))     \
    {                      \
        *(int *)0 = 0;     \
    }

#define u64 uint64_t

typedef struct String
{
    char *data;
    int length;
} String;

static String
ReadFileToHeap(char *Filename)
{
    String result = {};

    void *fileHandle = fopen(Filename, "r");

    if (!fileHandle)
        return result;

    fseek(fileHandle, 0, SEEK_END);
    size_t fileSize = ftell(fileHandle);
    if (!fileSize)
        return result;

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

static bool CompareChars(char *a, char *b, int length)
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

static bool CompareString(String a, String b)
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

static char *CString(String s)
{

    char *result = (char *)malloc(s.length + 1);

    for (int i = 0; i < s.length; i++)
    {
        result[i] = s.data[i];
    }
    result[s.length] = 0;

    return result;
}

typedef struct StringArray {
    String str[4096];
    int count;
} StringArray;

typedef struct arr {
    void *data;
    int length;
} arr;

static int CountPaths(int *adapters, int adapterCount, int val, int index, int *arr) {
    
    if (val == 1) return 1;
    if (val == 2) return 2;
    if (val == 3) return 3;
    
    // caching
    if (arr[val-1] > 0) {
        return arr[val-1];
    }

    arr[val-1] = CountPaths(adapters, adapterCount, val-1, index, arr) + 
                 CountPaths(adapters, adapterCount, val-2, index, arr) + 
                 CountPaths(adapters, adapterCount, val-3, index, arr);
    
    return arr[val-1];
}
    

static void SplitString(String *input, StringArray *lines) {
    
    int lineIndex = lines->count++;
    lines->str[lineIndex].data = input->data;

    for (int i = 0; i < input->length; i++)
    {
        char ch = input->data[i];

        if (ch == '\n')
        {
            lines->str[lineIndex].length = input->data + i - lines->str[lineIndex].data; // end line start new one
            lineIndex = lines->count++;
            lines->str[lineIndex].data = input->data + i + 1;
        }
    }
    lines->str[lineIndex].length = input->data + input->length - lines->str[lineIndex].data;
}

int comp (const void *a, const void *b) 
{
    int f = *(int *)a;
    int s = *(int *)b;
    if (f < s) return -1;
    if (f > s) return 1;
    return 0;
}

int main()
{
    String result = ReadFileToHeap("input.txt");

    StringArray lines = {};
    SplitString(&result, &lines);

    int adapters[1024];
    int adapterCount = lines.count;

    // store in int array
    for (int i=0; i<lines.count; ++i) {
        char *str = CString(lines.str[i]);
        int val = ParseInt(str);
        // printf("%d\n", val);

        adapters[i] = val;
    }

    qsort(adapters, adapterCount, sizeof(*adapters), comp);

    u64 *arr = (u64 *)calloc(adapterCount, sizeof(u64));
    // int totalPaths = CountPaths(adapters, adapterCount, 15, adapterCount-1, arr);
    
    for (int i = 0; i < adapterCount; i++) {

        int val = adapters[i]; 
        printf("[%d] %d\n", i, val);
        if (i == 0) {
            if (val <= 3) arr[i] = 1;
        } else if (i == 1) {
            if (val <= 3) arr[i]++;
            if (val - arr[0] <= 3) arr[i] += arr[0];
        } else if (i == 2) {
            if (val <= 3) arr[i]++;
            if (val - arr[1] <= 3) arr[i] += arr[1];
            if (val - arr[0] <= 3) arr[i] += arr[0];
        } else {

            for (int j = i-3; j < i; j++) {
                int val2 = adapters[j];
                int diff = val - val2;
                if (diff <= 3) {
                    arr[i] += arr[j];
                }
            }
        }
    }

    printf("----------\n");
    int totalPaths = 0;
    for (int i = 0; i < adapterCount; i++) {
        printf("[%d] a: %d, arr: %lld\n", i, adapters[i], arr[i]);
        totalPaths += arr[i];
    }


    printf("answer: %d.\n", totalPaths);

}
