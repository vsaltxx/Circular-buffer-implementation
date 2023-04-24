/*** ------------------------------- ***
 *
 * tail.c
 * Řešení IJC-DU2, příklad 1), 17.4.2023
 * Autor: Veranika Saltanava, FIT
 * Přeloženo: gcc 11.3.0
 *
 *** ------------------------------- ***/


#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define DEFAULT_LINE_COUNT 10
#define MAX_LINE_LENGTH 4095

/////////////////////////////////////////////
/*** --- Circular buffer realization --- ***/
/////////////////////////////////////////////

typedef struct {
    char **buffer;                  //buffer array of pointers to strings of the maximum line size
    int head;                       //index of the head/end of the buffer
    int tail;                       //index of the tail/start of the buffer
    int size;                       //maximum size of the buffer
    int count;                      //number of lines(strings) in the buffer
} circular_buffer;

/*** ------------------------------------------------------------------------- ***
 *
 * First allocate memory to the circular_buffer structure and check if the
 * memory was allocated successfully. If not, return NULL. Then initialize the fields of the
 * circular_buffer structure.
 *
 * param:   n - number of lines(strings) in the buffer ; the number of lines to output
 * return:  pointer to the circular buffer structure
 *
 *** ------------------------------------------------------------------------- ***/

circular_buffer* cb_create(int n){
    circular_buffer *cb = malloc(sizeof(circular_buffer));
    if (cb == NULL) {
        return NULL;
    }
    cb->buffer = (char**)calloc(n, sizeof(char*));
    if (cb->buffer == NULL) {
        return NULL;
    }
    cb->head    = 0;
    cb->tail    = 0;
    cb->count   = 0;
    cb->size    = n;

    return cb;
}


/*** ------------------------------------------------------------------------- ***
 *
 * The function cb_put works directly with the circular buffer that stores lines.
 * It takes as arguments a pointer to the circular buffer (cb) and a pointer to
 * the string (line) to be added to the buffer. The function calculates the
 * index into which the line will be added using the % operation and then copies
 * the line to the buffer.
 *
 *  param:  cb - pointer to the circular buffer,
 *          line - pointer to the string to put in the buffer
 *  return: true if the string was put in the buffer,
 *          false if there was a mistake
 *
 *** ------------------------------------------------------------------------- ***/

char cb_put(circular_buffer *cb, char *line){
    if (cb == NULL || line == NULL) {     //check that the circular buffer and the line have been created
        return -1;
    }
    if (strlen(line) > MAX_LINE_LENGTH) {                   //check that the line is not too long
        fprintf(stderr, "ERROR: Line is too long.\n");
        return -1;
    }

    // allocate memory for the new line and copy in it the line from the input
    // 1 extra byte for the null terminator '\0'
    char *new_line = (char*)malloc(strlen(line) + 1);
    if (new_line == NULL) {
        fprintf(stderr, "ERROR: Memory allocation error.\n");
        return -1;
    }

    strcpy(new_line,line);
    new_line[strlen(line)] = '\0';

    if (cb->buffer[cb->head] != NULL)
        free(cb->buffer[cb->head]);                     //free the memory occupied by the old line

    cb->buffer[cb->head] = new_line;                        //put the new line in the buffer
    cb->head = (cb->head + 1) % cb->size;                   //update the head index ;

                                                            // when the index exceeds the buffer size,
                                                            // it is "wrapped" back to the beginning of the buffer
                                                            // using the % operation

    if (cb->count < cb->size) {                             //if the buffer is not full
        cb->count++;                                        //update the number of lines in the buffer
    }

    return 1;
}

/*** ---------------------------------------------------------------------- ***
 *
 * The function starts by checking that the buffer is not empty, otherwise it
 * returns NULL. The function then gets the string from the buffer head or from
 * the buffer tail, updates the its idx, and decrements the number of elements in
 * the buffer. I compare the head and tail indexes, because I have to print
 * from the oldest line to the newest one, not from the 0 idx (tail). Finally,
 * the function returns the string received from the buffer.
 *
 * param:   cb - pointer to the circular buffer
 * return:  pointer to the string
 *
 *** ---------------------------------------------------------------------- ***/

