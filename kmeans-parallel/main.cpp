/*****************************************************************************/
/*********************   Includes, Definitions & Usings  *********************/

#include "kmeans.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <oompi.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#ifdef XML_USE_NAMESPACE
using namespace XMLPP;
#else
#include "xml.h"
#endif

using namespace std;



/*****************************************************************************/
/************************   Class Implementations   **************************/

/*****************************************************************************/
/****************   Class CodeList constructor & functions   *****************/

//Static class member definitions
OOMPI_Datatype CodeList::type;

//Constructor
CodeList::CodeList() : OOMPI_User_type(type, this, CODE_FLAG)
{
    // Build the data type if it is not built already
    if (!type.Built())
    {
        type.Struct_start(this);
        type << code << document_frequency;
        for (int i = 0; i < TERM_SIZE; i++)
            type << term[i];
        type.Struct_end();
    }
}

//Functions
void CodeList::set_term(char* _term)
{
    strcpy(term, _term);
}

/*****************************************************************************/
/**************   Class TextTermEntry constructor & functions   **************/

//Static class member definitions
OOMPI_Datatype TextTermEntry::type;

//Constructor
TextTermEntry::TextTermEntry() : OOMPI_User_type(type, this, TERM_FLAG)
{
    // Build the data type if it is not built already
    if (!type.Built())
    {
        type.Struct_start(this);
        type << code << frequency << weight;
        type.Struct_end();
    }
}

//Copy constructor
TextTermEntry::TextTermEntry(const TextTermEntry& term) : OOMPI_User_type(type, this, TERM_FLAG)
{
    // Build the data type if it is not built already
    if (!type.Built())
    {
        type.Struct_start(this);
        type << code << frequency << weight;
        type.Struct_end();
    }

    //Copy the object attributes
    code = term.code;
    frequency = term.frequency;
    weight = term. weight;
}

/*****************************************************************************/
/****************   Class TextEntity constructor & functions   ***************/

//Static class member definitions
OOMPI_Datatype TextEntity::type;

//Constructor
TextEntity::TextEntity() : OOMPI_User_type(type, this, TEXT_FLAG)
{
    text_terms = NULL;

    // Build the data type if it is not built already
    if (!type.Built())
    {
        type.Struct_start(this);
        type << text_terms_size;
        for (int i = 0 ; i < FILEPATH_SIZE ; i++)
            type << file_path[i];
        type.Struct_end();
    }
}

//Copy constructor
TextEntity::TextEntity(const TextEntity& text) : OOMPI_User_type(type, this, TEXT_FLAG)
{
    // Build the data type if it is not built already
    if (!type.Built())
    {
        type.Struct_start(this);
        type << text_terms_size;
        for (int i = 0 ; i < FILEPATH_SIZE ; i++)
            type << file_path[i];
        type.Struct_end();
    }

    //Copy the object attributes
    strcpy(file_path, text.file_path);
    text_terms_size = text.text_terms_size;
    text_terms = new TextTermEntry[text_terms_size];

    for (int i = 0; i < text_terms_size; i++)
    {
        TextTermEntry* new_term = new TextTermEntry(text.text_terms[i]);
        text_terms[i] = *new_term;
    }
}

//Destructor
TextEntity::~TextEntity()
{
    text_terms = NULL;
}

//Functions
void TextEntity::set_file_path(char* _file_path)
{
    strcpy(file_path, _file_path);
}

void TextEntity::initialize_text_terms()
{
    text_terms = new TextTermEntry[text_terms_size];
}

void TextEntity::initialize_centroid_terms(int max_terms_size)
{
    delete [] text_terms;
    text_terms = NULL;
    text_terms_size = max_terms_size;
    text_terms = new TextTermEntry[max_terms_size];

    //Since the code collection list is sorted by code
    //initialize the code field and in set default values
    //to the rest attributes
    for (int i = 0; i < max_terms_size; i++)
    {
        text_terms[i].code = i + 1;
        text_terms[i].frequency = 0;
        text_terms[i].weight = 0.0;
    }
}

void TextEntity::initialize_centroid_terms()
{
    for (int i = 0; i < text_terms_size; i++)
    {
        text_terms[i].frequency = 0;
        text_terms[i].weight = 0.0;
    }
}

void TextEntity::get_weight_vector(float* weight_vector, int vector_size)
{
    int i = 0;
    int index = 0;

    //Since the codes in the code collection
    //are sorted and start from 1 we represent the codes
    //with i
    for (; i < vector_size; i++)
    {
        //If the code of the term is smaller than the index increase it
        while (text_terms[index].code < i + 1)
        {
            index++;

            //If the index exeeds the array bounds break the loop
            if (!(index < text_terms_size))
                break;
        }

        //If the index exeeded the array bounds break the loop
        if (!(index < text_terms_size))
            break;

        //If the code matches pass the weight to the array
        if (text_terms[index].code == i + 1)
            weight_vector[i] = text_terms[index].weight;
        else
            weight_vector[i] = 0.0;
    }

    //If the loop was broken because of text terms array bounds
    //continue the loop to the size of the code collection size
    //giving zero weight to the rest of the terms
    for (; i < vector_size; i++)
        weight_vector[i] = 0.0;
}

