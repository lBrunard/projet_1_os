#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#define MAX_IMAGE_NAME_LENGTH 999

void Process_args(char* img_path, int argc, char* argv[]);


/**
*Doit recevoir comme argument la Photo a compareet comme second 
*le dossier conentant les photos avec lesquelles il doit comparer
*
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
*
**/
int main(int argc, char* argv[]){
    char *image_to_compare;
    image_to_compare= (char *)malloc(sizeof(char)*MAX_IMAGE_NAME_LENGTH);
    if (image_to_compare == NULL) {
    fprintf(stderr, "Erreur d'allocation mémoire\n");
    exit(EXIT_FAILURE);
}
    Process_args(image_to_compare, argc, argv);


    free(image_to_compare);
    return 0;
}

void Process_args(char* image_to_compare, int argc, char* argv[]){

    if (argc!=2)
    {

        printf("nombre d'argument incorecte");
        exit(1);        
        
    }
    else
    {
        strcat(image_to_compare,argv[1]);
    }

    

    
}