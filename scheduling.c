#include<stdio.h>
#include<stdlib.h>
#include"header.h"
#include"scheduling.h"
#include"simulator.h"

#define DEBUG_ALGO6 0
#define DEBUG_ALGO7 0
#define DEBUG_ALGO8 0
#define MAX_NBOFLISTS 3000 //Max 2^20=1048577

extern int nbOfServicesInProgress, X, nbOfSrc, *queue, *packetSize;
extern double OV;
extern double ofdmaRate[12][7]; 
extern double phyRate[12];
extern int MCS[MAX_ARRAY];

/************************************************************************************************
 *												*
 *     	Common functions to the different service disciplines 					*
 *												*	
*************************************************************************************************/


//Process the queue and keeps in typeList/packetList
//the first packet of each different destination. 
//It returns the number of different dest found. 
int updateOfdmaPacketList(int* typeList, int* packetList)
{
   int differentType;		

   typeList[0]=queue[0];
   packetList[0]=0;
   differentType=1;

   int i=1; 
   while(differentType<nbOfSrc && i<X)
   {
	if (!isInList(queue[i], typeList, differentType))
	{
	   typeList[differentType]=queue[i];
	   packetList[differentType]=i;
	   differentType++;
	}
	i++;
   }

   return(differentType);
}

/*Recursive function that lists all subsets in a given array
* For array={0,1,2} it gives: 
 0 1 2
 0 1
 0 2
 0
 1 2
 1
 2
* 
*
*/

int nbOfSubsets=0; //Global Variable (must be updated in the calling function)
int listOfLists[MAX_NBOFLISTS][MAX_ARRAY];

void updateList(int* subset, int len)
{
  for(int i=0;i<len;i++) listOfLists[nbOfSubsets][i]=subset[i];
  for(int i=len;i<MAX_ARRAY;i++) listOfLists[nbOfSubsets][i]=-1;
  nbOfSubsets++;
  if(nbOfSubsets>=MAX_NBOFLISTS) { fprintf(stderr,"Error updateList() the maximal capacity of the list (MAX_NBOFLISTS) has been reached nbOfSubsets=%d. Do not forget that the number of lists is 2^n for n destnations. Abort.\n",nbOfSubsets); exit(3);}
}

//List all subsets among n destinations. 
//The number of lists for n destinations is 2^n. 
//For 20 destinations = 1024*1024 = 1048576
void allSubset(int pos, int len, int* subset, int* array, int arraySize) 
{
  if(pos==arraySize)
  { 
	//We process the set here  
	if(len>0) 
	{
  	  //for(int i=0;i<len;i++) printf(" %d",subset[i]); printf("\n");
	  updateList(subset,len);
	}

	return;//It is finished for this branch
  }

  subset[len]=array[pos];  
  allSubset(pos+1,len+1,subset,array,arraySize);
  allSubset(pos+1,len,subset,array,arraySize);  

}

//Returns 1 if the current allocation < 242 tones
//0 otherwise
int nbOfSubCarriers[7]={26,52,106,242,484,996,1992};
int checkAllocation(int len, int* combination)
{
  //We checks if this allocation is possible
  int sum=0;
  for(int i=0; i<len; i++) sum+=nbOfSubCarriers[combination[i]];
  if(sum>242) 
  {
    if(DEBUG_ALGO6 | DEBUG_ALGO7) fprintf(stderr,"DEBUG allcombination: this allocation is impossible.\n"); 
    return(0);
  }

  return(1);
}

