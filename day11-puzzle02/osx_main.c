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

#define Min(a, b) (a < b) ? a : b
#define Max(a, b) (a > b) ? a : b

#define u64 uint64_t

typedef struct String
{
    char *data;
    int length;
} String;

typedef struct Pos
{
    int col;
    int row;
} Pos;

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

typedef struct StringArray
{
    String str[4096];
    int count;
} StringArray;

typedef struct arr
{
    void *data;
    int length;
} arr;

static int CountPaths(int *adapters, int adapterCount, int val, int index, int *arr)
{

    if (val == 1)
        return 1;
    if (val == 2)
        return 2;
    if (val == 3)
        return 3;

    // caching
    if (arr[val - 1] > 0)
    {
        return arr[val - 1];
    }

    arr[val - 1] = CountPaths(adapters, adapterCount, val - 1, index, arr) +
                   CountPaths(adapters, adapterCount, val - 2, index, arr) +
                   CountPaths(adapters, adapterCount, val - 3, index, arr);

    return arr[val - 1];
}

static void SplitString(String *input, StringArray *lines)
{

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

int comp(const void *a, const void *b)
{
    int f = *(int *)a;
    int s = *(int *)b;
    if (f < s)
        return -1;
    if (f > s)
        return 1;
    return 0;
}

static inline bool IsValid(String *str, Pos P, int pitch)
{
    return (P.col >= 0 && P.col < pitch && P.row >= 0 && P.row <= (str->length / pitch) - 1);
}

static inline bool IsOccupied(String *str, Pos P, int pitch)
{
    int index = P.row * pitch + P.col;
    return (str->data[index] == '#');
}

static bool CheckSquare(String *str, Pos P, Pos dP, int pitch)
{
    Pos newPos = {P.col + dP.col, P.row + dP.row};
    int index = newPos.row * pitch + newPos.col;

    if (!IsValid(str, newPos, pitch))
    {
        return false;
    }

    if (str->data[index] == '#')
        return true;
    if (str->data[index] == 'L')
        return false;

    return CheckSquare(str, newPos, dP, pitch);
}

static int CountNeighborsOccupied(String *str, Pos P, int pitch)
{
    int numOccupied = 0;

    // loop through all directions until we find a # chair (L or #, not a floor (.) or end of room). This is the one that counts.
    Pos dir[8] = {
        {1,0},
        {1,1},
        {0,1},
        {-1,1},
        {-1,0},
        {-1,-1},
        {0,-1},
        {1,-1}
    };

    for (int i=0; i<ArrayCount(dir); ++i) {
        Pos dP = dir[i];
        if (CheckSquare(str, P, dP, pitch)) {
            numOccupied++;
        }
    }

    return numOccupied;
}

int main()
{
    String input = ReadFileToHeap("input.txt");

    int pitch = -1;
    int offset = 0;
    for (int i = 0; i < input.length; ++i)
    {
        char it = input.data[i];

        input.data[i - offset] = it;

        if (it == '\n')
        {
            if (pitch == -1)
            {
                pitch = i;
            }
            offset++;
        }
    }
    input.length -= offset;
    printf("pitch: %d\n\n", pitch);

    String buffer = {};
    buffer.data = (char *)malloc(input.length);
    buffer.length = input.length;

#if 0
    for (int i=0; i<result.length; ++i) {
        char it = result.data[i];
        printf("%c", it);
    }
    print("\n\n");
#endif

    // NOTE(gb): we can now use a col row notation, we know that (row * pitch + col) is the index.

    int totalOccupied = 0;

    while (true)
    {
        bool hasChanged = false;
        totalOccupied = 0;
        int rowCount = input.length / pitch;
        for (int row = 0; row < rowCount; row++)
        {
            for (int col = 0; col < pitch; col++)
            {
                int index = row * pitch + col;
                Pos P = {col, row};
                int numOccupied = CountNeighborsOccupied(&input, P, pitch);

                if (input.data[index] == 'L' && numOccupied == 0)
                {
                    buffer.data[index] = '#';
                    hasChanged = true;
                }
                else if (input.data[index] == '#' && numOccupied >= 5)
                {
                    buffer.data[index] = 'L';
                    hasChanged = true;
                }
                else
                {
                    buffer.data[index] = input.data[index];
                }

                printf("%c", buffer.data[index]);
                if (buffer.data[index] == '#')
                {
                    totalOccupied++;
                }
            }
            printf("\n");
        }

        printf("\n\n");

        if (!hasChanged)
            break;

        // flip pointers
        char *temp = buffer.data;
        buffer.data = input.data;
        input.data = temp;
    }

    printf("Answer: %d", totalOccupied);
}
