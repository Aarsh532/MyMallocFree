#include <stdlib.h>
#include <stdio.h>

#include "mymalloc.h"

#define ERR(x) log_warn(x, filename, line)


#define MEMSIZE 4096
#define HEADERSIZE sizeof(struct block)


struct block {
    size_t sizeB;
    int free;
};

static char memory[MEMSIZE] = {0};


static void log_warn(const char *warning, const char *fname, int line)
{
    fprintf(stderr, "File: %s Line: %d %s\n", fname, line, warning);
}


static void initialize(void)
{
    struct block *m = (struct block *) memory;

    m->sizeB = MEMSIZE - HEADERSIZE;
    m->free = 1;
}

void *mymalloc(size_t size, char *filename, int line)
{
    struct block *m;
    static int initialized = 0;
    char *mem_byte = memory;
    const char *const mem_b = memory + MEMSIZE;

    if (!size)
    {
        return NULL;
    }

    //Checks if the heap is initalized.
    if (initialized == 0) 
    {
        initialize();
        initialized = 1;
    }

    //Goes through the heap in search of a free metadata block using pointer arthmitic
    while (mem_byte < mem_b)
    {
        m = (struct block *) mem_byte;
        if (m->free == 1 && (m->sizeB >= size))
        {
            break;
        }

        mem_byte = mem_byte +  HEADERSIZE + m->sizeB;
    } 


    mem_byte = mem_byte +  HEADERSIZE;

    //Checks if the byte is in bounds
    if (mem_byte >= mem_b) 
    {
        ERR("Out of Memory Warning");
        return NULL;
    }

    //Splits the block if total size is greater than the size required
    if (m->sizeB > size) 
    {
        struct block *next = (struct block *) (mem_byte + size);
        
        if ((char *)(next + 1) <= mem_b) 
        {
            next->free = 1;
            next->sizeB = m->sizeB - (size + sizeof(*next));
        }

    }
    m->free = 0;
    m->sizeB = size;
    return (void *) mem_byte;
}

//Function for cheching if pointer is in rour memory
static int inMem(void *ptr)
{
    char *temp = ptr;
    const char *const mem_b = memory + MEMSIZE;

    return (temp < memory) || (temp >= mem_b);
}


//Function to check if address is correct
static int correctAddress(void *ptr)
{
    
    char *mem_byte = memory;
    struct block *m;
    const char *const mem_b = memory + MEMSIZE;
    
    while(mem_byte < mem_b)
    {
        m = (struct block *) mem_byte;
        if(mem_byte + HEADERSIZE == ptr)
        {
            return 0;
        }
        mem_byte = mem_byte +  HEADERSIZE + m->sizeB;
    }
    return 1;

}

static void merge(void)
{
    struct block *first, *next;
    int freeSize;
    const char *const mem_b = memory + MEMSIZE;
    char *mem_byte = memory;

    do 
    {
        first = (struct block *) mem_byte;
        mem_byte += first->sizeB + sizeof(*first);
        if (first->free) 
        {
            while ((mem_byte + sizeof(*first)) <= mem_b) 
            {
                next = (struct block *) mem_byte;
                if (!next->free)
                    break;
                freeSize = next->sizeB + sizeof(*next);
                first->sizeB += freeSize;
                mem_byte += freeSize;
            }
        }
    } while ((mem_byte + sizeof(*first)) <= mem_b);
}

void myfree(void *ptr, char *filename, int line)
{
    struct block *m = (struct block *) ((char *) ptr - HEADERSIZE);;
    const char *err = NULL;



    //Checks if the ptr is null
    if (ptr == NULL)
    {
        err = "NULL Pointer Warning";
        ERR(err);
        return;
    }
    //need to check that pointer is valid and within the range of memory
    else if (inMem(ptr))
    {
        err = "Pointer Not in Memory Warning";
        ERR(err);
        return;
    }
    //Checks to make sure adress is correct
    else if (correctAddress(ptr))
    {
        err = "Incorrect Address Warning";
        ERR(err);
        return;
    }

    if (m->free == 0) 
    {
        m->free = 1;
        merge();
    } else 
    {
        ERR("Redundant Freeing Warning");
    }
}