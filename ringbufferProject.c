/* File: ringbuffer.c
 * ------------------
 * Simple lock-free ring buffer that allows for concurrent
 * access by 1 reader and 1 writer.
 *
 * Author: Philip Levis <pal@cs.stanford.edu>
 * Author: Julie Zelenski <zelenski@cs.stanford.edu>
 */

#include "ringbufferProject.h"
#include "malloc.h"

#define LENGTH 10000

/*
 * A ring buffer is represented using a struct containing a fixed-size array 
 * and head and tail fields, which are indexes into the entries[] array.
 * head is the index of the frontmost element (head advances during dequeue)
 * tail is the index of the next position to use (tail advances during enqueue)
 * Both head and tail advance circularly, i.e. index = (index + 1) % LENGTH
 * The ring buffer is empty if tail == head
 * The ring buffer is full if tail + 1 == head
 * (Note: one slot remains permanently empty to distinguish full from empty)
 */

struct ringbufferProject {
    int entries[LENGTH];
    unsigned int head, tail; 
};


rb_Project_t *rb_Project_new(void) 
{
    rb_Project_t *rb = malloc(sizeof(struct ringbufferProject));
    rb->head = rb->tail = 0;
    return rb;
}

bool rb_Project_empty(rb_Project_t *rb) 
{
    return rb->head == rb->tail;
}

bool rb_Project_full(rb_Project_t *rb) 
{
    return (rb->tail + 1) % LENGTH == rb->head;
}

/*
 * Note: enqueue is called by writer. enqueue advances rb->tail, 
 * no changes to rb->head.  This design allows safe concurrent access.
 */
bool rb_Project_enqueue(rb_Project_t *rb, int elem) 
{
    if (rb_Project_full(rb)) return false;

    rb->entries[rb->tail] = elem;
    rb->tail = (rb->tail + 1) % LENGTH;
    return true;
}

/*
 * Note: dequeue is called by reader. dequeue advances rb->head,
 * no changes to rb->tail. This design allows safe concurrent access.
 */
bool rb_Project_dequeue(rb_Project_t *rb, int *p_elem)
{
    if (rb_Project_empty(rb)) return false;

    *p_elem = rb->entries[rb->head];
    rb->head = (rb->head + 1) % LENGTH;
    return true;
}
