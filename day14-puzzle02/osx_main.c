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

typedef struct String
{
    char *data;
    int length;
} String;

typedef struct StringArray
{
    String str[4096];
    int count;
} StringArray;

typedef struct HashedCommand
{
    u64 value;
    u64 address;
    struct HashedCommand *next;
    bool isInit;
} HashedCommand;

static String
ReadFileToHeap(char *Filename)
{
    String result = {};

    void *fileHandle = fopen(Filename, "r");

    if (!fileHandle)
        return result;

    fseek(fileHandle, 0, SEEK_END);
    size_t fileSize = ftell(fileHandle);
    if (!fileSize) {
        Assert("File not found.");
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
            result += number - '0';
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

    for (int i = 0; i < len; ++i)
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

static void SplitString(String *input, StringArray *lines, char splitChar)
{

    int lineIndex = lines->count++;
    lines->str[lineIndex].data = input->data;

    for (int i = 0; i < input->length; i++)
    {
        char ch = input->data[i];

        if (ch == splitChar)
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
    HashedCommand *f = (HashedCommand *)a;
    HashedCommand *s = (HashedCommand *)b;
    if (f->address < s->address)
        return -1;
    if (f->address > s->address)
        return 1;
    return 0;
}

static void StoreInHash(HashedCommand *table, int tableSize, HashedCommand item)
{
    // NOTE(gb): Obligatory Use Better Hash Function!
    int hashSlot = item.address % tableSize;

    // since there is always an entity in the slot (and not a null pointer), we need to check a value to see if it's an empty entity.
    HashedCommand *itemInList = table + hashSlot;
    if (!itemInList->isInit)
    {
        // array pos is empty, just store
        item.isInit = true;
        *itemInList = item;
    }
    else
    {
        if (itemInList->address == item.address)
        {
            itemInList->value = item.value;
            return;
        }
        // hash collision!

        // traverse the next pointers until we find an empty spot
        while (itemInList->next)
        {
            itemInList = itemInList->next;
            if (itemInList->address == item.address)
            {
                itemInList->value = item.value;
                return;
            }
        }

        // NOTE(gb): we allocate new heap memory here.
        // we usually will point to our pre-allocated memory here and
        // increase its pointer
        HashedCommand *heapItem = (HashedCommand *)malloc(sizeof(HashedCommand));
        item.isInit = true;
        *heapItem = item;
        // assign it to our hash table:
        itemInList->next = heapItem;
    }
}

static void FloatingValue(u64 floatingAddress, int floatingIndex, int *floatingBits, int floatLen, u64 *mem, u64 val, HashedCommand *hashTable, int tableSize) {
    if (floatingIndex >= floatLen) {
        // printf("reached address %llu, val %llu\n", floatingAddress, val);
        // store in hash
        HashedCommand hashCom = {};
        hashCom.value = val;
        hashCom.address = floatingAddress;
        StoreInHash(hashTable, tableSize, hashCom);

        mem[floatingAddress] = val;
        return;
    }

    int floatingBit = floatingBits[floatingIndex];
    u64 addressA = (floatingAddress | ((u64)1 << floatingBit));
    u64 addressB = addressA ^ ((u64)1 << floatingBit);

    FloatingValue(addressA, floatingIndex+1, floatingBits, floatLen, mem, val, hashTable, tableSize);
    FloatingValue(addressB, floatingIndex+1, floatingBits, floatLen, mem, val, hashTable, tableSize);
}

int main()
{

    String input = ReadFileToHeap("input.txt");

    // Time
    mach_timebase_info_data_t tb;
    mach_timebase_info(&tb);
    u64 global_nanoseconds_per_tick = tb.numer / tb.denom;
    u64 _startTime = mach_absolute_time();

    // Program
    StringArray lines = {};
    SplitString(&input, &lines, '\n');

    HashedCommand hashTable[1024] = {};

    // const u64 memorySize = 68719476736;
    const u64 memorySize = 68719476736;
    u64 *memory = calloc(memorySize, sizeof(u64));

    String activeMask = {};
    int numInstructions = 0;

    for (int line = 0; line < lines.count; line++)
    {
        String *lineStr = lines.str + line;

        if (CompareChars(lineStr->data, "mask", 4))
        {
            int trim = 7;
            String mask = {lineStr->data + trim, lineStr->length - trim};
            activeMask = mask;

        }
        else
        {
            // its a mem line
            int trim = 4;

            u64 address = ParseU64FromChars(lineStr->data + trim);

            // NOTE(gb) we could use a length count for a number here.
            int lenAddress = 0;
            while (true)
            {
                char ch = lineStr->data[trim + lenAddress++];
                if (!IsNumber(ch))
                    break;
            }

            int trimToVal = 3;
            u64 value = ParseU64FromChars(lineStr->data + trim + lenAddress + trimToVal);

            // printf("before: mem[%llu] = %llu\n", address, value);
            
            int floaters[36] = {};
            int floatCount = 0;

            // here we have the number, so let's mask it with the active mask.
            for (int i = 0; i < activeMask.length; i++)
            {
                char ch = activeMask.data[i];
                // printf("%c", ch);

                u64 shift = activeMask.length - 1 - i;
                if (ch == '1')
                {
                    address = (address | ((u64)1 << shift));
                }
                else if (ch == 'X')
                {
                    floaters[floatCount++] = shift;
                    // NOTE(gb) we could prepare the value here to always be zero.
                }
            }

            
            FloatingValue(address, 0, floaters, floatCount, memory, value, hashTable, ArrayCount(hashTable));

        }
    }

    u64 total = 0;

#if 0
    total = 0;
    for (u64 i = 0; i < memorySize; ++i)
    {
        u64 *com = memory + i;
        // printf("[%llu] %llu\n", i, *com);
        total += *com;
    }
    printf("Answer in array: %llu.\n", total);
#endif

#if 1
    total = 0;
    for (int i = 0; i < ArrayCount(hashTable); ++i)
    {
        HashedCommand *com = hashTable + i;

        while (com)
        {
            total += com->value;
            com = com->next;
        }
    }

    printf("Answer in hashtable: %llu.\n", total);
#endif

    // Time
    u64 _endTime = mach_absolute_time();
    u64 dTicks = _endTime - _startTime;
    u64 nanoseconds = dTicks * global_nanoseconds_per_tick;
    float ms = (float)nanoseconds * 1.0E-6;

    printf("It took %fms.\n", ms);
}
