/**
 * @file error_handling.h
 * @brief Contient les fonctions de gestion d'erreur
 * @note Les fonctions de ce fichier sont inspirée des fonctions donnée en séance de TP
*/
#ifndef __ERROR_HANDLING_H
#define __ERROR_HANDLING_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Fonction qui permet de gérer les erreurs
 * @param ret la valeur de retour de la fonction à tester
 * @param calling_function le nom de la fonction appelante
 * @return la valeur de retour de la fonction à tester
 * @note Si la valeur de retour est négative, la fonction affiche un message d'erreur et quitte le programme avec EXIT_FAILURE
*/
int _checked(int ret, char*calling_function){
  if(ret == -1){
    perror(calling_function);
    exit(EXIT_FAILURE);
  }
  return ret;
}
#define checked(call) _checked(call, #call)

#endif // !__ERROR_HANDLING_H
