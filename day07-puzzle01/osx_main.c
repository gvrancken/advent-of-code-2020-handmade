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

static bool CompareCString(char *a, char *b, int length)
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

typedef struct String_t
{
    char *data;
    int length;
} String_t;

typedef struct Bag
{
    String_t color;
    String_t desc;
    struct Bag *contains[32];
    int containCount;
} Bag;

static char *CString(String_t s)
{

    char *result = (char *)malloc(s.length + 1);

    for (int i = 0; i < s.length; i++)
    {
        result[i] = s.data[i];
    }
    result[s.length] = 0;

    return result;
}

static void FindBagInContains(Bag *targetBag, Bag *allBags, int allBagsCount, Bag **fitBags, int *fitCount, int depth) {
    for (int f=0; f < allBagsCount; ++f) {
        Bag *bag = allBags + f;
    
        for (int r=0; r<bag->containCount; r++) {
            Bag *ref = bag->contains[r];
            
            // NOTE(gb) we can just compare pointers...
            if (CompareCString(ref->color.data, targetBag->color.data, ref->color.length)) {
                
                for (int d=0;d<depth;d++) {
                    printf("  ");
                }
                printf("bag '%s' can contain '%s'\n", CString(bag->color), CString(targetBag->color));

                bool found = false;
                for (int q=0; q < *fitCount; ++q) {
                    Bag *fitBag = fitBags[q];
                    if (CompareCString(fitBag->color.data, bag->color.data, bag->color.length)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    fitBags[*fitCount] = bag;
                    *fitCount = *fitCount + 1;
                
                    FindBagInContains(bag, allBags, allBagsCount, fitBags, fitCount, depth+1);
                }
            }
        }
        
    }
} 

int main()
{
    debug_read_file_result_t result = ReadFileToHeap("input.txt");

    String_t input;
    input.data = result.contents;
    input.length = result.contentsSize;

    Bag bags[1024];
    int bagsCount = 0;
    bags[0].desc.data = input.data;

    for (int i = 0; i < input.length; i++)
    {
        char *ch = input.data + i;

        if (*ch == '\n')
        {
            Bag *bag = bags + bagsCount;
            bag->desc.length = input.data + i - 1 - bag->desc.data;
            bagsCount++;
            bags[bagsCount].desc.data = input.data + i + 1;
        }
    }
    bags[bagsCount].desc.length = input.data + input.length - bags[bagsCount].desc.data;
    bagsCount++;

    printf("number of bags: %d\n", bagsCount);

    // fill each bag with its color
    // also grab out a pointer to our targetBag when we're at it.
    Bag *targetBag;
    char *targetColor = "shiny gold";

    for (int i = 0; i < bagsCount; i++)
    {
        Bag *bag = bags + i;
        char *str = bag->desc.data;

        // get the color of the bag
        int bagsIndex[64] = {};

        for (int j = 0; j < bag->desc.length - 3; j++)
        {

            if (CompareCString(" bag", str + j, 4))
            {
                bag->color.data = str;
                bag->color.length = j;
                if (CompareCString(bag->color.data, targetColor, bag->color.length)) {
                    targetBag = bag;
                }
                break;
            }
        }
    }

    // fill contains property with the pointers to their respective bags
    for (int i = 0; i < bagsCount; i++)
    {
        Bag *bag = bags + i;
        
        // printf("desc: %s\ncolor: %s\n", CString(bag->desc), CString(bag->color));

        char *str = bag->desc.data;
        int bagsFound = 0;
        for (int j = 0; j < bag->desc.length - 3; j++)
        {
            if (CompareCString(" bag", str + j, 4))
            {

                if (bagsFound == 0) {
                    // this is the color, skip it.
                    bagsFound++;
                    continue;
                }
                // is it an empty bag?
                if (CompareCString("no other", str + j - 8, 8))
                {
                    break;
                }
                // backtrack to numeric to get each bag
                int n = j;
                while (true)
                {
                    char ch = str[n];
                    if (IsNumber(ch))
                        break;
                    n--;
                }
                String_t clr = {};
                clr.data = str + n + 2;
                clr.length = j - (n + 2);
                
                for (int f=0; f < bagsCount; ++f) {
                    Bag *ref = bags + f;
                   
                    if (CompareCString(ref->color.data, clr.data, clr.length)) {
                        bag->contains[bag->containCount++] = ref;
                        break;
                    }
                }
            }   
        }
    }

    Bag *fitBags[512] = {};
    int fitCount = 0;

    FindBagInContains(targetBag, bags, bagsCount, fitBags, &fitCount, 0);
    
    printf("Number of bags that fit: %d.\n", fitCount);

    printf("Bags that can (eventually) contain '%s':\n", CString(targetBag->color));

#if 1
    for (int f=0; f < fitCount; ++f) {
        Bag *bag = fitBags[f];
        printf("%s", CString(bag->color));
        if (f != fitCount - 1) {
            printf(", ");
        }
    }
#endif

}
