#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"header.h"
#include"simulator.h"
#include"aleatoire.h"
#include"testUnitaire.h"
#include"parserTrace.h"
#include"scheduling.h"

int MCS[MAX_ARRAY];

double lambda, OV, factorOnTimeTrace, lambdai[MAX_ARRAY], Di[MAX_ARRAY];
int nbOfSrc, trace=0;

void scenario(int sce, int algo)
{
  double current; 
  int nbOfPoints=0;
  double valueD2Sce1[15]={19.1,42.3,65.6,88.8,112.1,135.3,158.6,181.9,205.1,228.4,251.6,274.9,298.1,321.4,344.7};
  double valueD2Sce2[12]={960.0,480.0,320.0,240.0,160.0,120.0,106.7,96.0,80.0,72.0,64.0,57.6};

  //setParameters();

  if(sce==1)
  {
    //Scenario 1: the transmission time D2 increases / the other parameters are constant
    //Warning: OV, D1, D2 are expressed in microsec   
    nbOfSrc=2;
    lambdai[0]=500.0/1.0e6; lambdai[1]=100.0/1.0e6;
    MCS[0]=0; MCS[1]=1;
    OV=214.5; Di[0]=240; Di[1]=19.1; 
    fprintf(stderr,"main() DEBUG MCS[1]=%d MCS[0]=%d\n",MCS[1],MCS[0]);

    nbOfPoints=15;
    printf("#lambda1=%f lambda2=%f D1=%f OV=%f\n",lambdai[0],lambdai[1],Di[0],OV);
    printf("#D2 ");
  }

  if(sce==2)
  {
    //Scenario 2: the MCS of the second  destination changes 
    //Warning: OV, D1, D2 are expressed in microsec   
    nbOfSrc=2;
    lambdai[0]=500.0/1.0e6; lambdai[1]=100.0/1.0e6;
    OV=214.5; Di[0]=240.0; Di[1]=19.1; 
    MCS[0]=0; MCS[1]=1;

    nbOfPoints=12;
    printf("#lambda1=%f lambda2=%f D1=%f OV=%f\n",lambdai[0],lambdai[1],Di[0],OV);
    printf("#D2 ");
  }

  if( sce==3 )
  {
    //Scenario 3: the transmission time D2 increases / the other parameters are constant 
    //Warning: OV, D1, D2 are expressed in microsec   
    nbOfSrc=2;
    lambdai[0]=500.0/1.0e6; lambdai[1]=500.0/1.0e6;
    MCS[0]=0; MCS[1]=1;
    OV=214.5; Di[0]=240; Di[1]=19.1;

    nbOfPoints=15;
    printf("#lambda1=%f lambda2=%f D1=%f OV=%f\n",lambdai[0],lambdai[1],Di[0],OV);
    printf("#load1/load2 "); //(lambda1*D1)/(lambda*D2)
  }
  
  if( sce==4 )
  {
    //Scenario 4: lambda_2 increases / the other parameters are constant
    //Warning: OV, D1, D2 are expressed in microsec   
    nbOfSrc=2;
    lambdai[0]=300.0/1.0e6; 
    OV=214.5; Di[0]=960.0; Di[1]=960.0;
    MCS[0]=0; MCS[1]=1;

    nbOfPoints=15;
    printf("#lambda1=%f D1=%f D2=%f OV=%f\n",lambdai[0],Di[0],Di[1],OV);
    printf("#load1/load2 "); //(lambda1*D1)/(lambda*D2)
  }
  
  if( sce==5 )
  {
    //Scenario 5: lambda_2 increases / the other parameters are constant
    //Warning: OV, D1, D2 are expressed in microsec   
    nbOfSrc=2;
    lambdai[0]=30.0/1.0e6; 
    OV=214.5; Di[0]=960.0; Di[1]=960.0;
    MCS[0]=0; MCS[1]=0;//8.6Mbit/s

    nbOfPoints=35;
    printf("#lambda1=%f D1=%f D2=%f OV=%f\n",lambdai[0],Di[0],Di[1],OV);
    printf("#lambda1(frame/sec) "); //(lambda1*D1)/(lambda*D2)
  }

  if( sce==6 )
  {
    //Scenario 6: each source has the same parameters but the number of sources increases
    //Warning: OV, D1, D2 are expressed in microsec   
    OV=214.5; 

    for(int i=0; i<MAX_ARRAY;i++) 
    {
      lambdai[i]=15.0/1.0e5; 
      Di[i]=240.0; 
    }

    nbOfPoints=20; nbOfSrc=nbOfPoints;
    printf("#lambda_i=%f Di=%f OV=%f\n",lambdai[0],Di[0],OV);
    printf("#nbOfSrc "); 
  }

  if( sce==7 )//Real traces
  {
    OV=214.5; trace=1;

    nbOfPoints=20; nbOfSrc=0; //The number of src will be updated according to the trace file
    printf("#OV=%f\n",OV);
    printf("#factor_on_trace_time "); 
  }

  if( sce==8 )//Real traces - only one point
  {
    OV=214.5; trace=1;

    nbOfPoints=1; nbOfSrc=0; //The number of src will be updated according to the trace file
    printf("#OV=%f\n",OV);
    printf("#factor_on_trace_time "); 
  }

  printf(" meanW varW meanR varR meanServiceTime meanNbOfServices meanLoad meanTau ");
  for(int i=0;i<nbOfSrc;i++) printf("meanRiS[%d] meanRiS[%d]-D%d meanWi[%d] ",i,i,i,i); 
  printf(" meanMeanR varMeanR");
  printf("\n");


  for(int i=0;i<nbOfPoints;i++)
  {
    if(sce==1){ current=valueD2Sce1[i]; Di[1]=current; }
    if(sce==2){ current=valueD2Sce2[i]; Di[1]=current; }

    if(sce==3 ){ current=lambdai[1]*valueD2Sce1[i]/(lambdai[0]*Di[0]); Di[1]=current;}
    if(sce==4 ){ 
	    lambdai[1]=0.00005+1.0*i*0.00003; current=lambdai[1]*Di[1]/(lambdai[0]*Di[0]); 
	    //fprintf(stderr,"Simulation point %d/15 lambda2=%f load1=%f load2=%f load2/load1=%e current=%e\n",i+1,lambdai[1],lambdai[0]*Di[0],lambdai[1]*Di[1],lambdai[1]*Di[1]/(lambdai[0]*Di[0]),current);
    }
    if(sce==5 ){ 
	    lambdai[1]=0.00005+1.0*i*0.00006; current=lambdai[1]*1.0e6;
	    //fprintf(stderr,"Simulation point %d/15 lambda2=%f load1=%f load2=%f load2/load1=%e current=%e\n",i+1,lambdai[1],lambdai[0]*Di[0],lambdai[1]*Di[1],lambdai[1]*Di[1]/(lambdai[0]*Di[0]),current);
    }

    if(sce==6 ){ 
      	    MCS[i]=3;//34.4Mbit/s
	    nbOfSrc=i+1; current=1.0*nbOfSrc; 
    }

    if(sce==7 ){ 
	    //current=0.8+1.0*i*0.1; //Trace auditorium
	    current=0.17+1.0*i*0.028;  //Trace settopbox
	    factorOnTimeTrace=current;
    }

    if(sce==8 ){ 
	    current=1.0; 
	    factorOnTimeTrace=current;
    }

    //Update lambda
    lambda=lambdai[0];
    for(int j=1;j<nbOfSrc;j++) lambda+=lambdai[j];

    //Simulation
    printf("%e ",current); simulate(algo);
  }

}

/********* All times are in microsec ************/
/********* All times are in microsec ************/
/********* All times are in microsec ************/
/********* All times are in microsec ************/

int main (int argc, char* argv[])
{
  //Default value
  int sce=1, algo=0;

  fprintf(stderr,"DEBUG main 0\n");

  if(argc>1) sce=atoi(argv[1]);    	
  if(argc>2) algo=atoi(argv[2]);    	


  fprintf(stderr,"DEBUG main 1\n");
  scenario(sce,algo);



  /*** Test ***/
  //test_traceFile();
  //testUnitaireRealisticMaxPool();

  return(0);
}

