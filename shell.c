#include "shell.h"
#include "shell_commands.h"
#include "strings.h"
#include "malloc.h"
#include "keyboard.h"
#include "pi.h"
#include "printf.h"
#include <stdint.h>
#include "uart.h"
#include "graphics.h"
#include "draw.h"
#include "printf.h"
#include "draw.h"
#include "interrupts.h"
#include "gl.h"
#include "mouse.h"
#include "console.h"
#include "gpio.h"
#include "ringbufferProject.h"
#include "assert.h"
#include "graphics.h"
#include "axis.h"
#include "pen.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "console.h"


static int (*shell_printf)(const char * format, ...);
static int (*shell_putchar)(int ch);
int cmd_history(int argc, const char *argv[]);
int cmd_disassemble(int argc, const char *argv[]);
void historyFunc(const char *line);
int cmd_shape(int argc, const char *argv[]);
int cmd_draw(int argc, const char *argv[]);

#define LINE_LEN 80
#define cols 40
#define rows 20
#define speed 4000

#define XCOILPIN1 10
#define XCOILPIN2 9
#define XCOILPIN3 20
#define XCOILPIN4 21

#define YCOILPIN1 8
#define YCOILPIN2 7
#define YCOILPIN3 5
#define YCOILPIN4 6


static int indexOfHistory = 0;
static char history[10][80];
static int first;

static const command_t commands[] = {
    {"help",   "<cmd> prints a list of commands or description of cmd", cmd_help},
    {"echo",   "<...> echos the user input to the screen", cmd_echo},
    {"reboot", "reboot the Raspberry Pi back to the bootloader", cmd_reboot},
    {"peek", "Prints the contents (4 bytes) of memory at hex_addr", cmd_peek},
    {"history", "displays the history of recent commands, each prefixed with its command number", cmd_history},
    {"poke", "Stores `hex_val` into the memory at `hex_addr`.", cmd_poke},
    {"decode", "Decodes instructions into assembly.", cmd_disassemble},
    {"shape", "Draws shape with N number of sides", cmd_shape},
    {"draw", "Enter paint application", cmd_draw}

};

static const char *cond[16] = {"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", 
                               "hi", "ls", "ge", "lt", "gt", "le", "", ""};
static const char *opcodes[16] = {"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc", 
                                  "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"};

struct insn  { 
    uint32_t reg_op2:4;     //Rm register
    uint32_t one:1;  
    uint32_t shift_op: 2;   //shift create array
    uint32_t shift: 5;      //Shift amount
    uint32_t reg_dst:4;     //RD register
    uint32_t reg_op1:4;     //RN registers
    uint32_t s:1;
    uint32_t opcode:4;
    uint32_t imm:1;
    uint32_t kind:2;
    uint32_t cond:4; 
};

//This chunk of code, dissasembles instrutions and prints them out as assembly.
void decode(unsigned int *addr) 
{
    struct insn in = *(struct insn *)addr;
    int code = in.opcode;
    int con = in.cond;
    int op1 = in.reg_op1;       //RN
    int op2 = in.reg_op2;       //RM
    int regDst = in.reg_dst;    //RD
    int kind = in.kind;
    int one = in.one;
    int shift = in.shift_op;   

    if(kind == 0 && one == 0 ) { //Data processing immmediate shift 
        if (code == 4 || code == 2 || code == 3  || code == 5 || code == 6 || code == 7) { //ADD, SUB, RSB, ADC, SBC, and RSC
            shell_printf("%p: %x %s R%d, R%d #%d \n", addr, *addr, opcodes[code], regDst, op1, (op2 >> shift));
        } else if (code == 8 || code == 9) { //TST TEQ
            shell_printf("%p: %x %s %s R%d, R%d R%d \n", addr, *addr, opcodes[code], cond[con], regDst, op1, (op2 >> shift));
        } else if (code == 10 || code == 11) { //CMP CMN
            shell_printf("%p: %x %s R%d, #%d \n", addr, *addr, opcodes[code], op1, (op2 >> shift));
         } else if (code == 13 || code == 15 ){ //MOV MVN
            shell_printf("%p: %x %s %s R%d, R%d \n", addr, *addr, opcodes[code], cond[con], regDst , op2);
        }  else if (code == 0 || code == 1 || code == 12 || code == 14) { //AND, ORR, EOR, BIC, and ORN
            shell_printf("%p: %x %s %s R%d, R%d #%d \n", addr, *addr, opcodes[code], cond[con], regDst , op1, op2);
        } else {
            shell_printf("%p: %x \n", addr, *addr);
        }
    } else {
        shell_printf("%p: %x \n", addr, *addr);
    }
}

