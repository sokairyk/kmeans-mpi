/*****************************************************************************/
/****************************   Include Files   ******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "stem.h"
#include "stop.h"
#include "preprocess.h"

/*****************************************************************************/
/*********************   External variable declaration   *********************/

extern texts text_collection;       //Array containing all the text filenames with
                                    //paths and the first element of their term list

extern int collection_size;         //Number of text files in the collection

extern termVectorCode code_list;    //The list containing the codes of the terms of the collection

/*****************************************************************************/
/*******************************   Functions   *******************************/


/*****************************************************************************/
/*****************************   Preprocessing   *****************************/

void preprocesing()
{
    collection_size = get_collection_size();                        //Get the size of the collection
    text_collection = (texts)malloc(collection_size * sizeof(tS));  //and allocate dynamically the
                                                                    //appropriate memory for the array
    //Memory allocation check
    if (!text_collection)
    {
        printf("\n\nError!\nUnable to allocate memory!\nExiting...");
        exit;
    }

    //Extract the document's terms process them and create the term vectors of the collection
    create_term_vectors();
    //Using the code list of the collection terms calculate their global frequency
    document_frequency_calculation();
    //Calculate the weight of each document's term according to the values of TF and DF
    weight_calculation();
}

/*****************************************************************************/
/*****************   Get the size of the texts collection   ******************/

int get_collection_size()
{
    FILE *fp = NULL;            //File containing the filenames with full paths
    char buffer[FILEPATH_SIZE]; //Full path of the file entry
    int counter = 0;            //Number of text files in the collection

    printf("Counting files entries in file %s...\t", FILENAMES);

    //File check
    if ((fp = fopen(FILENAMES, "r")) == NULL)
    {
        printf("\n\nError!\nCould not open file: %s\nExiting...\n", FILENAMES);
        exit;
    }

    while (fscanf(fp, "%s", buffer) != EOF)    //Read every text path entry in the
        counter++;                             //file and increase the counter

    printf("%d files found!\n\n", counter);

    fclose(fp);
    return(counter);    //Return the size of the collection
}

/*****************************************************************************/
/**********   Creation of the the array with all the term vectors   **********/

void create_term_vectors()
{
    FILE *fp = NULL;                //File containing the filenames with full paths
    char filepath[FILEPATH_SIZE];   //Full path of the file
    int index;                      //Index of the array text_collection

    //File check
    if ((fp = fopen(FILENAMES, "r")) == NULL)
    {
        printf("\n\nError!\nCould not open file: %s\nExiting...\n", FILENAMES);
        exit;
    }

    //Initialize array
    for (index = 0; index < collection_size; index++)
        text_collection[index].text_terms = NULL;
    index = 0;

    while (fscanf(fp, "%s", filepath) != EOF)                       //Read filename and path
    {
        printf("Processing file: %s...\t", filepath);
        strcpy(text_collection[index].text_name, filepath);         //Put full path of file to the first
                                                                    //member of struct text_collection
        text_collection[index].text_terms = get_terms(filepath);    //Put the first term of the file
                                                                    //to the second memeber of struct
                                                                    //variable text_collection
        index++;
        printf("OK!\n");
    }

    fclose(fp);
}

/*****************************************************************************/
/*******************   Text to Term Vector Transformation   ******************/

termVector get_terms(char filepath[FILEPATH_SIZE])
{
    FILE *fp = NULL;                //Text file from the collection to open
    char term[MAX_TERM_SIZE];       //Buffer for the reading of terms
    int code;                       //Code of the term
    termVector first_term = NULL;   //First element of the text file terms
    stopTerm stop_list = NULL;      //First element of the stop list

    //File check
    if ((fp = fopen(filepath,"r")) == NULL)
    {
        printf("\n\nError!\nCould not open file: %s\nExiting...\n", filepath);
        exit;
    }

    stop_list = load_stop_list();           //Load the stop word list

    while (fscanf(fp, "%s", &term) != EOF)  //Read every term
    {
        if (remove_invalid_chars(term))                 //Remove any invalid characters
            if (stop(term, stop_list))                  //Check if the term is in the stop word list
                if (Stem(term) && (strlen(term) > 0))   //Stem the term
                {
                    //Assign a code to the term
                    code = add_to_term_vector_code_list(term);
                    //Add the term to the term list
                    first_term = add_to_term_vector_list(code, first_term);
                }
    }

    fclose(fp);
    return (first_term);
}

