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

typedef struct v2
{
    int x;
    int y;
} v2;

static inline v2 V2(int a, int b) {
    v2 result = {a, b};
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

static int
ParseIntFromChars(char *str)
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
static int
ParseInt(String *str)
{
    int result = 0;

    for (int i = 0; i < str->length; ++i)
    {
        char number = str->data[i];
        if (IsNumber(number))
        {
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

typedef struct Ship
{
    v2 waypoint;
    v2 pos;
} Ship;

typedef struct Command
{
    char operation;
    int value;
} Command;

int main()
{
    String input = ReadFileToHeap("input.txt");

    StringArray lines = {};
    SplitString(&input, &lines);
    
    Ship ship = {};
    ship.pos = V2(0,0);
    ship.waypoint = V2(10,1);

    Command comList[1024] = {};
    int commandCount = 0;

    for (int i = 0; i < lines.count; ++i)
    {
        String *it = lines.str + i;

        char operation = it->data[0];
        String val = {it->data + 1,
                      it->length - 1};
        int value = ParseInt(&val);

        int commandIndex = commandCount++;
        Command *com = comList + commandIndex;
        com->value = value;
        com->operation = operation;
    }

    for (int i = 0; i < commandCount; ++i)
    {
        Command *com = comList + i;

        
        v2 offset = {};

        switch (com->operation) {
            case 'F': {
                offset = V2(ship.waypoint.x * com->value, ship.waypoint.y * com->value);
                ship.pos = V2(ship.pos.x + offset.x, ship.pos.y + offset.y);
            } break;
            case 'L': {
                int absRotAmount = abs(com->value / 90);
                while (absRotAmount-- > 0) { // lazy way out
                    ship.waypoint = V2(-ship.waypoint.y, ship.waypoint.x);
                }
            } break;
            case 'R': {
                int absRotAmount = abs(com->value / 90);
                while (absRotAmount-- > 0) { // lazy way out
                    ship.waypoint = V2(ship.waypoint.y, -ship.waypoint.x);
                }
            } break;
            case 'W' : {
                ship.waypoint = V2(ship.waypoint.x - com->value, ship.waypoint.y);
            } break;
            case 'N' : {
                ship.waypoint = V2(ship.waypoint.x, ship.waypoint.y + com->value);
            } break;
            case 'S' : {
                ship.waypoint = V2(ship.waypoint.x, ship.waypoint.y - com->value);
            } break;
            case 'E' : {
                ship.waypoint = V2(ship.waypoint.x + com->value, ship.waypoint.y);
            } break;
            default: {
                Assert("Invalid Case");
            }
        }
    }

    printf("Ship position: %d, %d.\n\n", ship.pos.x, ship.pos.y);
    int manhattanDistanceFromZero = abs(ship.pos.x) + abs(ship.pos.y);
    printf("Manhattan distance from start: %d.\n\n", manhattanDistanceFromZero);
}
