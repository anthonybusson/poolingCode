#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include"header.h" 
#include"aleatoire.h" 
#include"scheduling.h" 
#include"simulator.h" 
#include"parserTrace.h" 

#define DEBUG_SIMULATOR 0
#define TRAJECTORY 	0

//Parameters
extern double OV, lambda;
extern double Di[MAX_ARRAY], lambdai[MAX_ARRAY]; 
extern int nbOfSrc; 
extern int MCS[MAX_ARRAY];//max MAX_ARRAY nbOfSrc
extern int trace;


//Simulator
double nextArrival, nextDeparture, time;
int X;
int* queue;
int* packetSize;
int nbOfServicesInProgress;
int counter=0;

//Use in traceParser.c and here
FILE* aFile;
int nbOfDestFound=0;
char listOfIpAddress[100][30];
float cumulTime=0.0;



//Stat (meanRiS = meanRi - OV-Di)
double meanW, varW, meanR, varR, meanRiS[MAX_ARRAY], meanWi[MAX_ARRAY], arrivalTime[MAX_ARRAY], meanServiceTime, meanNbOfServices, meanLoad;
int nbOfW, nbOfR, nbOfA, nbOfServices, nbOfRiS[MAX_ARRAY], nbOfWi[MAX_ARRAY];

//1 spatial stream - 800ns GI - 20MHz
double phyRate[12]={8.6, 17.2, 25.8, 34.4, 51.6, 68.8, 77.4, 86.0, 103.2, 114.7, 129.0, 143.4};

//1 spatial stream and GI=800microsec
//Line: MCS (from 0 to 11) 
//7 Column: RU-26 RU-52 RU-106 RU-242(20MHz) RU-484 (40MHz) RU-996 (80MHz) RU-2x996 (160MHz)
double ofdmaRate[12][7] = {
			{0.9,	1.8,	3.8,	8.6,	17.2,	36.0,	72.1},
			{1.8,	3.5,	7.5,	17.2,	34.4,	72.1,	144.1},
			{2.6,	5.3,	11.3,	25.8,	51.6,	108.1,	216.2},
			{3.5,	7.1,	15.0,	34.4,	68.8,	144.1,	288.2},
			{5.3,	10.6,	22.5,	51.6,	103.2,	216.2,	432.4},
			{7.1,	14.1,	30.0,	68.8,	137.6,	288.2,	576.5},
			{7.9,	15.9,	33.8,	77.4,	154.9,	324.3,	648.5},
			{8.8,	17.6,	37.5,	86.0,	172.1,	360.3,	720.6},
			{10.6,	21.2,	45.0,	103.2,	206.5,	432.4,	864.7},
			{11.8,	23.5,	50.0,	114.7, 	229.4, 	480.4, 	960.7},
			{-1.0,	-1.0,	-1.0,	129.0,	258.1, 	540.4,	1080.9},
			{-1.0,	-1.0,	-1.0,	143.4,	286.8,	600.4,	1201.0}
			};



void init()
{
  //Call a function that sets the parameters Di lambda etc.
  
  nextArrival=0.0;
  nextDeparture=INFINITE; 
  time=0.0;
  cumulTime=0.0;
  nbOfServicesInProgress=0;

  X=0;
  nbOfW=0; nbOfR=0; nbOfA=0;

  meanW=0.0; meanR=0.0; varR=0.0; varW=0.0; meanServiceTime=0.0; meanNbOfServices=0.0; meanLoad=0.0;
  nbOfServices=0;

  for(int i=0;i<nbOfSrc;i++) { meanRiS[i]=0.0; nbOfRiS[i]=0; meanWi[i]=0.0; nbOfWi[i]=0;}

  for(int i=0;i<MAX_ARRAY;i++) 
  {
    	arrivalTime[i]=0.0;
  }

  queue=(int *) malloc(MAX_ARRAY*sizeof(int));
  if(queue==NULL){ perror("Error malloc() in init()"); exit(2); }

  packetSize=(int *) malloc(MAX_ARRAY*sizeof(int));
  if(packetSize==NULL){ perror("Error malloc() in init()"); exit(2); }


  for(int i=0;i<nbOfSrc;i++) MCS[i]= rand()%12;

}

int isInList(int arg, int* list, int listSize)
{
  if(listSize<=0) return(0);	

  for(int i=0;i<listSize;i++)
  {
    if(list[i]==arg) return(1);
  }

  return(0);
}

