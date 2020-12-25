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

typedef struct v2
{
    int x;
    int y;
} v2;

static inline v2 V2(int a, int b)
{
    v2 result = {a, b};
    return result;
}

typedef struct StringArray
{
    String str[4096];
    int count;
} StringArray;

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

typedef struct HashedCommand
{
    u64 value;
    u64 address;
    struct HashedCommand *next;
    bool isInit;
} HashedCommand;


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

static HashedCommand *
GetFromHash(HashedCommand *table, int tableSize, u64 address)
{
    int hash = address & (tableSize - 1);
    HashedCommand *itemInList = table + hash;

    if (itemInList->address == address)
    {
        return itemInList;
    }
    else if (!itemInList->isInit)
    {
        HashedCommand com = {};
        com.address = address;
        com.isInit = true;
        *itemInList = com;
        return itemInList;
    }
    else
    {
        while (itemInList->next)
        {
            // printf("<--- Collision! [%d] [%llu]\n", hash, address);
            itemInList = itemInList->next;
            if (itemInList->address == address)
            {
                // printf("<--- Found ON HEAP [%d] [%p]\n", hash, itemInList);
                return itemInList;
            }
        }

        Assert("we have not found this!");
        return 0;
    }
}

static void StoreInHash(HashedCommand *table, int tableSize, HashedCommand item)
{
    // hash the position, since we want to search an entity based on position
    int hashSlot = item.address & (tableSize - 1);

    // since there is always an entity in the slot (and not a null pointer), we need to check a value to see if it's an empty entity. In this example we assign a type ENTITY_UNINITIALISED to check for this.
    HashedCommand *itemInList = table + hashSlot;
    if (!itemInList->isInit) {
        // array pos is empty, just store
        item.isInit = true;
        *itemInList = item;
    } else {
        // hash collision! 
        
        // traverse the next pointers until we find an empty spot
        while (itemInList->next) {
            itemInList = itemInList->next;
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
    const u64 memorySize = 80000;
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

            // here we have the number, so let's mask it with the active mask.
            for (int i = 0; i < activeMask.length; i++)
            {
                char ch = activeMask.data[i];
                // printf("%c", ch);

                u64 shift = activeMask.length - 1 - i;
                if (ch == '1')
                {
                    value = (value | ((u64)1 << shift));
                }
                else if (ch == '0')
                {
                    value = (value | ((u64)1 << shift)) ^ ((u64)1 << shift);
                }
            }

            Assert(value < 68719476736); // 36-bit max
            Assert(address < memorySize);

            // printf("after: mem[%llu] = %llu\n", address, value);

            // store in hash
            HashedCommand hashCom = {};
            hashCom.value = value;
            hashCom.address = address;
            hashCom.isInit = true;

            StoreInHash(hashTable, ArrayCount(hashTable), hashCom);

            memory[address] = value;

        }

    }


    printf("\n");

    // qsort(hashTable, ArrayCount(hashTable), ArrayCount(hashTable) * sizeof(HashedCommand), comp);

    u64 total = 0;

    for (u64 i = 0; i < memorySize; ++i) {
        u64 *com = memory + i;
        // printf("[%llu] %llu\n", i, *com);
        total += *com;
    }

#if 0
    for (int i = 0; i < ArrayCount(hashTable); ++i)
    {
        HashedCommand *com = hashTable + i;

        // printf("mem[%d] = ", i);
        while (com)
        {
            if (!com->isInit) break;
            printf(" ");

            total += com->value;
            printf("[%llu] %llu\n", com->address, com->value);
            com = com->next;
        }
            
        
        // printf("total is now: %llu\n", total);
    }
#endif

    printf("Answer: %llu.\n", total);


    // Time
    u64 _endTime = mach_absolute_time();
    u64 dTicks = _endTime - _startTime;
    u64 nanoseconds = dTicks * global_nanoseconds_per_tick;
    float ms = (float)nanoseconds * 1.0E-6;

    printf("It took %fms.\n", ms);
}
