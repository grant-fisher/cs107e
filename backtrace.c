#include "backtrace.h"
#include "printf.h"

int backtrace(struct frame f[], int max_frames)
{
    unsigned int *cur_fp;
   	__asm__("mov %0, fp" : "=r" (cur_fp));
	unsigned int * name;
	unsigned int * savedFP = cur_fp - 3;
	unsigned int numCalls = 0;

	//If savedFrame pointer of caller is not 0, then program has not reached the top frame yet. If hit max frame bfore then, only print the frames specified.
	while(*savedFP != 0 && numCalls < max_frames) {
		struct frame currentFrame;
		
		//This line retrieves the contents of the saved FP which is the address of the caller saved pc.
		unsigned int * savedPC = (unsigned int *)*(savedFP); 

		currentFrame.resume_offset = ((*(cur_fp - 1)) - (*savedPC)) + 12; //Subtracts savedPC from saved LR to get positive value for offset, to determine how many lines of funciton have passef befroe calling callee.
		currentFrame.resume_addr = (void *)*(cur_fp - 1); //Accesses contents of the link register which holds the address of where to resume in caller frame.
		
		cur_fp = (unsigned int *)*savedFP;
		savedFP = cur_fp - 3; 
		name = (unsigned int *)(*cur_fp - 0x10);

		if((*name >> 24) == 0xff) { //Determine if the address's contents start with a 0xff which indicates it is a name, then finds the length of name to find address with name in it, or else return ???.
			int length = *name - 0xff000000;
			name -= (length/4); 
			currentFrame.name = (char *)name;
		} else {
			currentFrame.name = "???";
		}

		f[numCalls] = currentFrame;
		numCalls++;
	}
    return numCalls;
}

void print_frames (struct frame f[], int n)
{

    for (int i = 0; i < n; i++)
        printf("#%d %p at %s+%d\n", i, f[i].resume_addr, f[i].name, f[i].resume_offset);
}

void print_backtrace (void)
{
    int max = 50;
    struct frame arr[max];

    int n = backtrace(arr, max);
    print_frames(arr+1, n-1);   // print frames starting at this function's caller
}
