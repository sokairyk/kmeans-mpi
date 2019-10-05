/*****************************************************************************/
/****************************   Include Files   ******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include "preprocess.h"
#include "save-to-xml.h"

/*****************************************************************************/
/**********************   Global variable declaration   **********************/

texts text_collection;              //Array containing all the text filenames with paths and the first
                                    //element of their term list

int collection_size;                //Number of text files in the collection

termVectorCode code_list = NULL;    //The list containing the codes of the terms of the collection

/*****************************************************************************/
/*********************************   Main   **********************************/

int main()
{

        preprocesing();
        save_to_xml();

        return EXIT_SUCCESS;
}

/*****************************************************************************/