bool TextEntity::recalculate_terms_weight(float* new_weight_vector, int vector_size, int text_count)
{
    bool success = true;

    //Check that the vectors have the same size
    if (vector_size == text_terms_size)
    {
        //Set the new weight
        for (int i = 0; i < vector_size; i++)
            text_terms[i].weight = new_weight_vector[i];

        //Check if we have a text count > 0 to avoid division by 0
        if (!(text_count > 1))
            text_count = 1;

        //Divide the new weight (which is the summary of all the
        //sum of all the text files that belog to this centroid)
        //with the count of the files to calculate the average
        for (int i = 0; i < vector_size; i++)
            text_terms[i].weight /= (float)text_count;
    }
    else
        success = false;

    return success;
}

void TextEntity::delete_terms()
{
    delete [] text_terms;
}



/*****************************************************************************/
/**********************   Global variable declaration   **********************/

TextEntity* text_collection;        //Array to hold objects representing the text files (sorted)
int text_collection_size;           //Total number of text files in the collection
int local_collection_size;          //Number of text files each processor will handle

CodeList* code_collection;          //Array to hold the codes of the objects
int code_collection_size;           //Number of the codes that represent the terms

TextEntity* centroids;              //Centroids of the text files
int centroid_size;                  //Number of centroids

TextEntity* buffer_centroids;       //Buffer to accumulate the cetroid's vector weight sum
int* centroid_text_count;           //Count of the text files each centroid has
float* centroid_text_similarity;    //The similarity of a text file with a centroid
float mse, old_mse;                 //Curent mean squared error / Old mean squared error

float** text_weight_buffer;         //Buffer array for the get_weight_vector function of the centroid

int* text_count_reduce_buffer;      //Buffer for the AllReduce function on centroid_text_count
float** text_weight_reduce_buffer;  //Buffer array for the AllReduce function on the centroids weight vector
float mse_reduce_buffer;            //Buffer for the AllReduce function on mse

ostringstream output;               //Output for each processor

/*****************************************************************************/
/*********************************   Main   **********************************/

