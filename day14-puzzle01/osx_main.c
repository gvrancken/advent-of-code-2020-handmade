#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define dim 256

void binary(char *nToBin, int n);
void addMask(char *mask, char *nTB, char *r);
long backToDecimal(char *result);

int main(){
    FILE *f = fopen("input.txt", "r");
    char s[dim], **v, nToBin[37], result[37];
    int counter = 0, i = 0, k, y; 
    int n, max = 0;
    long converted, *memory;
    long R = 0;
    if (f == NULL)
        return 0;
    
    fgets(s, dim, f);
    counter++;
    while (feof(f) != 1){
        fgets(s, dim, f);
        counter++;
    }
    fclose(f);

    //create the bi-array
    v = (char **)malloc(counter*sizeof(char **));
    f = fopen("input.txt", "r");
    while (feof(f) != 1){
        v[i] = (char *)malloc(dim*sizeof(char));
        fgets(v[i], dim, f);
        i++;
    }
    fclose(f);

    //find the max memory address
    i = 0;
    while (i < counter){
        k = 0;
        while (k < strlen(v[i])){
            if ((v[i][k] >= 48 && v[i][k] <= 57) || v[i][k] == '=')
                break;
            k++;
        }
        n = atoi(&v[i][k]);
        if (n > max)
            max = n;
        i++;
    }
    printf ("max: %d\n", max);

    //create memory array
    memory = (long *)malloc(sizeof(long)*(max+1));
    for (i=0; i<=max; i++)
        memory[i] = 0;
    
    for (i=0; i<counter; i++){
        k = 0;
        y = 0;
        if (v[i][1] == 'a'){
            while (k < strlen(v[i])){
                if (v[i][k] == '=')
                    break;
                k++;
            }
            //curson on the =
            k += 2;
            while (k < strlen(v[i])){
                s[y] = v[i][k];
                y++;
                k++;
            }
            //s is the mask
            s[y] = '\0';
            printf ("\n%s\n", s);
            //printf ("\n");
        }
        else if (v[i][1] == 'e'){
            //get the value in base 2
            while (k < strlen(v[i])){
                if (v[i][k] == '=')
                    break;
                k++;
            }
            k += 2;
            n = atoi(&v[i][k]);
            //printf ("\n%d\n", n);
            binary(nToBin, n);
            //printf ("\n%s\n", nToBin);
            addMask(s, nToBin, result);
            //printf ("number + mask: %s\n", result);
            //get the memory address
            n = atoi(&v[i][4]);
            //printf ("add: %d\n", n); 
            converted = backToDecimal(result);
            //printf ("back: %li", converted);
            memory[n] = converted;
        }
    }
    for (i=0; i<=max; i++){
        if (memory[i] != 0){
            printf("[%d] %ld\n", i, memory[i]);
            R += memory[i];
        }
    }
    printf ("\ntotal: %ld\n", R);

    for (i=0; i<counter; i++){
        free(v[i]);
    }
    free(v);
    free(memory);
    return 0;
}

void binary(char *nToBin, int n){
    int pos = 35, i = 0, bin[36];
    for (i=0; i<36; i++)
        bin[i] = 0;

    nToBin[36] = '\0';
    while (pos >= 0 && n != 0){
        bin[pos] = (n%2);
        n /= 2;
        pos--;
    }
    if (pos != 0){
        while (pos >= 0){
            bin[pos] = 0;
            pos--;
        }
    }
    for (i=0; i<36; i++){
        nToBin[i] = bin[i]+'0';
    }
}

void addMask(char *mask, char *nTB, char *r){
    int i = 35;
    char rep = '0';
    r[36] = '\0';

    for(; i>=0; i--){
        if (mask[i] == 'X')
            r[i] = nTB[i];
        else 
            r[i] = mask[i];
    }
}

long backToDecimal(char *result){
    int i = 35, counter = 0;
    unsigned long n = 0, t;
    char l;

    while (i >= 0){
        l = result[i];
        t = atoi(&l);
        n += t*(pow(2, counter));
        counter++;
        i--;
    }
    return n;
}   