int drawTypeOfClient()
{
  double u, tmp=0.0;

  u=uniforme(0.0,1.0);

  for(int i=0; i<nbOfSrc; i++)
  {
     tmp+=lambdai[i]/lambda;
     if(u<tmp) return(i);
  }

  if(tmp>1.0)
  {
          fprintf(stderr,"Error in drawTypeOfClientAggOFDMA(): bad value\n");
          exit(1);
  }
  return(-1);//Not called
}


void printQueue()
{
  fprintf(stderr,"Queue: ");
  for(int i=0; i<X;i++) fprintf(stderr,"%d ",queue[i]);
  fprintf(stderr,"\n");
}

//Move the packets in listIndex at the beginning of the queue. 
//The other keeps the same order in the remainder of the queue. 
//The same work is done for the arrivalTimeOpt and packetSize arrays.
void movePackets(int listIndex[], int size)
{
  int tmpQueue[MAX_ARRAY];
  double tmpArrival[MAX_ARRAY];
  double tmpPacketSize[MAX_ARRAY];

  if(DEBUG_SIMULATOR) printQueue();

  //First we copy the queue in another one
  for(int i=0;i<X;i++)
  {
      tmpQueue[i]=queue[i];	
      tmpArrival[i]=arrivalTime[i];	
      tmpPacketSize[i]=packetSize[i];	
      if(arrivalTime[i]<0.0) { fprintf(stderr,"Error movePackets(): arrivalTime[%d]=%e<0.0\n",i,arrivalTime[i]); exit(4);}
  }

  //The packets that are going to be served are moved in the head of the queue
  for(int i=0;i<size;i++)
  {
      queue[i]=tmpQueue[listIndex[i]];	
      arrivalTime[i]=tmpArrival[listIndex[i]];	
      packetSize[i]=tmpPacketSize[listIndex[i]];	
      if(arrivalTime[i]<0.0) { fprintf(stderr,"Error movePackets(): arrivalTime[]=%e<0.0 listIndex[%d]=%d tmp[]=%e\n",arrivalTime[i],i,listIndex[i],tmpArrival[listIndex[i]]); exit(4);}
  }
  if(DEBUG_SIMULATOR) { fprintf(stderr,"DEBUG-SIMULATOR movePackets(): arrivalTime[] after removing packets in front:"); for(int i=0;i<X;i++) fprintf(stderr," %e",arrivalTime[i]); fprintf(stderr,"\n");}

  //The other packets are pushed in the queue (they keep the same order)
  int j=0; 
  for(int i=0;i<X;i++)
  {
      //If the packet was not in the list of the next service it is pushed in the queue	  
      if(!isInList(i,listIndex,size))
      { 
	 queue[j+size]=tmpQueue[i];
	 arrivalTime[j+size]=tmpArrival[i];
	 packetSize[j+size]=tmpPacketSize[i];
	 j++;
      }
  }
}

void updateQueue()
{
   //nbOfServicesInprogress clients leave the queue: we shift the values in the arrays
   for(int i=nbOfServicesInProgress; i<X; i++)
   {
	queue[i-nbOfServicesInProgress]=queue[i];
	arrivalTime[i-nbOfServicesInProgress]=arrivalTime[i];
	packetSize[i-nbOfServicesInProgress]=packetSize[i];
   }
}


