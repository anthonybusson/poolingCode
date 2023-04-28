#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "parserTrace.h"

#define DEBUG_TRACE 0
#define MAX_DEST 100

extern int nbOfDestFound;
extern int nbOfSrc; 
extern float cumulTime;
extern char listOfIpAddress[100][30];//A maximum of MAX_DEST destinations
extern double factorOnTimeTrace; 

//Test unitaire
void test_traceFile()
{
  FILE* file;
  struct traceEntry myEntry; 

  file=openTrace("traces/settopbox.txt");
  while(getNextFrameTrace(file, &myEntry)==1);

}

//Open the file
FILE* openTrace(char* fileName)
{
   FILE* theFile;//A FILE* must also be declared in the function that calls openTrace()
   	   
   if((theFile=fopen(fileName,"r"))<0) 
   {
	perror("Error fopen() in openTrace()");
	exit(2);
   }	   

   return(theFile);
}

//struct traceEntry is defined in header.h 
//read each line of the file until an error occurs or if EOF.
int getNextFrameTrace(FILE* fileTrace, struct traceEntry* entry)
{
  float time, pRate;
  int size; 
  char ipAddress[100]; 
  
  //Trace format: time size ipAddress(destination) phyRate
  if(fscanf(fileTrace,"%f %d %s %f\n",&time,&size,ipAddress,&pRate)<=0)
  {
	fclose (fileTrace);  
	return(0);//end of file
  } 
  
  if(time<0) { fprintf(stderr,"Error getNextFrameTrace(): negative time (%e). Abort.\n",time); exit(1); }

  entry->time=1.0e6*(time/factorOnTimeTrace - cumulTime);
  if(DEBUG_TRACE) fprintf(stderr,"DEBUG getNextFrameTrace(): time=%f time/factor=%f time/factor-cumulTime=%f *1.0e6=%f\n",time,time/factorOnTimeTrace,time/factorOnTimeTrace - cumulTime,1.0e6*(time/factorOnTimeTrace-cumulTime));
  cumulTime=time/factorOnTimeTrace;//time is the time from the beginning in the trace file
  entry->size=size;
  entry->dest=mapAddressToDest(ipAddress);
  entry->mcs=mapMCS(pRate);

  if(DEBUG_TRACE) fprintf(stderr,"DEBUG getNextFrameTrace(): time=%f microsec - size=%d ipAddress=%s phyRate=%f\n",entry->time,size,ipAddress,pRate);

  if(DEBUG_TRACE) fprintf(stderr,"DEBUG getNextFrameTrace(): pRate=%f mcs=%d\n",pRate,entry->mcs);

  return(1);
}

//This function maps an ip address to one of the destination (numbered from 0 to nbOfDest-1) 
//It returns the destination number
int mapAddressToDest(char* ipAddress)
{
				       //
  for(int i=0; i<nbOfDestFound;i++)
  {
	if(DEBUG_TRACE) fprintf(stderr,"DEBUG mapAddressToDest(): ipAddress (string)=%s compare to %s gives %d (should be 0 if they are equal)\n",ipAddress,listOfIpAddress[i],strcmp(ipAddress,(char *) listOfIpAddress[i]));
	if(strcmp(ipAddress,(char *) listOfIpAddress[i])==0) return(i);
  }

  //We have found a new destination
  if(nbOfDestFound>=MAX_DEST-1) { fprintf(stderr,"Error mapAddressToDest(): the maximum number of destinations has been reached.\n"); exit(1); }
  strcpy(listOfIpAddress[nbOfDestFound],ipAddress); 
  nbOfDestFound++;
  nbOfSrc=nbOfDestFound; //We update nbOfSrc used in the other files with the good number of sources/dest
  if(DEBUG_TRACE) fprintf(stderr,"DEBUG mapAddressToDest(): new ip address %s maps to destination %d\n",ipAddress,nbOfDestFound-1);
  
  return(nbOfDestFound-1);
}

//This function maps the 802.11n phy rate of the traces to the same MCS number in 802.11ax
//It returns the MCS (an integer)
int mapMCS(float pRate)
{

  if(pRate < 6.5) return(0);  //It is not 802.11n: 1, 2, 5.5 Mbit/s can be found in trace file --> we associate to MCS 0
  if(6.0 <= pRate && pRate  < 7.5)  return(0); //include 802.11n 6.5 -> MCS 0   and 7.2 MCS 0 but with a GI=400ns
  if(7.5 <= pRate && pRate < 14.5) return(1); //include 13 and 14.44 (shorter GI)
  if(15.0 <= pRate && pRate < 20.0) return(2); 
  if(21.0 <= pRate && pRate < 22.0) return(2);//shorter GI
  if(24.0 <= pRate && pRate < 27.0) return(1); //It is MCS 9 for 802.11n but with the same characteristics as MCS 1 (QPSK code 1/2). The difference is the use of 2 spatial streams rather than 1.
  if(28.0 <= pRate && pRate < 29.0) return(3); 
  if(39.0 <= pRate && pRate <=39.9) return(4);
  if(43.0 <= pRate && pRate <=44.0) return(4);//shorter GI
  if(52.0 <= pRate && pRate < 55.0) return(5);
  if(57.0 <= pRate && pRate < 58.0) return(5);//shorter GI
  if(58.0 <= pRate && pRate < 59.0) return(6);
  if(65.0 <= pRate && pRate < 66.0) return(7);
  if(72.0 <= pRate && pRate < 73.0) return(7);//shorter GI
  if( pRate >= 78.0 ) return(11);//max is 11
	
  fprintf(stderr,"Error mapMCS(): the MCS %f has not been found. The mapping needs to be updates. Abort.\n",pRate);  
  exit(1);	  

}