//
void allCombination(int* packetList, int len, int* combination, int nbOfPackets, int* bestAllocation, double* ptrBestScore) 
{ 

  //7 Column: RU-26 RU-52 RU-106 RU-242(20MHz) RU-484 (40MHz) RU-996 (80MHz) RU-2x996 (160MHz)
	
  if(DEBUG_ALGO6){ fprintf(stderr,"DEBUG allCombination() for the packet/allocation "); for(int i=0;i<len;i++) fprintf(stderr,"%d/%d ",packetList[i],combination[i]); fprintf(stderr,"\n"); }

  //Do we exceed the number of tones? 
  if(!checkAllocation(len,combination)) return; 

  if(len==nbOfPackets)
  {  
	if(DEBUG_ALGO6) fprintf(stderr,"DEBUG allCombination(): process this set\n");
	
	//Process here  
  	double localScore=computeOverheadPacketRatio(packetList,combination,len);
	if(DEBUG_ALGO6) fprintf(stderr,"DEBUG allCombination(): localScore=%e\n",localScore);

	if(localScore < *ptrBestScore) 
	{
		*ptrBestScore=localScore;
		for(int i=0; i<len; i++) bestAllocation[i]=combination[i];
	}

	if(DEBUG_ALGO6) fprintf(stderr,"DEBUG allCombination(): localScore=%e bestScore=%e\n",localScore,*ptrBestScore);
	return;//It is finished for this branch
  }

  //3 is the number of possible tones (26, 52, and 106 here) 
  for(int i=0; i<3; i++) 
  {
	combination[len]=i;
	allCombination(packetList,len+1,combination,nbOfPackets,bestAllocation,ptrBestScore);
  }
}