/*****************************************************************************/
/***********   Import a term to the code list and assign a code   ************/

int add_to_term_vector_code_list(char term[TERM_SIZE])
{
    termVectorCode tmp_rec, new_rec;    //tmp_rec is used to run through the list
                                        //and new_rec is the new term to assign a code

    int flag = 0;                       //This flag is used to check whether the term has
                                        //already a code assigned
                                        //flag = 0 The term doesn't have a code assigned
                                        //flag = 1 The term has a code assigned

    int code;                           //This variable holds the code of the term that is passed
                                        //to the function

    if (code_list == NULL)  //Check if the list is empty and add the first entry of the list
    {
        new_rec = (termVectorCode)malloc(sizeof(tVCS)); //Allocate a new termVectorCode

        //Memory allocation check
        if (!new_rec)
        {
            printf("\n\nError!\nUnable to allocate memory!\nExiting...");
            exit;
        }

        //Fill the newly created element's fields
        new_rec->code=code = 1;
        strcpy(new_rec->term, term);
        new_rec->DF = 0;
        new_rec->next = NULL;
        code_list = new_rec;
    }
    else                            //If the list has entries then run
    {
                                    //through it and check if the term
        tmp_rec = code_list;        //already exists
        while (tmp_rec->next != NULL)
        {
            if (strcasecmp(term, tmp_rec->term) == 0)   //If there is a code assigned break
                break;                                  //the loop
            else
                tmp_rec=tmp_rec->next;                  //else check the next entry
        }

        if (strcasecmp(term, tmp_rec->term) == 0)       //This line updates the flag if the
            flag = 1;                                   //loop is broken. If it's not it's a
                                                        //check for the last element of the
                                                        //list

        code = tmp_rec->code;   //Set the element's code to the one
                                //where the loop stoped

        if (!flag)              //If the term has no code assigned
        {
            //Allocate a new termVectorCode
            new_rec = (termVectorCode)malloc(sizeof(tVCS));

            //Memory allocation check
            if (!new_rec)
            {
                printf("\n\nError!\nUnable to allocate memory!\nExiting...");
                exit;
            }

            //Fill the newly created element's fields and update the code
            strcpy(new_rec->term, term);
            new_rec->code = code = tmp_rec->code + 1;
            new_rec->DF = 0;
            new_rec->next = NULL;       //And add it to the end of the list
            tmp_rec->next = new_rec;    //since tmp_rec is pointing at the
        }                               //last entry
    }
    return(code);
}

/*****************************************************************************/
/**************   Import a term to the document's term vector   **************/