int main(int argc, char *argv[])
{
    //Initialize MPI
    OOMPI_COMM_WORLD.Init(argc, argv);
    //Get size and rank
    int rank = OOMPI_COMM_WORLD.Rank();
    int size = OOMPI_COMM_WORLD.Size();

    //Define these objects to build their types and use them for
    //type description in packed objects
    TextEntity text_pack_description;
    TextTermEntry text_term_pack_description;
    CodeList code_pack_description;

    //Load all the collection data from the XML file
    //and sort it
    if (rank == COORDINATOR)
    {
        TextEntity* unsorted_collection = NULL;

        unsorted_collection = load_xml_collection(unsorted_collection);
        cout << "Processor: " << rank << " loaded the text collection" << endl;
        cout << "Collection contains " << text_collection_size << " text files" << endl;

        //Sort the text files (based on their terms count) in the array
        //using the quicksort algorithm implementation for TextEntity types
        quicksort(unsorted_collection, 0, text_collection_size - 1);

        /* Now distribute the sorted array to the actual text_collection array
         *
         *  The distribution is done in a way like dealing cards:
         *
         *  e.g. If we have 7 text files and 3 processors the above sorted
         *       array will be copied to the global one accessing its index
         *       in the following order:
         *
         *       0, 2, 4, 1, 3, 5, 6
         *
         *       This is done because as it's described bellow the texts files
         *       will be distibuted the following way:
         *
         *       Processor: 0 -> Texts 0, 1
         *       Processor: 1 -> Texts 2, 3, 6
         *       Processor: 2 -> Texts 4, 5
         *
         *       That way the weight among the processors is more balanced
         *       and the parallelization timings are more accurate
         */

        //Initialize the array
        text_collection = new TextEntity[text_collection_size];

        //And the local variables
        int local_texts_size = text_collection_size / size;
        int index = text_collection_size - 1;

        //Copy the sorted data accesing the index as described above
        for (int j = 0; j < local_texts_size; j++)
            for (int i = 0; i < size; i++)
            {
                TextEntity* temp = new TextEntity(unsorted_collection[index]);

                text_collection[local_texts_size * i + j] = *temp;
                index--;
            }

        //Check for remainder additional files
        for (int i = text_collection_size - (text_collection_size % size); i < text_collection_size; i++)
        {
            text_collection[i] = unsorted_collection[index];
            index--;
        }
    }

    //Broadcast the text file collection size
    OOMPI_COMM_WORLD[COORDINATOR].Bcast(text_collection_size);

    //Ensure that the processors are less than the collection size
    if (text_collection_size < size)
    {
        if (rank == COORDINATOR)
        {
            cout << "The processors are more than the text files!" << endl;
            cout << "Exiting..." << endl;
        }

        OOMPI_COMM_WORLD.Finalize();
        return 0;
    }

    /* The general idea of distributing the files is to send to each
     * processor the same size of files from the integer division
     * and one file from the remainder of the division at the end
     *
     * e.g.
     * Processor 0 has: 1, 2, 3, 4, ...30 files
     * Processor size: 4
     * Integer division of files (30/4) equals 7 with remainder 2
     *
     * Processor 0 will handle files 1 to 7
     * Processor 1 will receive files 8 to 14
     * Processor 2 will receive files 15 to 21
     * Processor 3 will receive files 22 to 28
     *
     * The 2 files (29, 30) that were not send will be distributed
     * equaly to the worker processors.
     *
     * So Processor 1 will receive additionaly file 29 and Processor 2
     * will receive additionaly file 30
     */

    //Calculate the number of text files each processor will handle
    //If the text files do not divide among the processors equaly
    //then the worker processors will recieve one more text file
    if ((text_collection_size % size) != 0)
    {
        //If the processor's rank is less or equal to the remainder
        //then the processor will handle one extra text file
        if ((rank != COORDINATOR) && (rank <= text_collection_size % size))
            local_collection_size = (text_collection_size / size) + 1;
        else
            local_collection_size = text_collection_size / size;
    }
    else
        local_collection_size = text_collection_size / size;

    //Initialize the worker processors local collection object array
    if (rank != COORDINATOR)
        text_collection = new TextEntity[local_collection_size];

    cout << "Processor " << rank << " handles " << local_collection_size << " text files" << endl;

    if (rank == COORDINATOR)
    {
        //Prepare for each worker processor the appropriate
        //part of the text file collection
        for (int i = 1; i < size; i++)
        {
            //Since the local collection size might differ among processors we calculate it
            //seperately for each worker processor
            int pack_size = local_collection_size + (i <= text_collection_size % size ? 1 : 0);

            //Create a pack object which will hold the TextEntity objects for each processor
            //The size of the TextEntity object array of each processor is equal to
            //the local text file collection size
            OOMPI_Packed local_text_pack(OOMPI_COMM_WORLD.Pack_size(text_pack_description, pack_size), OOMPI_COMM_WORLD, PACK_FLAG);

            //Pack the TextEntity objects
            local_text_pack.Start();

            //Pack the standard local collection size
            for (int j = 0; j < local_collection_size; j++)
                local_text_pack.Pack(text_collection[(i * local_collection_size) + j]);

            //Now check if we should send an extra text file from the end of the array
            if (pack_size > local_collection_size)
                local_text_pack.Pack(text_collection[(local_collection_size * size) + i - 1]);

            local_text_pack.End();

            //And send the packed object to the appropriate worker processor
            OOMPI_COMM_WORLD[i].Send(local_text_pack);
        }

        //Output the text files the coordinator processor will handle
        for (int i = 0; i < local_collection_size; i++)
            cout << "Processor: " << rank << " handles " <<  text_collection[i].file_path << endl;
    }
    else
    {
        //Create a pack object which will hold the TextEntity objects for each processor
        //The size of the TextEntity object array of each processor is equal to
        //the local text file collection size
        OOMPI_Packed local_text_pack(OOMPI_COMM_WORLD.Pack_size(text_pack_description, local_collection_size), OOMPI_COMM_WORLD, PACK_FLAG);

        //Each worker processor will receive its pack of TextEntity array
        OOMPI_COMM_WORLD[COORDINATOR].Recv(local_text_pack);

        //Unpack the object and save it to the TextEntity object array
        local_text_pack.Start();
        for (int i = 0; i < local_collection_size; i++)
            local_text_pack.Unpack(text_collection[i]);
        local_text_pack.End();

        //Output the text files each worker processor will handle
        for (int i = 0; i < local_collection_size; i++)
            cout << "Processor: " << rank << " handles " <<  text_collection[i].file_path << endl;
    }

    //Now the worker processors must initialize their TextTermEntry arrays
    //inside of every TextEntity object in their file collection array
    if (rank != COORDINATOR)
        for (int i = 0; i < local_collection_size; i++)
            text_collection[i].initialize_text_terms();


    //The coordinator processor has the arrays of every object initialized during
    //the XML file load. The coordinator processor must now send the TextTermEntry
    //arrays to the appropriate worker processor
    if ( rank == COORDINATOR)
    {
        for (int i = 1; i < size; i++)
        {
            //Since the local collection size might differ among processors we calculate it
            //seperately for each worker processor
            int pack_size = local_collection_size + (i <= text_collection_size % size ? 1 : 0);

            for (int j = 0; j < pack_size; j++)
            {
                int text_collection_index;

                //Set the index inside the range that the worker processor will handle
                //or at the end of the collection to handle one extra text file
                if ((local_collection_size < pack_size ) && (j == local_collection_size))
                    text_collection_index = (local_collection_size * size) + i - 1;
                else
                    text_collection_index = (i * local_collection_size) + j;

                //Get the TextTermEntry array size to send
                int text_terms_size = text_collection[text_collection_index].text_terms_size;

                //Create a pack object to contain the TextTermEntry array of the
                //text TextEntity object that is handled
                OOMPI_Packed local_terms_pack(OOMPI_COMM_WORLD.Pack_size(text_term_pack_description, text_terms_size), OOMPI_COMM_WORLD, PACK_FLAG);

                //Pack the TextTermEntry array
                local_terms_pack.Start();
                for (int k = 0; k < text_terms_size; k++)
                    local_terms_pack.Pack(text_collection[text_collection_index].text_terms[k]);
                local_terms_pack.End();

                //Send it to the appropriate worker processor
                OOMPI_COMM_WORLD[i].Send(local_terms_pack);

                //Output the successful send
                cout << "Coordinator processor send to processor: " << i;
                cout << " " << text_terms_size << " terms";
                cout << " for file " << text_collection[text_collection_index].file_path << endl;
            }
        }
    }
    else
    {
        //Each worker processor
        for (int i = 0; i < local_collection_size; i++)
        {
            //Get the TextTermEntry array size to receive
            int text_terms_size = text_collection[i].text_terms_size;

            //Create a pack object to contain the TextTermEntry array of the
            //text TextEntity object that is handled
            OOMPI_Packed local_terms_pack(OOMPI_COMM_WORLD.Pack_size(text_term_pack_description, text_terms_size), OOMPI_COMM_WORLD, PACK_FLAG);

            //Receive the packed TextTermEntry array
            OOMPI_COMM_WORLD[COORDINATOR].Recv(local_terms_pack);

            //Unpack it
            local_terms_pack.Start();
            for (int j = 0; j < text_terms_size; j++)
                local_terms_pack.Unpack(text_collection[i].text_terms[j]);
            local_terms_pack.End();

            //Output the successful reveive
            cout << "Processor: " << rank << " received " << text_terms_size << " terms";
            cout << " for file: " << text_collection[i].file_path << endl;
        }
    }

    //Delete unused TextEntity terms from the coordinator processor to
    //free some memory
    if (rank == COORDINATOR)
        for (int i = local_collection_size; i < text_collection_size; i++)
            text_collection[i].delete_terms();

    //Now that each processor has its part of the processed files in the collection
    //the coordinator must load the codelist and send it to each processor

    //Load all the code data from the XML file
    if (rank == COORDINATOR)
    {
        load_xml_codelist();
        cout << "Processor: " << rank << " loaded the code list" << endl;
    }

    //Broadcast the code list size
    OOMPI_COMM_WORLD[COORDINATOR].Bcast(code_collection_size);

    //Initialize the worker processors local collection object array
    if (rank != COORDINATOR)
        code_collection = new CodeList[code_collection_size];

    //Create a pack object which will hold the CodeList objects for each processor
    OOMPI_Packed code_list_pack(OOMPI_COMM_WORLD.Pack_size(code_pack_description, code_collection_size), OOMPI_COMM_WORLD, PACK_FLAG);

    //The coordinator processor will pack the codelist array
    if (rank == COORDINATOR)
    {
        code_list_pack.Start();
        for (int i = 0; i < code_collection_size; i++)
            code_list_pack.Pack(code_collection[i]);
        code_list_pack.End();
    }

    //Broadcast the packed code list object array
    OOMPI_COMM_WORLD[COORDINATOR].Bcast(code_list_pack);

    //And have the worker processors unpack the code list
    if (rank != COORDINATOR)
    {
        code_list_pack.Start();
        for (int i = 0; i < code_collection_size; i++)
            code_list_pack.Unpack(code_collection[i]);
        code_list_pack.End();

        //Output the successful reveive
        cout << "Processor: " << rank << " recieved the code list" << endl;
    }

    //At this point every processor has all the data it needs:
    //  - Its part of the text collection
    //  - The code list
    //So now the centroid size is given by the user
    //and a random number of text files is selected as the initial
    //centroids

    //Initialize the centroids size
    if (rank == COORDINATOR)
    {
        //Initialize with an invalid value to run the while loop
        //at least once
        centroid_size = text_collection_size + 1;

        while (centroid_size > text_collection_size)
        {
            //TODO: Output the messages of each processor in order
            //      to avoid messages of other processors during cin
            sleep(1);
            cout << "Input the desired number of centroids to use: ";
            cin >> centroid_size;   //Read input from user

            if (centroid_size > text_collection_size)
                cout << "Centroid number shouldn't be more than the collection size!" << endl;
        }
    }

    //Broadcast the centroid size
    OOMPI_COMM_WORLD[COORDINATOR].Bcast(centroid_size);

    //Initialize the centroids
    centroids = new TextEntity[centroid_size];

    //Initialize the centroid terms
    //The terms size should be the maximum size possible
    //and that is the size of the code collection list
    //This is done so that the vectors can be summarized
    //into one later
    for (int i = 0; i < centroid_size; i++)
        centroids[i].initialize_centroid_terms(code_collection_size);

    /* The initial approach was to set the initial centroids
     * selecting random text files from the collection

    if (rank == COORDINATOR)
    {
        int random_index;       //Random index

        srand(time(NULL));      //Initialize random seed

        cout << "Selecting random centroids: ";

        for (int i = 0; i < centroid_size; i++)
        {
            //Get random number
            random_index = rand() % text_collection_size;

            if (i > 0)
                cout << ", ";
            cout << random_index;

            //Copy from a random TextEntity object, its terms to a centroid
            copy_terms(&centroids[i], &text_collection[random_index]);
        }
        cout << endl;
    }
    */

    //Create random centroids
    if (rank == COORDINATOR)
    {
        srand(time(NULL));          //Initialize random seed
        fstream random_seed_file;   //Set a file handler for static seed
        string line;                //Buffer for the random seed file

        cout << "Selecting random centroids..." << endl;

        ifstream random_seed("random");

        if (random_seed.is_open())
        {
            //If the file exists read initial values from file
            for (int i = 0; i < centroid_size; i++)
                for (int j = 0; j < centroids[i].text_terms_size; j++)
                {
                    getline (random_seed, line);
                    centroids[i].text_terms[j].weight = atof(line.c_str());
                }
            cout << "Loaded initial centroid values from file" << endl;
        }
        else
        {
            //else generate new values on each run with rand()
            for (int i = 0; i < centroid_size; i++)
                for (int j = 0; j < centroids[i].text_terms_size; j++)
                    centroids[i].text_terms[j].weight = (float)rand() / static_cast<float>(RAND_MAX);

            cout << "Generated random initial centroid values" << endl;
        }

        random_seed.close();
    }

    //Create a pack object which will hold one centroid's terms
    OOMPI_Packed centroid_term_pack(OOMPI_COMM_WORLD.Pack_size(text_term_pack_description, code_collection_size), OOMPI_COMM_WORLD, PACK_FLAG);

    //The coordinator processor will pack all the terms of the selected centroids
    for (int i = 0; i < centroid_size; i++)
    {
        if (rank == COORDINATOR)
        {
            centroid_term_pack.Start();
            for (int j = 0; j < centroids[i].text_terms_size; j++)
                centroid_term_pack.Pack(centroids[i].text_terms[j]);
            centroid_term_pack.End();
        }

        //Broadcast the centroid terms
        OOMPI_COMM_WORLD[COORDINATOR].Bcast(centroid_term_pack);

        //And have the worker processors unpack the centroid terms
        if (rank != COORDINATOR)
        {
            centroid_term_pack.Start();
            for (int j = 0; j < centroids[i].text_terms_size; j++)
                centroid_term_pack.Unpack(centroids[i].text_terms[j]);
            centroid_term_pack.End();

            //Output the successful reveive
            cout << "Processor: " << rank << " recieved centroid #" << i << endl;
        }
    }

    //The coordinator processor should delete the TextEntity objects that distributed
    //(since they are now hanlded by worker processors) in order to free some memory
    if (rank == COORDINATOR)
        for (int i = local_collection_size; i < text_collection_size; i++)
            text_collection[i].~TextEntity();

    //Delay to output all messages
    sleep(1);

    //************************************************//
    //************* INITIALIZE VARIABLES *************//
    //************************************************//

    //Create the buffer centroids
    buffer_centroids = new TextEntity[centroid_size];

    //And create their terms
    for (int i = 0; i < centroid_size; i++)
        buffer_centroids[i].initialize_centroid_terms(code_collection_size);

    //Create the text count array to hold the number
    //of texts that belong to each text file
    centroid_text_count = new int[centroid_size];

    //Create the text similarity array to hold the
    //similarity value of a text file with every centroid
    centroid_text_similarity = new float[centroid_size];

    //Allocate 2 arrays of pointers
    //Each element will point to a float array that will
    //have the size of the code collection
    //All vectors should have the same size for the calculations
    text_weight_buffer = new float*[centroid_size];
    text_weight_reduce_buffer = new float*[centroid_size];
    //text_weight_buffer = (float **)malloc(centroid_size * sizeof(float *));
    //text_weight_reduce_buffer = (float **)malloc(centroid_size * sizeof(float *));

    for (int i = 0; i < centroid_size; i++)
    {
        //Create new arrays
        float* new_text_weight_buffer =  new float[code_collection_size];
        float* new_text_weight_reduce_buffer =  new float[code_collection_size];
        //And assign their pointer to the main arrays
        text_weight_buffer[i] = new_text_weight_buffer;
        text_weight_reduce_buffer[i] = new_text_weight_reduce_buffer;
    }

    text_count_reduce_buffer = new int[centroid_size];

    //Begin with an invalid squared error to run the loop at
    //least once
    mse = -1;

    //Set the start time for each processor
    double time_start = OOMPI_ENV.Wtime();

    //************************************************//
    //************** K-MEANS LOOP START **************//
    //************************************************//

    //Now the k-means core algorithm will start
    do
    {

        //Initialize variables on every loop
        for (int i = 0; i < centroid_size; i++)
        {
            buffer_centroids[i].initialize_centroid_terms();
            centroid_text_count[i] = 0;
        }

        old_mse = mse;
        mse = 0.0;

        //For each text file
        for (int i = 0; i < local_collection_size; i++)
        {
            //Calculate the similarity with every cetroid
            for (int j = 0; j < centroid_size; j++)
                centroid_text_similarity[j] = calculate_similarity(&centroids[j], &text_collection[i], code_collection, code_collection_size);

            //Add the weights of the text terms vector of the most similar text file
            //to the corresponding weigths of the centroid's text terms vector
            summarize_text_weight(&buffer_centroids[float_max_index(centroid_text_similarity, centroid_size)], &text_collection[i]);

            //And increase that centroid's text file count
            centroid_text_count[float_max_index(centroid_text_similarity, centroid_size)]++;

            //Update the mean squared error
            mse += float_max_value(centroid_text_similarity, centroid_size);

            //Log the centroid this text file belongs
            //output << "File " << text_collection[i].file_path << " set to centroid: ";
            //output << float_max_index(centroid_text_similarity, centroid_size) << endl;
        }

        //Summarize the text file count of the centroids on all processors
        OOMPI_COMM_WORLD.Allreduce(centroid_text_count, centroid_size, text_count_reduce_buffer, OOMPI_SUM);

        //For each centroid
        for (int i = 0; i < centroid_size; i++)
        {
            //Get the weight list of the centroid
            buffer_centroids[i].get_weight_vector(text_weight_buffer[i], code_collection_size);

            //Summarize the weight list of all processors
            OOMPI_COMM_WORLD.Allreduce(text_weight_buffer[i], code_collection_size, text_weight_reduce_buffer[i], OOMPI_SUM);

            centroids[i].recalculate_terms_weight(text_weight_reduce_buffer[i], code_collection_size, text_count_reduce_buffer[i]);
        }

        OOMPI_COMM_WORLD.Allreduce(mse, mse_reduce_buffer, OOMPI_SUM);

        mse = mse_reduce_buffer;

        if (rank == COORDINATOR)
            cout << "MSE: " << mse_reduce_buffer << " Old MSE: " << old_mse << endl;
    }
    while (mse > old_mse);

    //Output the time elapsed for each processor
    double seconds;

    if (rank == COORDINATOR)
    {
        cout << "Processor: "<< rank << " | Time elapsed: " << OOMPI_ENV.Wtime() - time_start << endl;

        for (int i = 1; i < size; i++)
        {
            OOMPI_COMM_WORLD[i].Recv(seconds);
            cout << "Processor: "<< i << " | Time elapsed: " << seconds << endl;
        }

    }
    else
    {
        seconds = OOMPI_ENV.Wtime() - time_start;

        OOMPI_COMM_WORLD[COORDINATOR].Send(seconds);
    }

    //************************************************//
    //*************** K-MEANS LOOP END ***************//
    //************************************************//

    OOMPI_COMM_WORLD.Finalize();

    return 0;
}

