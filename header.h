#ifndef HEADER_FILE_H
#define HEADER_FILE_H

#define INFINITE 99999999.9
#define MAX_ARRAY 2000
#define DEBUG 0

struct traceEntry {
  double time; 
  int size; 
  int dest; //must be an integer (from 0 to nbofDest-1)
  int mcs; 
  int nbOfSrc; 
};


#endif





