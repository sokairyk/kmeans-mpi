/*****************************************************************************/
/******************************   Constants   ********************************/

#define FILENAMES "filenames"
#define TERM_SIZE 100
#define FILEPATH_SIZE 500
#define MAX_TERM_SIZE 2000

/*****************************************************************************/
/***************************   Data Structures   *****************************/

//Struct for the vector space model of the document terms
struct termVectorStruct
{
    int code;                       //Term code
    int TF;                         //Term frequency
    double weight;                  //Term weight
    struct termVectorStruct *next;  //Pointer to next term
};

typedef struct termVectorStruct tVS;
typedef tVS *termVector;

//Struct for the matching of terms with codes
struct termVectorCodeStruct
{
    char term[TERM_SIZE];               //Term
    int code;                           //Term code
    int  DF;                            //Document frequency
    struct termVectorCodeStruct *next;  //Next element of the code list
};

typedef struct termVectorCodeStruct tVCS;
typedef tVCS    *termVectorCode;

//Struct for the array of texts
struct textsStruct
{
    char text_name[FILEPATH_SIZE];      //Text name including path
    termVector text_terms;              //Pointer to the first element of the text terms
};

typedef struct textsStruct tS;
typedef tS *texts;

/*****************************************************************************/
/************************   Function Declarations   **************************/

void preprocesing(void);
int get_collection_size(void);
void create_term_vectors(void);
termVector get_terms(char filepath[FILEPATH_SIZE]);
int add_to_term_vector_code_list(char term[TERM_SIZE]);
termVector add_to_term_vector_list(int code, termVector first_term);
void document_frequency_calculation(void);
void weight_calculation(void);

/*****************************************************************************/