/*****************************************************************************/
/*******************************   Functions   *******************************/

/*****************************************************************************/
/********   Load the text collection from the XML file to an array   *********/

TextEntity* load_xml_collection(TextEntity* unsorted_collection)
{
    XML* xml_file;                      //Object to hold the XML data of the file

    //Load the file
    xml_file = new XML(COLLECTION);

    //If the XML data is valid
    if ((xml_file->ParseStatus() < VALID_XML ) && (xml_file->IntegrityTest()))
    {
        xml_file->CompressMemory();

        //Get the root element and get the number of it's child nodes
        XMLElement* xml_root = xml_file->GetRootElement();
        text_collection_size = xml_root->GetChildrenNum();

        //Allocate memory for the TextEntity object array that will hold
        //the text file collection
        unsorted_collection = new TextEntity[text_collection_size];

        //For each node that represents a text file
        for (int i = 0 ; i < text_collection_size ; i++)
        {
            //This is the buffer for reading the path attribute of the node
            char file_path[FILEPATH_SIZE];

            //Get the immediate nth child of the node we are
            XMLElement* text_node = xml_root->GetChildren()[i];

            //Read the path attribute
            text_node->FindVariableZ("path")->GetValue(file_path);

            //Set it to the text collection object
            unsorted_collection[i].set_file_path(file_path);

            //Get the number of the text file's term nodes
            unsorted_collection[i].text_terms_size = text_node->GetChildrenNum();

            //Initialize the TextEntity object's TextTermEntry array
            unsorted_collection[i].initialize_text_terms();

            //Load the data of each term node to the TextTermEntry array
            //of the TextEntity object
            for (int j = 0; j < unsorted_collection[i].text_terms_size ; j++)
            {
                //Get the immediate nth child of the node we are
                XMLElement* text_term = text_node->GetChildren()[j];

                //Fill the text's term object values
                unsorted_collection[i].text_terms[j].code = text_term->FindVariableZ("code")->GetValueInt();
                unsorted_collection[i].text_terms[j].frequency = text_term->FindVariableZ("frequency")->GetValueInt();
                unsorted_collection[i].text_terms[j].weight = text_term->FindVariableZ("weight")->GetValueFloat();
            }
        }
    }

    //Delete the XML object
    delete xml_file;

    return unsorted_collection;
}

