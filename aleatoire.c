#include<math.h>
#include<stdlib.h>
#include<stdio.h>

#define NUMERICS_PI        3.14159265358979323846
#define NUMERICS_E         2.71828182845904523536
#define NUMERICS_EULER     0.5772156649
#define NUMERICS_ITMAX     100
#define NUMERICS_MAX_ERROR 5.0e-9
#define NUMERICS_FLOAT_MIN DBL_MIN
#define NUMERICS_FLOAT_MAX DBL_MAX


double uniforme(double a, double b)
{
	if(b<a) 
	{	
		fprintf(stderr,"Warning: in function uniform(), b must be greater than a\n"); 
	}

	return(1.0*(b-a)*rand()/(RAND_MAX+1.0) + a);
}

double gammln(double xx)
{
    double x, y, tmp, ser;
    static double cof[6]={76.18009172947146, -86.50532032941677,
        24.01409824083091, -1.231739572460166, 0.1208650973866179e-2,
        -0.5395239384953e-5};
    int j;

    if(xx <= 0.0){
		printf("gammln : Invalid xx value\n");
	}
    y = x = xx;
    tmp = x + 5.5;
    tmp -= (x + 0.5) * log(tmp);
    ser = 1.000000000190015;
    for(j = 0; j <= 5; j++) ser += cof[j] / ++y;
    return -tmp + log(2.5066282746310005 * ser / x);
}


int poisson(double xm)
{
    static double sq, alxm, g, oldm = -1.0;
    double em, t, y;
    
    if(xm < 12.0){
        if(xm != oldm){
            oldm = xm;
            g = exp(-xm);
        }
        em = -1;
        t = 1.0;
        do {
            ++em;
            t *= 1.0*rand()/(RAND_MAX+1.0);
        } while(t > g);
    } else {
        if(xm != oldm){
            oldm = xm;
            sq = sqrt(2.0 * xm);
            alxm = log(xm);
            g = xm * alxm - gammln(xm + 1.0);
        }
        do {
            do {
                y = tan(3.1415 * rand()/(RAND_MAX+1.0));
                em = sq * y + xm;
            } while (em < 0.0);
            em = floor(em);
            t = .9 * (1.0 + y*y) * exp(em *alxm-gammln(em+1.0)-g);
        } while(1.0*rand()/(RAND_MAX+1.0) > t);
    }
    
    return (int) em;
}

//genere un echantilon X d'une loi Gamma(m,beta)
//E[X]=m/beta et var(X)=m/(beta*beta)
//utilise uniquement la methode du rejet
double gammaX(double m, double beta) 
{ 
  double am,e,s,v1,v2,x,y,z; 
  

  if(m<0.0){
	fprintf(stderr,"Erreur sur m=%f dans gammaX\n",m);
	return(0.0);
  }

  if(m>1.0)
  {
   do { 
     do { 
	do { 
	  v1=2.0* 1.0*rand()/(RAND_MAX+1.0) -1.0; 
	  v2=2.0* 1.0*rand()/(RAND_MAX+1.0) -1.0; 
	} while (v1*v1+v2*v2 > 1.0); 
	y = v2/v1; 
	am=m-1.0; 
	s=sqrt(2.0*am+1.0); 
	x=s*y+am; 
      } while (x <= 0.0); 
      e=(1.0+y*y)*exp(am*log(x/am)-s*y); 
    } while (1.0*rand()/(RAND_MAX+1.0)  > e); 
  }//fin du if 

  if(m<=1.0){
	x=0.0;
  	do{
  		e=2.718281828;
    		z=1.0*rand()/(RAND_MAX+1.0); 
		if(z<(e/(m+e))) y=pow((m+e)*z/e,1.0/m);
		else y=-log((1.0-z)*(m+e)/(m*e));
    		v1=1.0*rand()/(RAND_MAX+1.0); 
		if((y>=0.0)&&(y<1.0)&&(v1<=exp(-y))) x=y;
		else if((y>1.0)&&(v1<=pow(y,m-1.0))) x=y;
	}while(x==0.0);
  }

  return (x/beta); 
} 

double normale(double m, double sigma)
{
	double x1, x2;


	x1 = 1.0 * rand()/(RAND_MAX+1.0);
	x2 = 1.0 * rand()/(RAND_MAX+1.0);

	if((x1==0.0)||(x2==0.0)||(x1==1.0)||(x2==1.0)){
		x1 = 1.0 * rand()/(RAND_MAX+1.0);
		x2 = 1.0 * rand()/(RAND_MAX+1.0);
	}

	return(m+sigma*sqrt(-2.0*log(x1))*cos(2.0*3.1415*x2));
}

//retourne un echantillon d'une logNormale
//Y suit une logNormale si ln(Y)=X avec X->Normale(m,sigma^2)
//les deux parametres de Y sont alors m=ln(M) et sigma^2=ln(k)
//E[Y]=M*sqrt(k) et var(Y)=M*M*k*(k-1)
double logNormale( double M, double k)
{
	return(exp(normale(log(M),sqrt(log(k)))));
}

//Attention de moyenne 1/mu
double exponentiel(double mu)
{
	double x;

	x = 1.0 * rand()/(RAND_MAX+1.0);


	return(-log(1.0-x)/mu);
}

double poissonPdf(double lambda, int k)
{
  if( lambda<0.0 || k<0 ) 
  {
	fprintf(stderr,"Error function poissonPdf() bad parameters (%e,%d)\n",lambda,k);
	exit(1);
  } 

  if(k==0) return(exp(-lambda));

  //k>0
  double tmp;
  tmp=1.0*k*log(lambda) - lambda;
  for(int i=2; i<=k ; i++) tmp-=log(1.0*i);

  return(exp(tmp));
}