termVector add_to_term_vector_list(int code, termVector first_term)
{
    //tmp_rec is used to run through the list, new_rec is the new term to add to the
    //document's term vector and prv_rec holds the previous term while running
    //through the list to find the correct place to insert the new term in the term
    //vector so that the term vector is sorted by the code of the terms

    termVector tmp_rec, new_rec, prv_rec;

    int flag = 0;       //This flag is used to check whether the term
                        //exists in the term vector list
                        //flag = 0 The term doesn't exist
                        //flag = 1 The term exists and the TF is increased

    if (first_term == NULL)     //If the text's term list is empty
    {
        new_rec = (termVector)malloc(sizeof(tVS));  //Allocate a new termVector

        //Memory allocation check
        if (!new_rec)
        {
            printf("\n\nError!\nUnable to allocate memory!\nExiting...");
            exit;
        }

        //Fill the newly created element's fields and set it as the first list element
        new_rec->code = code;
        new_rec->TF = 1;
        new_rec->next = NULL;
        first_term = new_rec;
    }
    else                        //If the list has entries then run
    {
                                //through it and check if the term
        tmp_rec = first_term;   //already exists
        while (tmp_rec->next != NULL)
        {
            if (tmp_rec->code == code)      //If the term is in the list break
                break;                      //the loop
            else
                tmp_rec = tmp_rec->next;    //else check the next entry
        }

        if (tmp_rec->code == code)  //This line updates the flag if the
            flag = 1;               //loop is broken. If it's not it's a
                                    //check for the last element of the
                                    //list

        if (!flag)      //If the term is not in the list
        {
            new_rec = (termVector)malloc(sizeof(tVS));  //Allocate a new termVector

            //Memory allocation check
            if (!new_rec)
            {
                printf("\n\nError!\nUnable to allocate memory!\nExiting...");
                exit;
            }

            //Fill the newly created element's fields
            new_rec->code = code;
            new_rec->TF = 1;

            //And find the correct place to insert the new term
            tmp_rec = first_term;   //Set tmp_rec to the start of the term vector
            prv_rec = NULL;         //Set prv_rec as NULL cause there isn't any
                                    //previous term before the first

            while (tmp_rec != NULL) //Run through the term list
            {
                if (tmp_rec->code > new_rec->code)  //If the code of the new term
                {
                                                    //is smaller than the current
                                                    //then it must be inserted before

                    if (prv_rec == NULL)            //If the previous term is NULL
                    {
                                                    //this means that we're at the first
                        first_term = new_rec;       //term of the list so the new term
                        new_rec->next = tmp_rec;    //is changed to be the first element

                    }
                    else
                    {
                                                    //else the new term is entered
                        prv_rec->next = new_rec;    //before the current (tmp_rec)
                        new_rec->next = tmp_rec;    //and after the previous (prv_rec)
                    }

                    break;  //Break the loop
                }
                else                               //else if the current term's code
                {
                                                    //is smaller than the new term's
                    prv_rec = tmp_rec;              //set the previous term for the next loop
                    tmp_rec = tmp_rec->next;        //and check the next term
                }
            }

            if (tmp_rec == NULL)            //If tmp_rec is null then it means
            {
                                            //that we're at the end of the list
                new_rec->next = NULL;       //So we insert the new term at the end
                prv_rec->next = new_rec;    //set prv_rec to poin at the new_rec
            }
        }
        else
        {
                                //else if the term exists
            tmp_rec->TF++;      //update the term frequency
        }
    }
    return(first_term);
}

/*****************************************************************************/
/**********  Calculate the frequency of the terms in the collection  *********/

void document_frequency_calculation()
{
    int index;                  //Index of the array text_collection
    termVectorCode tmp_code;    //tmp_code is used to run through the code list
    termVector tmp_term;        //tmp_term is used to run through each
                                //document's term vector list

    tmp_code = code_list;   //Set the code to the first element of the
                            //code list

    printf("\nCalculating term frequency in the collection...\t");

    while (tmp_code != NULL)    //Run through the code list
    {
        for (index = 0; index < collection_size; index++)   //For each document
        {
            //Set the tmp_term to the first element of
            //the document's term vector list
            tmp_term = text_collection[index].text_terms;

            while (tmp_term != NULL)    //Run through its term list
            {
                                        //and check if the term exists
                if (tmp_code->code == tmp_term->code)
                {
                                                //If the term exists
                    tmp_code->DF++;             //increase the document frequency
                    break;                      //and break the loop
                }
                else
                    tmp_term = tmp_term->next;  //else check the next term
            }
        }
        tmp_code = tmp_code->next;              //Next code
    }

    printf("OK!\n");
}

/*****************************************************************************/
/***********  Calculate the term weight for each document's terms  ***********/

void weight_calculation()
{
    int index;                  //Index of the array text_collection
    termVectorCode tmp_code;    //tmp_code is used to run through the code list
    termVector tmp_term;        //tmp_term is used to run through each
                                //document's term vector list

    printf("Calculating term weight for each document...\t");

    for (index = 0; index < collection_size; index++)   //For each document
    {
        //Set the tmp_term to the first element of
        //the document's term vector list
        tmp_term = text_collection[index].text_terms;

        while (tmp_term != NULL)    //Run through its term list
        {
            tmp_code = code_list;
            while (tmp_code != NULL)                    //Find the associated element
                if (tmp_code->code == tmp_term->code)   //from the code list
                    break;                              //and break the loop
                else
                    tmp_code = tmp_code->next;

            //When the code entry is found
            //calculate the weight of the term
            tmp_term->weight = ((double)tmp_term->TF) * log10((double)collection_size / (double)tmp_code->DF);

            //And do the same for the next term
            tmp_term = tmp_term->next;
        }
    }

    printf("OK!\n\n");
}

/*****************************************************************************/