/*****************************************************************************/
/***********   Load the code list from the XML file to an array   ************/

void load_xml_codelist(void)
{
    XML* xml_file;                      //Object to hold the XML data of the file

    //Load the file
    xml_file = new XML(CODELIST);

    //If the XML data is valid
    if ((xml_file->ParseStatus() < VALID_XML ) && (xml_file->IntegrityTest()))
    {
        xml_file->CompressMemory();

        //Get the root element and get the number of it's child nodes
        XMLElement* xml_root = xml_file->GetRootElement();
        code_collection_size = xml_root->GetChildrenNum();

        //Allocate memory for the TextEntity object array that will hold
        //the text file collection
        code_collection = new CodeList[code_collection_size];

        //For each node that represents a code
        for (int i = 0 ; i < code_collection_size; i++)
        {
            //This is the buffer for reading the term of the node
            char term[TERM_SIZE];

            //Get the immediate nth child of the node we are
            XMLElement* code_node = xml_root->GetChildren()[i];

            //Fill the text's term object values

            //Read the term attribute
            code_node->FindVariableZ("value")->GetValue(term);

            //Set it to the code collection object
            code_collection[i].set_term(term);
            //Fille the rest values
            code_collection[i].code = code_node->FindVariableZ("id")->GetValueInt();
            code_collection[i].document_frequency = code_node->FindVariableZ("df")->GetValueInt();
        }
    }

    //Delete the XML object
    delete xml_file;
}