char* cb_get(circular_buffer *cb){
    if (cb->count == 0) {                   //check that the buffer is not empty
        return NULL;
    }
    char *line;

    if (cb->head != cb->tail) {
        line = cb->buffer[cb->head];
        cb->head = (cb->head + 1) % cb->size;   //update the head index
    }
    else {
        line = cb->buffer[cb->tail];
    }
    cb->tail = (cb->tail + 1) % cb->size;   //update the tail index
    cb->count--;                            //update the number of lines in the buffer

    return line;
}

/*** ---------------------------------------------------------------------- ***
 *
 * The function starts by checking that a circular buffer has been created and
 * that an array of string pointers has been allocated.
 * If not, the function simply returns control.
 * The function frees the memory occupied by the array of pointers to
 * strings and the circular buffer structure using the free function.
 *
 * param: cb - pointer to the circular buffer
 * return: -
 *
 *** ---------------------------------------------------------------------- ***/

void cb_free(circular_buffer *cb){
    if (cb == NULL) {           //check tha the circular buffer has been created
        return;
    }
    for (int i = 0; i < cb->count; i++) {
        if (cb->buffer[i] != NULL) {
            free(cb->buffer[i]);    //free the memory occupied by the strings
        }
    }
    //free the memory occupied by the circular buffer structure
    free(cb->buffer);
    free(cb);
}

/*** ---------------------------------------------------------------------- ***
 *
 * The function prints the last n lines of the input file.
 *
 * param: cb - pointer to the circular buffer
 *        n - number of lines to output
 * return: -
 *
 *** ---------------------------------------------------------------------- ***/

void cb_last_n(circular_buffer *cb, int n) {
    if (n <= 0 || cb == NULL || cb->count == 0) { // check for invalid input or empty buffer
        return;
    }

    for (int i = 0; i < n; i++) {
        char *line = cb_get(cb);
        if (line == NULL) {
            break;
        }

        printf("%s", line);
        free(line);
    }
}

/*** --------------------------------------------------------------------- ***
 *                               Parsing arguments
 *** --------------------------------------------------------------------- ***/


typedef struct {
    int n;          //number of lines to output
    FILE *input;    //pointer to the input file
} arguments;

int string_to_int(char *str){
    int n = 0;
    for (int i = 0; i < (int)strlen(str); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return -1;
        }
        n = n * 10 + (str[i] - '0');
    }
    return n;
}


/*** ---------------------------------------------------------------------- ***
*
* param:   args - pointer to the structure containing the arguments
*          argc - number of arguments
*          argv - array of arguments
* return:  0 if the arguments were parsed successfully,
*          -1 if there was an error
*
*** ---------------------------------------------------------------------- ***/

int parse_args(arguments *args, int argc, char **argv){

    args->n = DEFAULT_LINE_COUNT;   //default value for the number of lines to output
    args->input = NULL;

    switch (argc) {
        case 1:                     //no arguments
            args->input = stdin;    //read from stdin
            return 1;
        case 2:                     //one argument
            args->input = fopen(argv[1], "r");  //open the file
            if (args->input == NULL) {
                fprintf(stderr, "ERROR: File not found.\n");
                return -1;
            }
            return 1;
        case 3:
        case 4: {
            char *n_str;

            if (strcmp(argv[1], "-n") == 0) {
                n_str = argv[2];
            }
            else {
                fprintf(stderr, "ERROR: Unknown arguments.\n");
                return -1;
            }

            if (argc == 3) {
                args->input = stdin;
            } else {
                args->input = fopen(argv[3], "r");
            }

            args->n = string_to_int(n_str);
            if (args->n == -1) {
                fprintf(stderr, "ERROR: Invalid number of lines.\n");
                return -1;
            }
            break;
        }
        default:
            fprintf(stderr, "ERROR: Too many arguments.\n");
            return -1;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    arguments args;                                     //structure for storing the arguments

    char line_buf[MAX_LINE_LENGTH];                     //buffer for reading lines from the input

    if (parse_args(&args, argc, argv) == -1) {
        fprintf(stderr, "ERROR: Invalid arguments.\n");
        return 1;
    }

    circular_buffer *cb = cb_create(args.n);        //create the circular buffer

    while (fgets(line_buf, MAX_LINE_LENGTH, args.input) != NULL) {
        if (cb_put(cb, line_buf) == 1)
            continue;
        else
            return 1;
    }

    cb_last_n(cb, args.n);

    fclose(args.input);
    cb_free(cb);

    return 0;
}