//This function draws shapes using the CNC drawing machine
int cmd_shape(int argc, const char *argv[]) 
{
    if(argc != 6) {
        shell_printf("error: shape expects 5 arguments [x] [y] [number of sides] [height] [width] \n");
        return 1;
    }

    int shapeArg[5];

    for(int i = 1; i <= 5; i++){
        char *endptr = NULL;
        unsigned int hexInt = strtou(argv[i], &endptr, 10); 
        if(*endptr != '\0') {
            printf("*endptr: %d \n", *endptr);
            shell_printf("error: shape cannot convert '%s' \n", argv[i]);
            return 1;
        } 
        shapeArg[i] = (int)hexInt;
    }

    //Adds this funciton so that the pen and the axis only get initilized once within the shell
    if(first){
        pen_init();
        axis_init(XCOILPIN1, XCOILPIN2, XCOILPIN3, XCOILPIN4, YCOILPIN1, YCOILPIN2, YCOILPIN3, YCOILPIN4);
        first = 0;
    }

    drawShape(shapeArg[1], shapeArg[2], shapeArg[3], shapeArg[4], shapeArg[5], speed);
    return 0;
}

//This funciton enters mouse painting mode using the CNC drawing machine
int cmd_draw(int argc, const char *argv[])  
{
    if(first){
        pen_init();
        axis_init(XCOILPIN1, XCOILPIN2, XCOILPIN3, XCOILPIN4, YCOILPIN1, YCOILPIN2, YCOILPIN3, YCOILPIN4);
        first = 0;
    }
    image();
    gl_init(cols*gl_get_char_width(), rows*gl_get_char_height(), GL_DOUBLEBUFFER); //Initializes the fb back to doublebuffering from single to support shell.
    return 0;
}

int cmd_echo(int argc, const char *argv[]) 
{
    for (int i = 1; i < argc; ++i) 
        shell_printf("%s ", argv[i]);
        shell_printf("\n");
    return 0;
}

//This funciton restarts the bootloader.
int cmd_reboot(int argc, const char *argv[]) 
{
    pi_reboot();
    return 0;
}

//This command decodes instructions to assembly.
int cmd_disassemble(int argc, const char *argv[]) {
    char *endptr = NULL;
    unsigned int hexInt = strtou(argv[1], &endptr, 16);
    decode((unsigned int * )hexInt);
    return 0;
}

//This command prints out the command history most recently inputted.
int cmd_history(int argc, const char *argv[]) 
{
    int index = 0;
    if(indexOfHistory > 10){
        index = 10;
    } else {
        index = indexOfHistory;
    }
    for(int i = 0; i < index; i++){
        shell_printf("%d %s \n", i+1, history[i]);
    } 
    return 0;
}

//This command prints out the list of commands and their uses.
int cmd_help(int argc, const char *argv[]) 
{
    int sizeCommands = sizeof(commands)/sizeof(commands[0]);
    if(argc == 1){
        for(int index = 0; index < sizeCommands; index++){
            shell_printf("%s: %s \n", commands[index].name, commands[index].description);
        }
    } else if (argc == 2){
        for(int i = 0; i < sizeCommands; i++){
            if(strcmp(commands[i].name, argv[1]) == 0){ 
                shell_printf("%s: %s \n", commands[i].name, commands[i].description); 
                return 1;
            }
        }
        shell_printf(" error: no such command '%s'.\n", argv[1]); 
    }
    return 0;
}

//This function looks at an address and then prints out their contents.
int cmd_peek(int argc, const char* argv[]) {
    if(argc <=1){ 
        shell_printf("error: peek expects 1 argument [hex address] \n");
        return 1;
    }
    char *endptr = NULL;
    unsigned int hexInt = strtou(argv[1], &endptr, 16); 
    if(*endptr != '\0') {
        shell_printf("error: peek cannot convert '%s' \n", argv[1]);
        return 1;
    } 
    if ((hexInt % 4) != 0) {
        shell_printf("error: peek address must be 4-byte aligned \n");
        return 1;
    }
    int * pointer = (int *) hexInt; 
    shell_printf("%p: %08x \n", pointer, *pointer);    
    return 0;
}

//This function changes what is within an address provided by the user.
int cmd_poke(int argc, const char* argv[]) {
    if(argc < 3){
        shell_printf("error: poke expects 2 arguments [hex address] [hex value] \n");
        return 1;
    } 
    unsigned int arg1 = 0;
    for(int argIndex = 1; argIndex < 3; argIndex++){
        char *endptr = NULL;
        unsigned int hexInt = strtou(argv[argIndex], &endptr, 16);
        if(*endptr != '\0') { //If it is not an Int, then return that it did not work.
            shell_printf("error: peek cannot convert '%s' \n", argv[argIndex]);
            return 1;
        }
        if (argIndex == 1) { 
            arg1 = hexInt;
            if ((hexInt % 4) != 0) {
                shell_printf("error: peek address must be 4-byte aligned \n");
                return 1;
            }
        }
        if (argIndex == 2) { //Take the update the contents within the address given by user. 
            int * pointer = (int *) arg1; 
            *pointer = hexInt;
            return 0;
        }
    }
    return 0;
}

void shell_init(formatted_fn_t print_fn, char_fn_t putchar_fn)
{
    first = 1;
    shell_printf = print_fn;
    shell_putchar = putchar_fn;
}

void shell_bell(void)
{
    shell_putchar('\a');
}

void leftArrow(void){ //0xBA
    shell_putchar('\b');
}

void rightArrow(char * pointBuf, int bufSize){ //0xBA
    char repeatChar = *pointBuf;
    shell_putchar(repeatChar);
}