//return -1 if the buffer is full (> MAX_ARRAY)
//In this case the simulation stops and the point is not considered (see simulator() below). 
int updateSimulator(int algo)
{
   int type, packetLength; 

   if(DEBUG_SIMULATOR) fprintf(stderr,"DEBUG SIMULATOR: arrival  departure (%f  %f) \n",nextArrival, nextDeparture);

   //What is the next event?	
   if ( nextArrival < nextDeparture )    
   {
	time+=nextArrival;
	nextDeparture-=nextArrival; 

	if(DEBUG_SIMULATOR) fprintf(stderr,"DEBUG SIMULATOR: an arrival < departure (%f < %f) at time %f\n",nextArrival, nextDeparture,time);
	//Next arrival
	if(trace) 
	{
	  struct traceEntry myEntry; 
	  if(getNextFrameTrace(aFile, &myEntry)!=1) return(0); //end of the trace file
	  nextArrival=myEntry.time;
	  type=myEntry.dest;
	  if(type<0){ fprintf(stderr,"type is <0!!!!\n"); abort();}
	  packetLength=myEntry.size;
	  MCS[type]=myEntry.mcs; //The MCS for a same destination can change during the simulation according to the trace file
	}
	else 
	{
	  nextArrival=exponentiel(lambda);
	  type=drawTypeOfClient();   
	  //packetLength=64 + rand()%(1500-64);
	  packetLength=1000;
	}
					     //
	if(DEBUG_SIMULATOR) fprintf(stderr,"DEBUG SIMULATOR: nextArrival=%e\n",nextArrival);
	nbOfA++;

        //We update the arrival time for this client 
  	arrivalTime[X]=time;


	if(X==0)//The queue was empty before this arrival 
	{
	  X=1;	
	  nbOfServicesInProgress=1;//Only one client enters in the server 
	  queue[0]=type;
	  packetSize[0]=packetLength;//Used only for realistic OFDMA algorithm 
			      
	  if(algo<6) nextDeparture=OV+Di[type];
	  else nextDeparture=OV+(8.0*packetSize[0])/phyRate[MCS[type]]; //Only one packet

	  if(DEBUG_SIMULATOR) fprintf(stderr,"X=0 new arrival nextArrival=%e nextDeparture=%e\n",nextArrival,nextDeparture);
	  meanW+=0.0; 
	  nbOfW++;
	  meanWi[type]+=0.0;
	  nbOfWi[type]+=1;

	  meanServiceTime+=nextDeparture;
	  meanNbOfServices+=1.0;
	  nbOfServices++;
	  meanLoad+=1.0; //estimation of pi_A(0)

	} else {
	  packetSize[X]=packetLength;//Used only for realistic OFDMA algorithm 
	  X++;
	  if(X>=MAX_ARRAY)
	  {
		fprintf(stderr,"Warning: the number of customers (%d) is greater than the buffer size (%d) - we stop this simulation.\n",X,MAX_ARRAY);
		return(-1);//error
	  }	  

	  queue[X-1]=type;
	}
   } else {

	//Departure
	nextArrival-=nextDeparture;
	time+=nextDeparture;

	//For the client that leave the system, we compute the sojourn time 
	for(int i=0; i<nbOfServicesInProgress; i++)
	{
  	  double tmp=time-arrivalTime[i];
	  meanR+=tmp;
	  varR+=tmp*tmp;
	  if(DEBUG_SIMULATOR) fprintf(stderr,"DEBUG-SIMULATOR: a client leaves the system with R=%f time=%e arrivalTime[%d]=%e\n",tmp,time,i,arrivalTime[i]);

	  meanRiS[queue[i]]+=tmp;
	  nbOfRiS[queue[i]]++;

          //If we write the trajectory: the Ri for each packet 
	  if(TRAJECTORY) 
	  {
	    fprintf(stderr,"%d %e\n",counter,tmp);
	    counter++;
	  }
	}
	nbOfR+=nbOfServicesInProgress;

	//We remove the clients that leave the system 
	updateQueue(); 
	X-=nbOfServicesInProgress; 
	if(DEBUG_SIMULATOR) fprintf(stderr,"Depart nbOfServicesInProgress=%d\n",nbOfServicesInProgress);

	if(X<0) 
	{
		fprintf(stderr,"Error in updateSimulator(): X<0\n");
		exit(1);
	}


	//Next service
   	// - We set the clients that enter teh server
	// - We set the service time 

	if(X==0)
	{
	  nbOfServicesInProgress=0;
	  nextDeparture=INFINITE;
	} else {
	
	  //1. We set the clients that enter in the server	
	  //printQueue(); //DEBUG
	  int packetList[MAX_ARRAY];
	  double serviceTime;

	  switch(algo)
	  {
            case 0: setServicesFIFO(packetList);
		    break;
            case 1: setServicesFIFOpooling(packetList);
		    break;
            case 2: setServicesAgg(packetList);
		    break;
            case 3: setServicesOFDMA(packetList);
		    break;
            case 4: setServicesOpt(packetList);
		    break;
            case 5: setServicesFIFOMaxPool(packetList);
		    break;
	    case 6: serviceTime=setServicesRealisticMaxPool(packetList);
		    break;
	    case 7: serviceTime=setServicesRealisticFifoMaxPool(packetList);
		    break;
	    case 8: serviceTime=setServicesRealisticFifoPooling(packetList);
		    break;


	    default: fprintf(stderr,"Error in simulator: bad scheduling value. Abort.\n");
		     exit(1);
	  }

	  //2. We push the selected packets in the head of the queue
	  if(DEBUG_SIMULATOR){ fprintf(stderr,"Array arrivalTime[.] before movePackets X=%d for packets:",X); for(int i=0; i<nbOfServicesInProgress;i++) fprintf(stderr," %d",packetList[i]); fprintf(stderr,"\n"); for(int i=0;i<10;i++) fprintf(stderr," %f\n",arrivalTime[i]); }
          movePackets(packetList,nbOfServicesInProgress);
	  if(DEBUG_SIMULATOR){ fprintf(stderr,"Array arrivalTime[.] after movePackets X=%d: \n",X); for(int i=0;i<10;i++) fprintf(stderr," %f\n",arrivalTime[i]); }

	  //3. We update  nextDeparture and nbOfServicesInProgress
          nextDeparture=OV;
	  if(algo<6) { for(int j=0; j<nbOfServicesInProgress; j++) nextDeparture+=Di[queue[j]]; }
	  else {
	   nextDeparture+=serviceTime;
 	   if(DEBUG_SIMULATOR){ 
		   fprintf(stderr,"serviceTime=%e for %d packets - nextDeparture=%e (nbOfSrc=%d)\n",serviceTime,nbOfServicesInProgress,nextDeparture,nbOfSrc);
	   }
	  }

	  /*
	  printQueue();
	  fprintf(stderr,"**************\n");
	  */
	  meanServiceTime+=nextDeparture;
	  meanNbOfServices+=1.0*nbOfServicesInProgress;
	  nbOfServices++;


	  for(int i=0;i<nbOfServicesInProgress;i++) //Update and stat of the waiting time
	  { 
	    double tmp=time-arrivalTime[i]; 
	    if(arrivalTime[i]<0.0)
	    {
	      fprintf(stderr,"DEBUG-- time=%f arrivalTime for this packet=%f w=%e i=%d\n",time,arrivalTime[i],tmp,i);
	      exit(4);
	    }
     	    meanW+=tmp; varW+=tmp*tmp; nbOfW++; 
	    meanWi[queue[i]]+=tmp;
	    nbOfWi[queue[i]]+=1;
	  }
	}
   }
   return(1);
}

