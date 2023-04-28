#include<stdio.h>
#include<stdlib.h>
#include"header.h"
#include"scheduling.h"
#include"simulator.h"

extern double ofdmaRate[12][7]; 
extern double phyRate[12];
extern int MCS[MAX_ARRAY];//ICI 

extern int nbOfServicesInProgress, X, nbOfSrc, queue[MAX_ARRAY], packetSize[MAX_ARRAY];
extern double OV;




void testUnitaireRealisticMaxPool()
{
  OV=214.5;

  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): malloc OK\n"); 
  //We test with 7 packets to 4 diff destinations
  queue[0]=0; queue[1]=0; queue[2]=1; queue[3]=2; queue[4]=1; queue[5]=3; queue[6]=3;
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): queue OK\n"); 
  packetSize[0]=1000; packetSize[1]=500; packetSize[2]=1000; packetSize[3]=500; packetSize[4]=1000; packetSize[5]=500; packetSize[6]=1000; 
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): packetSize OK\n"); 
  nbOfSrc=4;
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): nbOfSrc OK\n"); 
  MCS[0]=0; MCS[1]=1; MCS[2]=2; MCS[3]=3; //MCS for each destination
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): MCS OK\n"); 
  X=7;
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): X=7 OK\n"); 

  //The possible OFDMA set (number of diff dest without change order) is packets 0-2-3-5
  double tpsDeService=setServicesRealisticMaxPool(queue);
  printf("Test unitaire - temps service: %e nbOfServicesInProgress=%d\n",tpsDeService,nbOfServicesInProgress);

  //Un seul paquet
  /*
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): malloc OK\n"); 
  //We test with 7 packets to 4 diff destinations
  queue[0]=0; 
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): queue OK\n"); 
  packetSize[0]=1000; 
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): packetSize OK\n"); 
  nbOfSrc=4;
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): nbOfSrc OK\n"); 
  MCS[0]=0; MCS[1]=1; MCS[2]=2; MCS[3]=3; //MCS for each destination
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): MCS OK\n"); 
  X=1;
  fprintf(stderr,"DEBUG testUnitaireRealisticMaxPool(): X=1 OK\n"); 

  //The possible OFDMA set (number of diff dest without change order) is packets 0-2-3-5
  double tpsDeService=setServicesRealisticMaxPool(queue);
  printf("Test unitaire - temps service: %e nbOfServicesInProgress=%d\n",tpsDeService,nbOfServicesInProgress);
  */

}

