/*****************************************************************************/
/****************************   Include Files   ******************************/

#include <stdio.h>
#include <stdlib.h>
#include "save-to-xml.h"
#include "preprocess.h"

/*****************************************************************************/
/*********************   External variable declaration   *********************/

extern texts text_collection;       //Array containing all the text filenames with paths and the first
//element of their term list

extern int collection_size;         //Number of text files in the collection

extern termVectorCode code_list;    //The list containing the codes of the terms of the collection

/*****************************************************************************/
/*******************************   Functions   *******************************/

/*****************************************************************************/
/***********************   Saving lists to xml files   ***********************/

void save_to_xml()
{
    int counter;                //Loop counters and general temporary variables
    termVector tmp_term;        //to run through the term vectors lists and
    termVectorCode tmp_code;    //code list

    FILE *fp = NULL;            //The xml files containing the needed data of the lists

    /*
        Template for code-list.xml:

        <codes>
            <term id="code" value="Term" df="document frequency" />
        </codes>
    */

    //File check
    if ((fp = fopen(CODELIST, "w+")) == NULL)
    {
        printf("\n\nError!\nCould not create file: %s\nExiting...\n", CODELIST);
        exit;
    }

    printf("Generating %s ...", CODELIST);

    //Write the root element open tag
    fprintf(fp, "<codes>");

    //Run through the code list and create the nodes for the xml file
    tmp_code=code_list;
    while (tmp_code!=NULL)
    {
        //Write the xml node
        fprintf(fp, "<term id=\"%d\" ", tmp_code->code);
        fprintf(fp, "value=\"%s\" ", tmp_code->term);
        fprintf(fp, "df=\"%d\" />", tmp_code->DF);
        tmp_code=tmp_code->next;
    }

    //Write the root element close tag
    fprintf(fp, "</codes>");

    printf("\tOK!\n");
    fclose(fp);

    /*
        Template for term-collection.xml:

        <collection size="number of files">
            <file path="Path of the filename">
                <term code="code" frequency="Frequency in the file" weight="Term weight" />
            </file>
        </collection>
    */

    //File check
    if ((fp = fopen(COLLECTION, "w+")) == NULL)
    {
        printf("\n\nError!\nCould not create file: %s\nExiting...\n", COLLECTION);
        exit;
    }

    printf("Generating %s ...", COLLECTION);

    //Write the root element open tag
    fprintf(fp, "<collection size=\"%d\">", collection_size);

    //Loop through the list and create nodes for the xml file
    for (counter=0; counter<collection_size; counter++)
    {
        //Write the text file node open tag
        fprintf(fp, "<file path=\"%s\">", text_collection[counter].text_name);

        tmp_term=text_collection[counter].text_terms;

        while (tmp_term!=NULL)
        {
            fprintf(fp, "<term code=\"%d\" ", tmp_term->code);
            fprintf(fp, "frequency=\"%d\" ", tmp_term->TF);
            fprintf(fp, "weight=\"%f\" />", tmp_term->weight);
            tmp_term=tmp_term->next;
        }

        //Write the text file node close tag
        fprintf(fp, "</file>");
    }

    //Write the root element close tag
    fprintf(fp, "</collection>");

    printf("\tOK!\n\n");
    fclose(fp);
}

/*****************************************************************************/
