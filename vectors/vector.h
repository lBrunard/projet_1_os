/*
Implementation from : https://www.sanfoundry.com/c-program-implement-vector/

*/



#ifndef VECTOR_H
#define VECTOR_H
 
#define VECTOR_INIT_CAPACITY 4
 
#define VECTOR_INIT(vec) vector vec; vector_init(&vec)
#define VECTOR_ADD(vec, item) vector_add(&vec, (void *) item)
#define VECTOR_SET(vec, id, item) vector_set(&vec, id, (void *) item)
#define VECTOR_GET(vec, type, id) (type) vector_get(&vec, id)
#define VECTOR_DELETE(vec, id) vector_delete(&vec, id)
#define VECTOR_TOTAL(vec) vector_total(&vec)
#define VECTOR_FREE(vec) vector_free(&vec)
#define VECTOR_PRINT_STR(vec) vector_print_str(&vec)
 
typedef struct vector {
    void **items;
    int capacity;
    int total;
} vector;
 
/**
 * Inisialize a vector by getting a reference to a vector type
 **/
void vector_init(vector *);
/**
 * Get the size of vector 
 * @return (int) size of vector
**/
int vector_total(vector *);
/**
 * change the size of the vector 
**/
static void vector_resize(vector *, int);
/**
 * add element at the end of the vector 
**/
void vector_add(vector *, void *);
/**
 * add element at the index i of the vector 
**/
void vector_set(vector *, int, void *);
/**
 * Get an element of a vector
**/
void *vector_get(vector *, int);
/**
 * Delete an element of the vector at the index i
**/
void vector_delete(vector *, int);
/**
 * Clean memory allcoated by the vector
**/
void vector_free(vector *);
/**
 *Print the vector 
**/
void vector_print_str(vector *);
 
#endif