/*****************************************************************************/
/******************   Copy the terms of text 2 to text 1   *******************/

void copy_terms(TextEntity* text_1, TextEntity* text_2)
{
    int index_2 = 0;                //Index for text 2

    //If the text 2 has terms
    if (text_2->text_terms_size > 0)
    {
        //Run through the terms of text_1
        for (int i = 0; i < text_1->text_terms_size; i++)
        {
            //Since the texts have their terms sorted by code run through text_2 and
            //stop at the code we loop
            while (text_2->text_terms[index_2].code < text_1->text_terms[i].code)
            {
                index_2++;
                //If the index exeeds the array bounds break the loop
                if (!(index_2 < text_2->text_terms_size))
                    break;
            }

            //If the index exeeded the array bounds break the loop
            if (!(index_2 < text_2->text_terms_size))
                break;

            //If the codes of the 2 TextEntity objects match
            if (text_1->text_terms[i].code == text_2->text_terms[index_2].code)
            {
                //Copy the attributes of text 2 to text 1
                text_1->text_terms[i].frequency = text_2->text_terms[index_2].frequency;
                text_1->text_terms[i].weight = text_2->text_terms[index_2].weight;
            }
        }
    }
}

/*****************************************************************************/
/************   Calculate the similarity between two text files   ************/

