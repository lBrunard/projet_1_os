#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "vectors/vector.h"

#define MAX_IMAGE_NAME_LENGTH 10

static void Process_args(char* img_path, vector database, int argc, char* argv[]);


/**
*Doit recevoir comme argument la Photo a compareet comme second 
*le dossier conentant les photos avec lesquelles il doit comparer
*
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
*
**/
int main(int argc, char* argv[]){
    printf("%s", "Passed");
    char* image_to_compare = malloc(sizeof(char) * MAX_IMAGE_NAME_LENGTH);
    
    vector images_database;
    vector_init(&images_database);

    Process_args(image_to_compare, images_database, argc, argv);

    printf("%s", image_to_compare);
    for(int i = 0; i < vector_total(&images_database);i++){
        printf("%s", (char*) vector_get(&images_database, i));
    }

    vector_free(&images_database);
    free(image_to_compare);

    return 0;
}

static void Process_args(char* img_path, vector database, int argc, char* argv[]){
    int img_path_index = 0;

    regex_t re_image;
    int re_img_value = regcomp( &re_image, "\\.bmp$", 0);
    if(re_img_value){perror("Regex not complied"); exit(1);}
    if (!img_path_index && !regexec(&re_image,argv[1],0,NULL,0)){
            img_path = argv[1];
        }
    regfree(&re_image);
    
    for(int i = 2; i < argc; ++i){
        vector_add(&database, argv[i]);
    }

    
}