

#include "malloc.h"
#include <stddef.h> 
#include "strings.h"

extern int __bss_end__;

#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

#define TOTAL_HEAP_SIZE 0x1000000// 16 Megabytes

static void *heap_next = NULL, *heap_max = NULL;

struct header {
    unsigned int size : 31;
    unsigned int status : 1;
}; 

enum {IN_USE = 0, FREE = 1};

void *malloc(size_t nbytes) 
{
    if( nbytes <= 0) {
        return NULL;
    }

    if (!heap_max) { //This chunk of code checks if this is the first time malloc is called, and then initializes the first free block of TOTAL_HEAP_SIZE.
        heap_next = (void *)&__bss_end__;
        heap_max = (char *)heap_next + TOTAL_HEAP_SIZE;
        struct header *hdr = heap_next; //ASK: add one?
        hdr->size = TOTAL_HEAP_SIZE - sizeof(struct header);
        hdr->status = FREE;
    }
    nbytes = roundup(nbytes, 4); 

    struct header *searchHeader = (struct header *)(&__bss_end__); 
    //If the next block is not in use or there is not enough available space, keep on searching for a header that will fit and is free.
    while(((searchHeader->size < (nbytes + sizeof(struct header)))) || (searchHeader-> status == IN_USE)){
        searchHeader = (struct header *)((char *)searchHeader + searchHeader->size + sizeof(struct header));
        if((char *)searchHeader > (char *)heap_max) {
            return NULL;
        }
    }
    heap_next = searchHeader;

    //This chunk of code checks to see if next header will be able to fit nBytes and an end header before hitting Heap_Max.
    if ((char *)heap_next + nbytes + sizeof(struct header) + sizeof(struct header) > (char *)heap_max) 
        return NULL;
    
    void *alloc = (char *) heap_next + sizeof(struct header);
    struct header *h = (struct header *) (heap_next); 
    int oldSize = h->size;
    h->size = nbytes;
    h->status = IN_USE;

    //If the nbytes and the former size are the same then don't add an end header.
    if(nbytes != oldSize) {
        heap_next = (char *)heap_next + nbytes + sizeof(struct header);

        struct header *hdrEnd = heap_next; 
        hdrEnd->size = oldSize - nbytes - sizeof(struct header);
        hdrEnd->status = FREE;
    }
    return alloc;
}

void free(void *ptr) 
{
    if(!ptr) {
        return;
    } 

    struct header *h = (void *)((char *) (ptr) - sizeof(struct header));
    h->status = FREE;

    struct header *oldH = h;
    h = (struct header *)((char *)h + h->size + sizeof(struct header));
        //Combine all conntected free blocks into one, and change first header to new size.
        while(h->status == FREE && ((unsigned int)h < (unsigned int)heap_max)){   
            oldH->size = oldH->size + h->size + sizeof(struct header);
            h = (struct header *) ((char *)h + h->size + sizeof(struct header));
        }
}

void *realloc (void *old_ptr, size_t new_size)
{
    if(old_ptr == NULL) {
        malloc(new_size);
        return NULL;
    }

    struct header *h = (struct header *) ((char *) (old_ptr) - sizeof(struct header)); 
    if(new_size == h->size) {
        return old_ptr;
    }

    if(new_size != 0) {
        new_size = roundup(new_size, 4);
    }

    //Combine all free blocks to find maximum space available. 
    free(old_ptr);

    //If the new size if less than the header size, then update header then add a new end header if appropriate.
    if(new_size <= h->size) { 
        int oldSize = h->size;
        h->size = new_size;
        h->status = IN_USE;
        if(new_size != oldSize) {
            struct header *hdrEnd = (struct header *)((char *)h + new_size + sizeof(h)); //old pointer + new size
            hdrEnd->size = oldSize - new_size - sizeof(struct header);
            hdrEnd->status = FREE;
        }
        return ((char *)h + sizeof(struct header));
    } else { //If the block will not fit into the free space, then malloc onto the end.
        void *new_ptr = malloc(new_size);
        if (!new_ptr) return NULL;
        if(h->size != 0){
            memcpy(new_ptr, old_ptr, new_size);
        }
        return new_ptr;
    }
}