float calculate_similarity(TextEntity* text_1, TextEntity* text_2, CodeList* codes, int codes_size)
{
    int index_1 = 0;                //Index for text 1
    int index_2 = 0;                //Index for text 2

    float text_1_sum = 0.0;         //Sum of the power of 2 of the text 1 terms weight
    float text_2_sum = 0.0;         //Sum of the power of 2 of the text 2 terms weight
    float texts_multiplied = 0.0;   //Sum of the multiplication of the weight of the texts
    float similarity = 0.0;         //Similarity of the texts

    //If both text files have at least one term run through the term list
    if ((text_1->text_terms_size > 0) && (text_2->text_terms_size > 0) && (codes_size > 0))
    {
        for (int i = 0; i < codes_size; i++)
        {
            //Since the texts have their terms sorted by code run through both and
            //stop at the code we loop
            while (text_1->text_terms[index_1].code < codes[i].code)
            {
                index_1++;
                //If the index exeeds the array bounds break the loop
                if (!(index_1 < text_1->text_terms_size))
                    break;
            }
            while (text_2->text_terms[index_2].code < codes[i].code)
            {
                index_2++;
                //If the index exeeds the array bounds break the loop
                if (!(index_2 < text_2->text_terms_size))
                    break;
            }

            //If the index of either text file exeeded the term count of its file break the loop
            if (!((index_1 < text_1->text_terms_size) && (index_2 < text_2->text_terms_size)))
                break;

            //If both texts stoped at the same term
            if (text_1->text_terms[index_1].code == text_2->text_terms[index_2].code)
                texts_multiplied += text_1->text_terms[index_1].weight * text_2->text_terms[index_2].weight;
        }

        for (int i = 0; i < text_1->text_terms_size; i++)
            text_1_sum += pow(text_1->text_terms[i].weight, 2.0);

        for (int i = 0; i < text_2->text_terms_size; i++)
            text_2_sum += pow(text_2->text_terms[i].weight, 2.0);

        //Calculate similarity
        if ((text_1_sum == 0.0) || (text_2_sum == 0.0))
            similarity = 0.0;
        else
            similarity = texts_multiplied / (sqrt(text_1_sum) * sqrt(text_2_sum));
    }

    return similarity;
}