double computeOfdmaTransmissionTime(int* packetList, int* allocation, int size)
{
  double time, greatestTime=0.0; 

  if(DEBUG_ALGO6){ fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() for the packet/allocation "); for(int i=0;i<size;i++) fprintf(stderr,"%d/%d ",packetList[i],allocation[i]); fprintf(stderr,"\n"); }
  for(int i=0;i<size;i++) 
  {
    if(DEBUG_ALGO6)
    {	    
     fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() packetSize = %d\n",packetSize[packetList[i]]);
     fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() type = %d\n",queue[packetList[i]]);
     fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() MCS = %d\n",MCS[queue[packetList[i]]]);
     fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() allocation = %d\n",allocation[i]);
     fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() ofdmaRate = %e\n",ofdmaRate[MCS[queue[packetList[i]]]][allocation[i]]);
    }
    time=(8.0*packetSize[packetList[i]])/(1.0*ofdmaRate[MCS[queue[packetList[i]]]][allocation[i]]); //Given in microSec
    if(time>greatestTime) greatestTime=time;												     
  }

  if(DEBUG_ALGO6) fprintf(stderr,"DEBUG computeOfdmaTransmissionTime() ofdmaTransmissionTime= %e\n",greatestTime+OV);
  return(greatestTime+OV);
}

double computeAggTransmissionTime(int* packetList, int size)
{
  double time=0.0; 

  for(int i=0;i<size;i++) 
  {
    if(DEBUG_ALGO6)
    {	    
     fprintf(stderr,"DEBUG aggTransmissionTime tmp result (i=%d)=%e\n",i,time);
     fprintf(stderr,"DEBUG aggTransmissionTime %d\n",packetSize[packetList[i]]);
     fprintf(stderr,"DEBUG aggTransmissionTime %d\n",MCS[queue[packetList[i]]]);
     fprintf(stderr,"DEBUG aggTransmissionTime %e\n",phyRate[MCS[queue[packetList[i]]]]);
    }

    time+=(8.0*packetSize[packetList[i]]) / (1.0*phyRate[MCS[queue[packetList[i]]]]); //in microSeconds
    double tmp=(8.0*packetSize[packetList[i]]) / (1.0*phyRate[MCS[queue[packetList[i]]]]); //in microSeconds
    if(DEBUG_ALGO6) fprintf(stderr,"DEBUG aggTransmissionTime tmp result (i=%d)=%e tmp=%e\n",i,time,tmp);
  }

  if(DEBUG_ALGO6) fprintf(stderr,"DEBUG aggTransmissionTime final result=%e\n",time+OV);
  return(time+OV);
}

//Compute the OFDMA overhead over packet ratio
//Each packet in packetList is intended to a different destination. 
double computeOverheadPacketRatio(int* packetList, int* allocation, int size)
{
  double ratio;

  if(DEBUG_ALGO6){ fprintf(stderr,"DEBUG computeOverheadPacketRatio() packetList/combination "); for(int i=0;i<size;i++) fprintf(stderr,"%d/%d ",packetList[i],allocation[i]); fprintf(stderr,"\n");}

  double time=computeOfdmaTransmissionTime(packetList,allocation,size);
  ratio=(OV+time-computeAggTransmissionTime(packetList,size))/(1.0*size); //overhead per packet = (OV + (ofdmaTime-AggTime))/#packets

  if(DEBUG_ALGO6) fprintf(stderr,"DEBUG computeOverheadPacketRatio() ratio=%e\n",ratio); 
  return(ratio);
}

//Compute the OFDMA overhead over bytes ratio
//Each packet in packetList is intended to a different destination. 
//It returns the best overhead over Bytes ratio and put in allocation the associated allocation.
double computeBestAllocation(int* packetList, int* allocation, int size)
{
  //!!!!!!!!!!!   packetList contains the index of the packet in the queue   		
  //!!!!!!!!!!!   queue its type (global variable) and packetSize (global variable)
  //!!!!!!!!!!!   The type/destination of the first packet in packetList is queue[packetList[0]] with size packetSize[packetList[0]]
  //!!!!!!!!!!!   its MCS is MCS[queue[packetList[0]]] and its phyRate phyRate[MCS[queue[packetList[0]]]] 
  double bestRatio=99999.0; //The best is the lowest

  //The idea is to send all the packets in packetList and not a subset - so a lot of combinations are not possible. 
  //802.11ax: 26, 52, 106 or 242 tones - one RU per destination (no more) 
  //242: a single packet only
  //106: two packets only
  //52: from 3 to 4 packets
  //26: up to 8 packets (a verifier avec Tuan) 

  if(size==0) { fprintf(stderr,"Error fct overheadBytesRatio(): 0 packet to process. Abort.\n"); exit(1); }
  if(size==1) 
  {
    allocation[0]=4;//242 tones
    bestRatio= OV/(1.0); 
    return bestRatio;
  }

  int combination[MAX_ARRAY]; 

  //We try all possible allocations and keeps the best. 
  allCombination(packetList,0,combination, size, allocation, &bestRatio);


  return(bestRatio);
}



/******************************************************
 *
 *     	FIFO: one packet at the time in FIFO order
 *
******************************************************/

//Cherche les paquets qui vont entrer dans le serveur
void setServicesFIFO(int* packetList)
{
   packetList[0]=0;
   nbOfServicesInProgress=1;
}


/******************************************************
 *
 *      FIFO: but that aggregates or performs OFDMA when it can
 *
******************************************************/

//Determine packets that are going to be served 
void setServicesFIFOpooling(int* packetList)
{

   if(X==1)
   {
	packetList[0]=0;	
   	nbOfServicesInProgress=1;
	return;
   } 
   
   //The two first frames determine if we use OFDMA or Agg
   if(queue[0]==queue[1]) //Agg
   {
   	int nbOfAggFrames=2;
 	while( nbOfAggFrames+1<=X && queue[nbOfAggFrames]==queue[0]) nbOfAggFrames++;

	for(int i=0;i<nbOfAggFrames;i++) packetList[i]=i;
   	nbOfServicesInProgress=nbOfAggFrames;

   } else { //OFDMA

   	//We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   
     	int typeList[MAX_ARRAY];

   	typeList[0]=queue[0];
   	typeList[1]=queue[1];

   	int nbOfOfdmaFrames=2, OK=1; 
   	while(nbOfOfdmaFrames<nbOfSrc && nbOfOfdmaFrames<X && OK)
   	{
	  if (!isInList(queue[nbOfOfdmaFrames], typeList, nbOfOfdmaFrames))
	  {
	     typeList[nbOfOfdmaFrames]=queue[nbOfOfdmaFrames];
	     nbOfOfdmaFrames++;
	  } else OK=0;
   	}
	for(int i=0;i<nbOfOfdmaFrames;i++) packetList[i]=i;
   	nbOfServicesInProgress=nbOfOfdmaFrames;
   }

   //DEBUG 
   //fprintf(stderr,"DEBUG FIFO pooling. Current queue:"); for(int i=0;i<X;i++) fprintf(stderr," %d",queue[i]); fprintf(stderr,"\n");
   //fprintf(stderr,"DEBUG FIFO pooling. Selected frames (their index in the queue):"); for(int i=0;i<nbOfServicesInProgress;i++) fprintf(stderr," %d",packetList[i]); fprintf(stderr,"\n");
}




/******************************************************
 *
 *       	Max Pooling 
 *
******************************************************/

//Determine packets that are going to be served 
void setServicesOpt(int* packetList)
{
   //How many packets of the same type we have? Keep the greatest.
   int bestType=0, maxType=0;

   if(X==1)
   {
	packetList[0]=0;	
   	nbOfServicesInProgress=1;
	return;
   }

   for(int type=0; type<nbOfSrc; type++)
   {
	int count=0;   
	for(int j=0;j<X;j++) if(queue[j]==type) count++; 
	if(count>maxType){ maxType=count; bestType=type; } 
   }

   //Does OFDMA better than Agg? 
   //We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   //It takes the first packet for each destination (keep in mind that we do not allow order change for a same destination)
   int typeList[MAX_ARRAY], differentType;		
   differentType=updateOfdmaPacketList(typeList, packetList);

   //Here we can change from priority to OFDMA or Agg in case of equality
   if(maxType >= differentType)  
   {
      //We update packetList
      int index=0; 
      for(int j=0;j<X;j++)
      { 
	  if(queue[j]==bestType)
	  {
		packetList[index]=j;
		index++;
	  }
      }
      nbOfServicesInProgress=maxType;

      //DEBUG
      if(index!=maxType){ fprintf(stderr,"Error index!=maxtTypes\n");exit(1);}
      //fprintf(stderr,"--DEBUG service: next service Aggregation with %d packets of type %d (X=%d)\n",maxType,bestType,X);

   } else {

      nbOfServicesInProgress=differentType;

      //DEBUG
      //fprintf(stderr,"--DEBUG service: next service OFDMA with %d packets (X=%d)\n",differentType,X);
   }

}



/******************************************************
 *
 *       	Max OFDMA 
 *
******************************************************/

//Determine packets that are going to be served 
void setServicesOFDMA(int* packetList)
{
   //OFDMA 
   //We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   
   int typeList[MAX_ARRAY], differentType;		
   differentType=updateOfdmaPacketList(typeList, packetList);

   nbOfServicesInProgress=differentType;

}


/******************************************************
 *
 *       	Max Aggregation
 *
******************************************************/

//Determine packets that are going to be served 
void setServicesAgg(int* packetList)
{
   //How many packets of the same type we have? Keep the greatest.
   int bestType=0, maxType=0;

   for(int type=0; type<nbOfSrc; type++)
   {
	int count=0;   
	for(int j=0;j<X;j++) if(queue[j]==type) count++; 
	if(count>maxType){ maxType=count; bestType=type; } 
   }

   //We update packetList
   int index=0; 
   for(int j=0;j<X;j++)
   { 
	 if(queue[j]==bestType)
	 {
	   packetList[index]=j;
	   index++;
	 }
   }
   nbOfServicesInProgress=maxType;

}




/******************************************************
 *
 *       	FIFO Max Pooling 
 *
******************************************************/

//We optimize he pooling but we keep the first packet in the queue

//Determine packets that are going to be served 
void setServicesFIFOMaxPool(int* packetList)
{
   //How many packets of the same type as the first packet we have? Keep the greatest.
   int type=queue[0];

   if(X==1)
   {
	packetList[0]=0;	
   	nbOfServicesInProgress=1;
	return;
   }

   int countAgg=0;   
   for(int j=0;j<X;j++) if(queue[j]==type) countAgg++; 

   //Does OFDMA better than Agg? 
   //We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   
   int typeList[MAX_ARRAY], differentType;		
   differentType=updateOfdmaPacketList(typeList, packetList);

   //Here we can change from priority to OFDMA or Agg in case of equality
   
   if(countAgg >= differentType)  
   {
      //We update packetList
      int index=0; 
      for(int j=0;j<X;j++)
      { 
	  if(queue[j]==queue[0])
	  {
		packetList[index]=j;
		index++;
	  }
      }
      nbOfServicesInProgress=countAgg;

      //DEBUG
      if(index!=countAgg){ fprintf(stderr,"Error index!=maxtTypes\n");exit(1);}
      //fprintf(stderr,"--DEBUG service: next service Aggregation with %d packets of type %d (X=%d)\n",maxType,bestType,X);

   } else {

      nbOfServicesInProgress=differentType;

      //DEBUG
      //fprintf(stderr,"--DEBUG service: next service OFDMA with %d packets (X=%d)\n",differentType,X);
   }

}


/***************************************************************************************
 *
 *       	Realistic Max Pooling that keeps packets order for a given destination
 *       	The same as Max Pooling but the OFDMA depends on the RU allocation
 *		It is a greedy algorithm that takes the packets that minimizes the ratio overhead/#packets.
 *		  - best agg in terms of nb of bytes (the ratio is then OV/nbOfBytes)
 *		  - among all possible destinations (we take only the first packet for each dest) the ratio is (OV+B)/#packets (we have also OV+B=OV+time_ofdma-time_agg)
 *			- the complexity for N sources is N+C^2_N +C^3_N+..+C^N_N=2^N
 *			- it is not polynomial with the number of destination but it probably is with the number of packets
 *
***************************************************************************************/

//Determine packets that are going to be served 
double setServicesRealisticMaxPool(int* packetList)
{
   int bestType=0;
   double bestRatioAgg;//Must be as small as possible

   //Only one packet in the queue
   if(X==1)
   {
	packetList[0]=0;	
   	nbOfServicesInProgress=1;
	int alloc=4;
        double time=computeOfdmaTransmissionTime(packetList, &alloc, 1);
	return (time);
   }

   //For aggregation, it suffices to count the nb of packets
   int maxNbOfPackets=0;
   for(int type=0; type<nbOfSrc; type++)
   {
	int nbOfPackets=0;    
	for(int j=0;j<X;j++) if(queue[j]==type) nbOfPackets++;
	if(nbOfPackets>maxNbOfPackets){ bestType=type; maxNbOfPackets=nbOfPackets; } 
	if(DEBUG_ALGO6) fprintf(stderr,"DEBUG new algo: AGG dest %d nbOfPackets=%d\n",type,nbOfPackets);
   }
   bestRatioAgg = OV / (1.0*maxNbOfPackets); 
   if(DEBUG_ALGO6) fprintf(stderr,"DEBUG setServicesReaisticMaxPool() agg bestRatioAgg=%e\n",bestRatioAgg);


   //Does OFDMA better than Agg? 
   //We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   //It takes the first packet of each destination (keep in mind that order change for a same destination is not allowed)
   int typeList[MAX_ARRAY], tmpPacketList[MAX_ARRAY], differentType;		
   differentType=updateOfdmaPacketList(typeList, tmpPacketList);

   //DEBUG
   if(DEBUG_ALGO6)
   {
    fprintf(stderr,"DEBUG list ofdma initial: ");
    for(int k=0;k<differentType;k++) fprintf(stderr," %d",tmpPacketList[k]);
    fprintf(stderr,"\n");
   }

   int subset[MAX_ARRAY];
   nbOfSubsets=0;//Do not forget this
   allSubset(0, 0, subset, tmpPacketList, differentType);

   //La fct ci-dessus a mis a jour une liste de listes (tableaux 2D): listOfLists.
   //Le nombre de listes est de nbOfSubsets
   //La taille d'une liste est determinee par les -1 (qui debute dans le tableau car on est arrive a la fin)
   //DEBUG
   if(DEBUG_ALGO6)
   { 
     fprintf(stderr,"** DEBUG liste de listes **\n");
     for(int k=0;k<nbOfSubsets;k++)
     { 
      fprintf(stderr,"Liste %d:",k);
      int l=0;
      while(listOfLists[k][l]>=0){ fprintf(stderr," %d",listOfLists[k][l]); l++;}
      fprintf(stderr,"\n");
     }
   }
   
   //On determine le meilleur sous ensemble 
   double bestScoreOFDMA=1000.0, scoreOFDMA;
   int allocation[MAX_ARRAY];
   int bestAllocation[MAX_ARRAY];
   int bestList, sizeList;

   for(int i=0; i<nbOfSubsets;i++)
   {
     int size=0; while(listOfLists[i][size]>=0) size++;
     if(DEBUG_ALGO6) fprintf(stderr,"Taille liste %d = %d\n",i,size);
     scoreOFDMA=computeBestAllocation(listOfLists[i], allocation, size);//C'est ici que cela deconne...
     if(scoreOFDMA<bestScoreOFDMA) { bestScoreOFDMA=scoreOFDMA; bestList=i; sizeList=size; for(int j=0;j<size;j++) bestAllocation[j]=allocation[j]; }
   }
    
   if(DEBUG_ALGO6) 
   {
    fprintf(stderr,"Best allocation found for OFDMA packet/allocation "); for(int i=0;i<sizeList;i++) fprintf(stderr,"%d/%d ",listOfLists[bestList][i],bestAllocation[i]); fprintf(stderr," score=%e\n",bestScoreOFDMA); 
    fprintf(stderr,"Best allocation found for AGG bestType=%d Nb of packets=%d ratio=%e\n",bestType,maxNbOfPackets,bestRatioAgg);
   }


   //Here we can change from priority to OFDMA or Agg in case of equality
   if(bestRatioAgg <= bestScoreOFDMA)  
   {
      //We update packetList
      int index=0; 
      for(int j=0;j<X;j++)
      { 
	  if(queue[j]==bestType)
	  {
		packetList[index]=j;
		index++;
	  }
      }
      
      nbOfServicesInProgress=maxNbOfPackets;
      double time=computeAggTransmissionTime(packetList, maxNbOfPackets);

      return(time);

   } else {
      for(int j=0;j<sizeList;j++) packetList[j]=listOfLists[bestList][j];
      nbOfServicesInProgress=sizeList;
      double time=computeOfdmaTransmissionTime(listOfLists[bestList], bestAllocation, sizeList);

      return(time);
   }
}



/***************************************************************************************
 *
 *       	Realistic FIFO MAX Pooling that keeps packets order for a given destination
 *       	The same as Realistic Max Pooling but the destinations considered in the OFDMA transmission are necessarily in the same order as the buffer.
 *       	For instance, if we have (dest): 0 0 2 1 3 4 4 0 in the buffer. 
 *       	The OFDMA possibility are 0 ; 0 2 ; 0 2 1 ; 0 2 1 3 ; 0 2 1 3 4 
 *       	It breaks the complexity as it is O(N) with N the number of destinations. 
 *
***************************************************************************************/

//Determine packets that are going to be served 
double setServicesRealisticFifoMaxPool(int* packetList)
{
   int bestType=0;
   double bestRatioAgg;//Must be as small as possible

   //Only one packet in the queue
   if(X==1)
   {
	packetList[0]=0;	
   	nbOfServicesInProgress=1;
	int alloc=4;
        double time=computeOfdmaTransmissionTime(packetList, &alloc, 1);
	return (time);
   }

   //For aggregation, it suffices to count the nb of packets
   int maxNbOfPackets=0;
   for(int type=0; type<nbOfSrc; type++)
   {
	int nbOfPackets=0;    
	for(int j=0;j<X;j++) if(queue[j]==type) nbOfPackets++;
	if(nbOfPackets>maxNbOfPackets){ bestType=type; maxNbOfPackets=nbOfPackets; } 
	if(DEBUG_ALGO6) fprintf(stderr,"DEBUG new algo: AGG dest %d nbOfPackets=%d\n",type,nbOfPackets);
   }
   bestRatioAgg = OV / (1.0*maxNbOfPackets); 
   if(DEBUG_ALGO7) fprintf(stderr,"DEBUG setServicesReaisticMaxPool() agg bestRatioAgg=%e\n",bestRatioAgg);


   //Does OFDMA better than Agg? 
   //We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   //It takes the first packet of each destination (keep in mind that order change for a same destination is not allowed)
   int typeList[MAX_ARRAY], tmpPacketList[MAX_ARRAY], differentType;		
   differentType=updateOfdmaPacketList(typeList, tmpPacketList);

   //DEBUG
   if(DEBUG_ALGO7)
   {
    fprintf(stderr,"DEBUG algo 7 list ofdma initial: ");
    for(int k=0;k<differentType;k++) fprintf(stderr," %d",tmpPacketList[k]);
    fprintf(stderr,"\n");
   }

   nbOfSubsets=differentType;//Do not forget this
   for(int i=0; i<differentType; i++)		 
   {
     for(int j=0; j<=i ; j++) listOfLists[i][j]=tmpPacketList[j];
     for(int j=i+1; j<MAX_ARRAY ; j++) listOfLists[i][j]=-1;
   }
		 

   //La fct ci-dessus a mis a jour une liste de listes (tableaux 2D): listOfLists.
   //Le nombre de listes est de nbOfSubsets
   //La taille d'une liste est determinee par les -1 (qui debute dans le tableau car on est arrive a la fin)
   //DEBUG
   if(DEBUG_ALGO7)
   { 
     fprintf(stderr,"** DEBUG liste de listes **\n");
     fprintf(stderr,"Queue: ");
     for(int k=0;k<X;k++){ fprintf(stderr," %d",queue[k]); } fprintf(stderr,"\n");

     for(int k=0;k<nbOfSubsets;k++)
     { 
      fprintf(stderr,"Liste %d:",k);
      int l=0;
      while(listOfLists[k][l]>=0){ fprintf(stderr," %d",listOfLists[k][l]); l++;}
      fprintf(stderr,"\n");
     }
   }
   
   //On determine le meilleur sous ensemble 
   double bestScoreOFDMA=1000.0, scoreOFDMA;
   int allocation[MAX_ARRAY];
   int bestAllocation[MAX_ARRAY];
   int bestList, sizeList;

   for(int i=0; i<nbOfSubsets;i++)
   {
     int size=0; while(listOfLists[i][size]>=0) size++;
     if(DEBUG_ALGO6) fprintf(stderr,"Taille liste %d = %d\n",i,size);
     scoreOFDMA=computeBestAllocation(listOfLists[i], allocation, size);//C'est ici que cela deconne...
     if(scoreOFDMA<bestScoreOFDMA) { bestScoreOFDMA=scoreOFDMA; bestList=i; sizeList=size; for(int j=0;j<size;j++) bestAllocation[j]=allocation[j]; }
   }
    
   if(DEBUG_ALGO7) 
   {
    fprintf(stderr,"Best allocation found for OFDMA packet/allocation "); for(int i=0;i<sizeList;i++) fprintf(stderr,"%d/%d ",listOfLists[bestList][i],bestAllocation[i]); fprintf(stderr," score=%e\n",bestScoreOFDMA); 
    fprintf(stderr,"Best allocation found for AGG bestType=%d Nb of packets=%d ratio=%e\n",bestType,maxNbOfPackets,bestRatioAgg);
   }


   //Here we can change from priority to OFDMA or Agg in case of equality
   if(bestRatioAgg <= bestScoreOFDMA)  
   {
      //We update packetList
      int index=0; 
      for(int j=0;j<X;j++)
      { 
	  if(queue[j]==bestType)
	  {
		packetList[index]=j;
		index++;
	  }
      }
      
      nbOfServicesInProgress=maxNbOfPackets;
      double time=computeAggTransmissionTime(packetList, maxNbOfPackets);

      return(time);

   } else {
      for(int j=0;j<sizeList;j++) packetList[j]=listOfLists[bestList][j];
      nbOfServicesInProgress=sizeList;
      double time=computeOfdmaTransmissionTime(listOfLists[bestList], bestAllocation, sizeList);

      return(time);
   }
}


/***************************************************************************************
 *
 *       	Realistic FIFO that keeps packets order for a given destination
 *       	A sequence change is not allowed in this algorithm: we take necessarily the first packets. 
 *       	For instance, if we have (dest): 0 0 2 1 3 4 4 0 in the buffer, we take 0 0.
 *       	For instance, if we have (dest): 0 1 2 1 3 4 4 0 in the buffer, we take at most 0 1 2.
 *
***************************************************************************************/

double setServicesRealisticFifoPooling(int* packetList)
{
   int bestType=0;
   double bestRatioAgg;//Must be as small as possible

   //Only one packet in the queue
   if(X==1)
   {
	packetList[0]=0;	
   	nbOfServicesInProgress=1;
	int alloc=4;
        double time=computeOfdmaTransmissionTime(packetList, &alloc, 1);
	return (time);
   }

   //For aggregation, it suffices to count the nb of packets
   int maxNbOfPackets=1;
   bestType=queue[0];
   while(maxNbOfPackets<X && queue[maxNbOfPackets]==bestType) maxNbOfPackets++;

   bestRatioAgg = OV / (1.0*maxNbOfPackets); 
   if(DEBUG_ALGO8) fprintf(stderr,"DEBUG setServicesReaisticFIFO() agg bestRatioAgg=%e\n",bestRatioAgg);


   //Does OFDMA better than Agg? 
   //We keep in packetListOFDMA[] the list of the packets that would correspond to an OFDMA transmission
   //It takes the first packet of each destination (keep in mind that order change for a same destination is not allowed)
   int typeList[MAX_ARRAY], tmpPacketList[MAX_ARRAY], differentType;		
   differentType=updateOfdmaPacketList(typeList, tmpPacketList);

   //Reduces the size of the list to allow only contigus different type at the beginning of the queue. 
   int k=1;
   while(k<differentType && tmpPacketList[k]==k) k++;
   differentType=k;


   //DEBUG
   if(DEBUG_ALGO8)
   {
    fprintf(stderr,"DEBUG algo 8 list ofdma initial: ");
    for(int k=0;k<differentType;k++) fprintf(stderr," %d",tmpPacketList[k]);
    fprintf(stderr,"\n");
   }

   nbOfSubsets=differentType;//Do not forget this
   for(int i=0; i<differentType; i++)		 
   {
     for(int j=0; j<=i ; j++) listOfLists[i][j]=tmpPacketList[j];
     for(int j=i+1; j<MAX_ARRAY ; j++) listOfLists[i][j]=-1;
   }
		 

   //La fct ci-dessus a mis a jour une liste de listes (tableaux 2D): listOfLists.
   //Le nombre de listes est de nbOfSubsets
   //La taille d'une liste est determinee par les -1 (qui debute dans le tableau car on est arrive a la fin)
   //DEBUG
   if(DEBUG_ALGO7)
   { 
     fprintf(stderr,"** DEBUG liste de listes **\n");
     fprintf(stderr,"Queue: ");
     for(int k=0;k<X;k++){ fprintf(stderr," %d",queue[k]); } fprintf(stderr,"\n");

     for(int k=0;k<nbOfSubsets;k++)
     { 
      fprintf(stderr,"Liste %d:",k);
      int l=0;
      while(listOfLists[k][l]>=0){ fprintf(stderr," %d",listOfLists[k][l]); l++;}
      fprintf(stderr,"\n");
     }
   }
   
   //We determine the best subset 
   double bestScoreOFDMA=1000.0, scoreOFDMA;
   int allocation[MAX_ARRAY];
   int bestAllocation[MAX_ARRAY];
   int bestList, sizeList;

   for(int i=0; i<nbOfSubsets;i++)
   {
     int size=0; while(listOfLists[i][size]>=0) size++;
     if(DEBUG_ALGO6) fprintf(stderr,"Taille liste %d = %d\n",i,size);
     scoreOFDMA=computeBestAllocation(listOfLists[i], allocation, size);//C'est ici que cela deconne...
     if(scoreOFDMA<bestScoreOFDMA) { bestScoreOFDMA=scoreOFDMA; bestList=i; sizeList=size; for(int j=0;j<size;j++) bestAllocation[j]=allocation[j]; }
   }
    
   if(DEBUG_ALGO8) 
   {
    fprintf(stderr,"Best allocation found for OFDMA packet/allocation "); for(int i=0;i<sizeList;i++) fprintf(stderr,"%d/%d ",listOfLists[bestList][i],bestAllocation[i]); fprintf(stderr," score=%e\n",bestScoreOFDMA); 
    fprintf(stderr,"Best allocation found for AGG bestType=%d Nb of packets=%d ratio=%e\n",bestType,maxNbOfPackets,bestRatioAgg);
   }


   //Here we can change from priority to OFDMA or Agg in case of equality
   if(bestRatioAgg <= bestScoreOFDMA)  
   {
      //We update packetList
      int index=0; 
      for(int j=0;j<X;j++)
      { 
	  if(queue[j]==bestType)
	  {
		packetList[index]=j;
		index++;
	  }
      }
      
      nbOfServicesInProgress=maxNbOfPackets;
      double time=computeAggTransmissionTime(packetList, maxNbOfPackets);

      return(time);

   } else {
      for(int j=0;j<sizeList;j++) packetList[j]=listOfLists[bestList][j];
      nbOfServicesInProgress=sizeList;
      double time=computeOfdmaTransmissionTime(listOfLists[bestList], bestAllocation, sizeList);

      return(time);
   }
}




