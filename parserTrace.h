

//Test unitaire
void test_traceFile();

//Open the file
FILE* openTrace(char* fileName);

//struct traceEntry is defined in header.h 
//read each line of the file unil an error occurs or if EOF.
int getNextFrameTrace(FILE* fileTrace, struct traceEntry* entry);

int mapAddressToDest(char* ipAddress);

int mapMCS(float phyRate);




