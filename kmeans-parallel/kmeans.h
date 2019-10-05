/*****************************************************************************/
/******************************   Constants   ********************************/

#define CODELIST "code-list.xml"
#define COLLECTION "term-collection.xml"
#define COORDINATOR 0
#define TERM_SIZE 100
#define FILEPATH_SIZE 500
#define VALID_XML 2
#define CODE_FLAG 1
#define TERM_FLAG 2
#define TEXT_FLAG 3
#define PACK_FLAG 4

/*****************************************************************************/
/****************************   Include Files   ******************************/

#include <oompi.h>

/*****************************************************************************/
/**************************   Class Declarations   ***************************/

//This class represents an entry of the term code list
//and an array of objects with this type will represent
//the entire list
class CodeList : public OOMPI_User_type
{
public:
    //Members
    int code;
    int document_frequency;
    char term[TERM_SIZE];

    //Constructors
    CodeList();

    //Functions
    void set_term(char* _term);

private:
    // Static variable to hold the newly constructed
    // MPI_Datatype
    static OOMPI_Datatype type;
};

//This class represents an entry of the texts terms
//and an array of this object type will hold all the
//terms of a text file in the collection
class TextTermEntry : public OOMPI_User_type
{
public:
    //Members
    int code;
    int frequency;
    float weight;

    //Constructors
    TextTermEntry();
    TextTermEntry(const TextTermEntry& term);

private:
    // Static variable to hold the newly constructed
    // MPI_Datatype
    static OOMPI_Datatype type;
};

//This class represents the text file entity
//The text's terms array should be initialized
//after the object is sent to a node, to be able
//to recieve a packed message of it
class TextEntity : public OOMPI_User_type
{
public:
    //Members
    char file_path[FILEPATH_SIZE];
    int text_terms_size;
    TextTermEntry* text_terms;

    //Constructors
    TextEntity();
    TextEntity(const TextEntity& text);
    //Destructors
    ~TextEntity();

    //Functions
    void set_file_path(char* _file_path);
    void initialize_text_terms(void);
    void initialize_centroid_terms(int max_terms_size);
    void initialize_centroid_terms();
    void get_weight_vector(float* weight_vector, int vector_size);
    bool recalculate_terms_weight(float* new_weight_vector, int vector_size, int text_count);
    void delete_terms(void);

private:
    // Static variable to hold the newly constructed
    // MPI_Datatype
    static OOMPI_Datatype type;
};

/*****************************************************************************/
/************************   Function Declarations   **************************/

TextEntity* load_xml_collection(TextEntity* unsorted_collection);
void load_xml_codelist(void);
float calculate_similarity(TextEntity* text_1, TextEntity* text_2, CodeList* codes, int codes_size);
void copy_terms(TextEntity* text_1, TextEntity* text_2);
float float_max_value(float array[], int array_size);
int float_max_index(float array[], int array_size);
void summarize_text_weight(TextEntity* text_1, TextEntity* text_2);
void quicksort(TextEntity* text_collection_array, int left_index, int right_index);
void swap_texts(TextEntity* text_1, TextEntity* text_2);

/*****************************************************************************/
