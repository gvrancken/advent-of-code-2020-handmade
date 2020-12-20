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

static u64 ParseU64(char *str)
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
    int lineCount;
} StringArray;


static void SplitString(String *input, StringArray *lines) {
    
    int lineIndex = lines->lineCount++;
    lines->str[lineIndex].data = input->data;

    for (int i = 0; i < input->length; i++)
    {
        char ch = input->data[i];

        if (ch == '\n')
        {
            lines->str[lineIndex].length = input->data + i - lines->str[lineIndex].data;
            lineIndex = lines->lineCount++;
            lines->str[lineIndex].data = input->data + i + 1;
        }
        if (i == input->length - 1)
        {
            lines->str[lineIndex].length = input->data + i + 1 - lines->str[lineIndex].data;
        }
    }
}

int comp (const void *a, const void *b) 
{
    u64 f = *(u64 *)a;
    u64 s = *(u64 *)b;
    if (f < s) return -1;
    if (f > s) return 1;
    return 0;
}

int main()
{
    String result = ReadFileToHeap("input.txt");

    StringArray lines = {};
    SplitString(&result, &lines);
    
    int preamble = 25;

    u64 invalidNumber = -1;

    for (int i = preamble; i < lines.lineCount; i++) {
        char *str = CString(lines.str[i]); // or write a fitting function
        u64 val = ParseU64(str);

        printf("[%d] trying to get %lld...\n", i, val);

        int offsetA = -preamble;
        int found = false;

        while (!found) {
            char *strA = CString(lines.str[i+offsetA]);
            u64 valA = ParseU64(strA);

            if (valA < val) {
                int offsetB = offsetA + 1;
                while (!found) {
                    char *strB = CString(lines.str[i+offsetB]);
                    u64 valB = ParseU64(strB);

                    u64 sum = valA + valB;
                    // printf("checking %lld + %lld = %lld\n", valA, valB, sum);

                    if (sum == val) {
                        // printf("Found: %lld + %lld.\n\n", valA, valB);
                        found = true;
                        break;
                    }

                    offsetB++;
                    if (offsetB == 0) break;
                }
            } else {
                // printf("Skipping %lld because it's larger than target %lld.\n", valA, val);
            }

            offsetA++;
            if (offsetA == 0) break;
        }

        if (!found) {
            invalidNumber = val;
            printf("Could not resolve this: %lld\n\n", val);
            break;
        }

    }

    

    u64 sumValues[1024];
    int sumCount = 0;

    for (int i = 0; i < lines.lineCount; i++) {
        
        bool found = false;
        u64 sum = 0;
        sumCount = 0;

        int counter = i;
        while (!found) {
            if (counter > lines.lineCount-1) break;

            char *str = CString(lines.str[counter]);
            u64 val = ParseU64(str);
            // printf(" + %lld\n", val);
            sum += val;
            sumValues[sumCount++] = val;

            if (sum == invalidNumber) {
                printf("FOUND!\n\n");
                found = true;
                break;
            } else if (sum > invalidNumber) {
                break;
            }

            counter++;
        }

        if (found) break;
    }

    qsort(sumValues, sumCount, sizeof(*sumValues), comp);

#if 1
    for (int i = 0; i < sumCount; i++) {
        printf("%lld + ", sumValues[i]);
    }
#endif

    printf("\n\n");
    u64 lowest = sumValues[0];
    u64 highest = sumValues[sumCount - 1];
    printf("lowest + highest\n%lld + %lld = %lld", lowest, highest, lowest + highest);

}
