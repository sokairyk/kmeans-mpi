/*****************************************************************************/
/******************************   Constants   ********************************/

#define STOPLIST "stoplist"
#define TERM_SIZE 100
#define MAX_TERM_SIZE 2000

/*****************************************************************************/
/***************************   Data Structures   *****************************/

//Struct used to implement the stop word list
struct stopTermStruct
{
    char term[TERM_SIZE];
    struct stopTermStruct *next;
};

typedef struct stopTermStruct sTS;
typedef sTS *stopTerm;

/*****************************************************************************/
/************************   Function Declarations   **************************/

stopTerm add_to_stop_list(char term[TERM_SIZE], stopTerm first);
stopTerm load_stop_list(void);
int stop(char term[TERM_SIZE], stopTerm first);
int remove_invalid_chars(char term[MAX_TERM_SIZE]);

/*****************************************************************************/
