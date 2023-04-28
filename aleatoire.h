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


double erfc_inverse(double x);
double erf_inverse(double y);
double pdfNormale(double x,double  moyenne,double  variance);
double pdfLogNormale(double x,double  mu,double sigma2);
//Pdf d'une loi Gamma(m,beta) en x
double pdfGamma(double x, double m, double beta);
double uniforme(double a, double b);
double gammln(double xx);
int poisson(double xm);
double poissonPdf(double lambda, int k);
//genere un echantilon X d'une loi Gamma(m,beta)
//E[X]=m/beta et var(X)=m/(beta*beta)
double gammaX(double m, double beta);
double normale(double m, double sigma);
//retourne un echantillon d'une logNormale
//Y suit une logNormale si ln(Y)=X avec X->Normale(m,sigma^2)
//les deux parametres de Y sont alors m=ln(M) et sigma^2=ln(k)
//E[Y]=M*sqrt(k) et var(Y)=M*M*k*(k-1)
double logNormale( double M, double k);
//Attention de moyenne 1/mu
double exponentiel(double mu);