//shell_readline reads the line characters typed on the keyboard and stores them into a buffer. 
//Reading for the current line stops when the user types enter (\n).
void shell_readline(char buf[], int bufsize)
{

    int currSize = 0;
    int curser = 0;
    char currChar = keyboard_read_next();
    while(currChar != '\n' && currSize <= bufsize) {
        if(currChar == '\b'){ //If the user presses backspace, either delete or notify that it is the end.
            if(currSize == 0){
                shell_bell();
            } else {
                shell_putchar('\b');
                buf[currSize - 1] = ' ';
                shell_putchar(' ');
                shell_putchar('\b');
                currSize--;
                curser--;
            }
        } else if (currChar == 0xa7) {
            curser--;
            leftArrow();

        } else if(currChar == 0xa9){
            char * pointBuf = buf + curser;
            curser--;
            rightArrow(pointBuf, bufsize);

        } else {
            if(currChar >= '\t' && currChar <= 0x80) { //Make sure it is an acceptable printable character before printing it.
                shell_putchar(currChar);
                buf[curser] = currChar;
                currSize++;
                curser++;
            }
        }
        currChar = keyboard_read_next();
    } 
    if(currChar == '\n'){
        shell_putchar(currChar);
        buf[currSize] = currChar;
    }

}

//This function keeps track of past commands that the user typed.
void historyFunc(const char *line){
    if(indexOfHistory < 10){ 
        for(int i = 0; *(line+i) != '\n'; i++){
            history[indexOfHistory][i] = *(line+i);
        } 
    } else {
        for(int index = 1; index <= 10; index++){ 
            for(int clear = 0; history[index-1][clear] != '\0'; clear++){ //Clear the new line, that way there is no leftover data.
                history[index-1][clear] = '\0';
            }
            if(index == 10){
                for(int i = 0; *(line+i) != '\n'; i++){
                    history[9][i] = *(line+i);
                }
            } else {
                for(int j = 0; history[index][j] != '\0'; j++){ //Swith previous string to higher priority.
                    history[index-1][j] = history[index][j];
                }
            }
        }
    }
    indexOfHistory++;
}

//This funciton checks for spaces within the line buffer.
int strlenSpaces(const char *s) {
    int i;
    for (i = 0; s[i] != ' '; i++){
        if(s[i] == '\n'){
            break;
        }
    }
    return i;
}

//Tokenize seperates the buffer into sepereate words.
int tokenize(char ** pointerTokenArr, const char *line){

    int length = 0;
    int numTokens = 0;
    int lineCount = 0;

    while (*line != '\n') { //Find the length of words, but not the length of spaces.
        length = 0;
        do {
            length = strlenSpaces(line);
            if(length == 0) {
                line++;
                lineCount++;
                if(*line == '\n'){
                    return numTokens;
                }
            }
        } while (length == 0);

        char * newWord = (char *)malloc(length + 1);
        char * memcpyWord = memcpy(newWord, line, length); 
        
        newWord += length;
        *newWord = '\0'; 
        pointerTokenArr[numTokens] = memcpyWord; 
        numTokens++;
        line += length;
        lineCount += length;
    }
    return numTokens;
}

//This funciton evaluates the buffer, finds the arguments, then gets the calls the commands.
int shell_evaluate(const char *line) 
{
    if(*line == '\n'){
        return 1; 
    }
    char * tokenArr[LINE_LEN/2];
    char ** pointerTokenArr = tokenArr;

    historyFunc(line);

    int numTokens = tokenize(pointerTokenArr, line);
    int sizeOfCommands = sizeof(commands)/sizeof(commands[0]);
    int index;
    for(index = 0; index < sizeOfCommands; index++){  //This chunk of code searches for if the first argument is a command.
        if(strcmp(commands[index].name, tokenArr[0]) == 0){ 
            break;
        } 
        if(index == (sizeOfCommands - 1)){
            shell_printf("error: no such command '%s'. \n", tokenArr[0]);
            return 1; 
        }
    }
    
    switch(index){
        case 0: //help
            cmd_help(numTokens, (const char **)tokenArr);
            break;
        case 1: //echo
            cmd_echo(numTokens, (const char **)tokenArr);
            break;
        case 2: //reboot
            cmd_reboot(numTokens,(const char **)tokenArr);
            break;
        case 3: //peek
            cmd_peek(numTokens,(const char **)tokenArr);
            break;
        case 4: //history
            cmd_history(numTokens,(const char **)tokenArr);
            break;
        case 5: //poke
            cmd_poke(numTokens,(const char **)tokenArr);
            break;
        case 6: //disassemble
            cmd_disassemble(numTokens,(const char **)tokenArr);
            break;
        case 7:                                                //NEW
            cmd_shape(numTokens,(const char **)tokenArr);
            break;
        case 8:                                                //NEW
            cmd_draw(numTokens,(const char **)tokenArr);
            break;
    }
       
    for(int i = 0; i < numTokens; i++){
        free(tokenArr[i]);
    }
    return 0;
}

void shell_run(void)
{
    //shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    while (1) 
    {
        char line[LINE_LEN];
        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}

