/*****************************************************************************/
/****************************   Include Files   ******************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "stop.h"

/*****************************************************************************/
/*******************************   Functions   *******************************/


/*****************************************************************************/
/***************   Adding a stop word to the stop term list   ****************/

stopTerm add_to_stop_list(char term[TERM_SIZE], stopTerm first_rec)
{
    stopTerm new_rec = NULL;        //The new term to be inserted
    stopTerm tmp_rec = NULL;        //tmp_rec is used to run through the list

    new_rec = (stopTerm)malloc(sizeof(sTS));    //Allocate memory for the new term

    //Memory allocation check
    if (!new_rec)
    {
        printf("\n\nError!\nUnable to allocate memory!\nExiting...");
        exit;
    }

    strcpy(new_rec->term, term);    //Fill the new element's fields
    new_rec->next = NULL;

    if (first_rec == NULL)          //Check if the list if empty
        first_rec = new_rec;        //and add the first entry
    else
    {
        tmp_rec = first_rec;            //Else run through the list until
        while (tmp_rec->next != NULL)   //tmp_rec points at the last element
            tmp_rec = tmp_rec->next;

        tmp_rec->next = new_rec;        //And enter the new stop word at the
    }                                   //of the list

    return(first_rec);  //Return the first element of the list
}

/*****************************************************************************/
/***********************   Load the stop word list   *************************/

stopTerm load_stop_list(void)
{
    FILE *fp = NULL;            //File containing the stop words
    stopTerm stop_list = NULL;  //The first element of the stop word list
    char term[TERM_SIZE];             //Buffer for reading the stop words from
                                //the file

    //File Check
    if ((fp = fopen(STOPLIST, "r")) == NULL)
    {
        printf("\n\nError!\nCould not open the file: %s\nExiting...", STOPLIST);
        exit;
    }

    while (fscanf(fp, "%s", term) != EOF)               //Read every term of the
        stop_list = add_to_stop_list(term, stop_list);  //file and add it to the
                                                        //stop word list
    fclose(fp);

    return(stop_list);  //Return the first element of the stop word list
}

/*****************************************************************************/
/*****************   Compare a term to the stop word list   ******************/

int stop(char term[TERM_SIZE], stopTerm first_rec)
{
    stopTerm tmp_rec;   //tmp_rec is used to run through the list
    int flag = 1;       //flag is used to determine whether the term
                        //is in the stop word list
                        //
                        //flag = 0 The term was found in the stop list
                        //flag = 1 The term was not found in the stop list

    tmp_rec = first_rec;

    while (tmp_rec->next != NULL)   //Run throught the list
    {
        if ((strcasecmp(term, tmp_rec->term) == 0)) //Check if the term is in the list
            break;                                  //If it's in the list break the loop
        else tmp_rec = tmp_rec->next;               //else point to the next element
    }

    if((strcasecmp(term, tmp_rec->term)==0))    //This line updates the flag if the
        flag = 0;                               //loop is broken. If it's not it's a
                                                //check for the last element of the
                                                //list
    return(flag);   //Return the flag
}

/*****************************************************************************/
/**************   Remove any invalid characters from the term   **************/

int remove_invalid_chars(char term[MAX_TERM_SIZE])
{
    //These are the valid characters that we allow
    char valid_chars[] = { 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F',
                           'g', 'G', 'h', 'H', 'i', 'I', 'j', 'J', 'k', 'K', 'l', 'L',
                           'm', 'M', 'n', 'N', 'o', 'O', 'p', 'P', 'q', 'Q', 'r', 'R',
                           's', 'S', 't', 'T', 'u', 'U', 'v', 'V', 'w', 'W', 'x', 'X',
                           'y', 'Y', 'z', 'Z', '0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9' };

    char buffer[MAX_TERM_SIZE]; //This will be the buffer that will contain all the
                                //valid chars
    int b_index;                //Index for the buffer
    int t_index;                //Index for the term
    int v_c_index;              //Index for the valid characters

    int flag;                   //This flag is used to check whether the character
                                //matched any of the valid ones
                                //flag = 0 Character did not match any of the valid ones
                                //flag = 1 Character matched one of the valid ones

    //For each charcter in the term
    for (t_index = 0, b_index = 0; t_index < strlen(term); t_index++)
    {
        flag = 0;       //Set the flag to false
        v_c_index = 0;  //Set the index to 0

        //Run through all the valid characters
        while (v_c_index < strlen(valid_chars))
        {
            //If we match a character
            if (term[t_index] == valid_chars[v_c_index])
            {
                flag = 1;       //Set the flag to true
                break;          //and break the loop
            }
            else
                v_c_index++;    //Else try the next valid character
        }

        if (flag)                               //If the character is valid
        {
            buffer[b_index] = term[t_index];    //save it to the buffer
            b_index++;                          //and increase the index
        }
    }

    //Copy the buffer to the original term
    strncpy(term, buffer, b_index);

    //And delete the remaining original characters at the end
    for (t_index = b_index; t_index < strlen(term); t_index++)
        term[t_index] = '\0';

    return strlen(term);
}

/*****************************************************************************/