/*****************************************************************************/
/************   Add the weight of text 2 to the weight of text 1   ***********/

void summarize_text_weight(TextEntity* text_1, TextEntity* text_2)
{
    int index_2 = 0;                //Index for text 2

    //If the text 2 has terms
    if (text_2->text_terms_size > 0)
    {
        //Run through the terms of text_1
        for (int i = 0; i < text_1->text_terms_size; i++)
        {
            //Since the texts have their terms sorted by code run through text_2 and
            //stop at the code we loop
            while (text_2->text_terms[index_2].code < text_1->text_terms[i].code)
            {
                index_2++;
                //If the index exeeds the array bounds break the loop
                if (!(index_2 < text_2->text_terms_size))
                    break;
            }

            //If the index exeeded the array bounds break the loop
            if (!(index_2 < text_2->text_terms_size))
                break;

            //If the codes of the 2 TextEntity objects match
            if (text_1->text_terms[i].code == text_2->text_terms[index_2].code)
            {
                //Add the weight of text 2 term to text 1 term
                text_1->text_terms[i].weight += text_2->text_terms[index_2].weight;
            }
        }
    }
}

/*****************************************************************************/
/*****************   Get the maximum value of a float array   ****************/

float float_max_value(float array[], int array_size)
{
    assert(array_size > 0);
    float max_val = array[0];

    for (int i = 1; i < array_size; i++)
        if (array[i] > max_val)
            max_val = array[i];

    return max_val;
}

/*****************************************************************************/
/***********   Get the index of the maximum value in a float array   *********/

int float_max_index(float array[], int array_size)
{
    assert(array_size > 0);
    int max_index = 0;

    for (int i = 1; i < array_size; i++)
        if (array[i] > array[max_index])
            max_index = i;

    return max_index;
}

/*****************************************************************************/
/* Quicksort function for sorting the text collection based on the term size */

void quicksort(TextEntity* text_collection_array, int left_bound, int right_bound)
{

    int terms_size_key, pivot;
    int left_index, right_index;   //Temp indexes to run through the divided parts of the array

    if ( left_bound < right_bound)
    {
        //Set the pivot
        pivot = (left_bound + right_bound) / 2;

        //Swap with the pivot index
        swap_texts(&text_collection_array[left_bound], &text_collection_array[pivot]);
        //Set the key (term size) to base the sorting
        terms_size_key = text_collection_array[left_bound].text_terms_size;

        //Set the temp indexes
        left_index = left_bound + 1;
        right_index = right_bound;

        //Move the temp indexes to each others direction
        while (left_index <= right_index)
        {
            while ((left_index <= right_bound) && (text_collection_array[left_index].text_terms_size <= terms_size_key))
                left_index++;

            while ((right_index >= left_bound) && (text_collection_array[right_index].text_terms_size > terms_size_key))
                right_index--;
            //Swap if necessary
            if (left_index < right_index)
                swap_texts(&text_collection_array[left_index], &text_collection_array[right_index]);
        }

        //Swap with left bound
        swap_texts(&text_collection_array[left_bound], &text_collection_array[right_index]);

        //Recursively sort the lesser array
        quicksort(text_collection_array, left_bound, right_index - 1);
        quicksort(text_collection_array, right_index + 1, right_bound);
    }
}

/*****************************************************************************/
/***************   Swap the pointers of a 2 TextEntity objects   *************/

void swap_texts(TextEntity* text_1, TextEntity* text_2)
{
    TextEntity* temp = new TextEntity(*text_1);

    *text_1 = *text_2;
    *text_2 = *temp;
}

/*****************************************************************************/
