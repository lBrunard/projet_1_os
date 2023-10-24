#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "vectors/vector.h"

#define MAX_IMAGE_NAME_LENGTH 999

void Process_args(char* img_path, vector database, int argc, char* argv[]);


/**
*Doit recevoir comme argument la Photo a compareet comme second 
*le dossier conentant les photos avec lesquelles il doit comparer
*
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
*
**/
int main(int argc, char* argv[]){
    if (argc < 3){{perror("No params"); exit(1);}}
    char *image_to_compare = "Hello";
    
    vector images_database;
    vector_init(&images_database);
    Process_args(image_to_compare, images_database, argc, argv);
    printf("%s\n", "Args Exited");
    vector_print_str(&images_database);
    printf("%s\n", "Args Exited");
    vector_free(&images_database);

    return 0;
}

void Process_args(char* image_to_compare, vector database, int argc, char* argv[]){
    int img_path_index = 0;
    regex_t re_image_bmp;
    int re_img_value = regcomp(&re_image_bmp, "\\.bmp$", 0);
    if(re_img_value){perror("Regex not complied"); exit(1);}
    if (!img_path_index){
        if(regexec(&re_image_bmp,argv[1],0,NULL,0) == 0){
            image_to_compare = argv[1];
        }else{
            {perror("Imput Photo is not .bmp"); exit(1);}
        }
    }
    regfree(&re_image_bmp);
    
    for(int i = 2; i < argc; ++i){
        printf("%s", argv[i]);
        vector_add(&database, argv[i]);
    }

    
}