void simulate(int algo)
{

  int maxNbOfEvents=9000000, nbOfEvents=0, error=1; 
  //int maxNbOfEvents=100000, nbOfEvents=0, error=1; 
  srandom(5);

  //Simu
  init();

  //Simulation:
  nbOfEvents=0;

  if(trace)
  { 
    //aFile=openTrace("traces/auditorium.txt");
    aFile=openTrace("traces/settopbox.txt");
  }

  //error=-1 -> max buffer
  //error=0 -> we have reached the end of the trace file 
  //error=1 -> nothing particular
  while(nbOfEvents < maxNbOfEvents && error>0)
  {
    error=updateSimulator(algo);
    nbOfEvents++;
  }

  if(error>=0)
  {
    meanW=meanW/(1.0*nbOfW);
    varW=varW/(1.0*nbOfW)-meanW*meanW;
    meanR=meanR/(1.0*nbOfR);
    varR=varR/(1.0*nbOfR) -meanR*meanR;
    meanServiceTime=meanServiceTime/(1.0*nbOfServices);
    meanNbOfServices=meanNbOfServices/(1.0*nbOfServices);
    meanLoad=1.0-meanLoad/(1.0*nbOfA);

    double sumLambdaD=0.0;
    double varMeanRi=0.0;	//Variance of the mean Ri
    double meanMeanRi=0.0; 	//Mean of the mean Ri
    int nbOfValidRi=0;

    for(int i=0;i<nbOfSrc;i++)
    { 
	    meanRiS[i]=meanRiS[i]/(1.0*nbOfRiS[i]);
	    meanWi[i]=meanWi[i]/(1.0*nbOfWi[i]);
	    sumLambdaD+=lambdai[i]*Di[i];//to compute tau (see the paper)
	    if(DEBUG_SIMULATOR) fprintf(stderr,"DEBUG-SIMULATOR: meanRiS[%d]=%e meanWi[%d]=%e nbOfRiS[%d]=%d\n",i,meanRiS[i],i,meanWi[i],i,nbOfRiS[i]);
	    if(meanRiS[i]>=0.0) { varMeanRi+=meanRiS[i]*meanRiS[i]; meanMeanRi+=meanRiS[i]; nbOfValidRi++; }
    }
    meanMeanRi=meanMeanRi/(1.0*nbOfValidRi);
    varMeanRi=varMeanRi/(1.0*nbOfValidRi) - meanMeanRi*meanMeanRi;

    double mu, tau;
    mu=(meanLoad-sumLambdaD)/OV;
    tau=lambda/mu;

    //printf("#nbofDest=%d\n",nbOfSrc);

    /* Printing results */
    printf("%e %e %e %e %e %e %e %e ",meanW,varW,meanR,varR,meanServiceTime,meanNbOfServices,meanLoad,tau);
    for(int i=0;i<nbOfSrc;i++) printf("%e %e %e ", meanRiS[i], meanRiS[i]-Di[i], meanWi[i]); 
    printf("%e %e",meanMeanRi,varMeanRi); printf("\n");

  } else printf("\n");//empty lines in the output file

  free(queue);
  free(packetSize);
}


