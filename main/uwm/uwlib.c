/** **************************************************************************
 **
 **
 ** This file may be used under the terms of the GNU General Public
 ** License version 2.0 as published by the Free Software Foundation.
 **
 **
 **  @author: Paolo Casari <casarip@dei.unipd.it>
 **
 **
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX(X,Y) ( X > Y ? X : Y ) 
#define MIN(X,Y) ( X > Y ? Y : X ) 

#include"uwlib.h"

double uwlib_GammLn(double xx);
double uwlib_Factorial(int n);
double uwlib_FactLn(int n);
double uwlib_Binomial(int n, int k);




/*************** CHANNEL MODEL AND PER ***************/
double uwlib_Thorp(double f)
{
	double f2 = pow(f,2);
	return (0.11*f2/(1.0+f2) + 44.0*f2/(4100.0+f2) +  2.75e-4*f2 + 0.003);
}

double uwlib_AInv(double d, double k, double f)
{
    // NB: what if dist < 1m??? 
  if (d > 0)    
    {
      // PC version
      double ainv = ( pow(10, -0.1*(d*uwlib_Thorp(f) +k*10*log10(1000*d)) ) );

      // NB version (BAD! because it is really k*10*log10(dist in meters)
      // I really hate all this mess in units (km, m, kHz, Hz, dB re uPa, W...)
      //double ainv = ( pow(10, -0.1*(d*uwlib_Thorp(f) +k*10*log10(d)) ) );

      if (ainv < 1.0) 
	return ainv;
      else 
	return 1.0;
    }
  else 
    return (1.0);  
}

double uwlib_Noise(double s, double w, double f)
{
	double	Nt = 17.0 - 30.0*log10(f),
		Ns = 40.0 + 20.0*(s-0.5) + 26.0*log10(f) - 60.0*log10(f+0.03),
		Nw = 50.0 + 7.5*sqrt(w) + 20.0*log10(f) - 40.0*log10(f+0.4),
		Nth = -15.0 + 20.0*log10(f);
	
	return ( pow(10,0.1*Nt) + pow(10,0.1*Ns) + pow(10,0.1*Nw) + pow(10,0.1*Nth) );
}


/*
BW[d_] := Piecewise[{
{58.83484936520060, 0 <= d <= 0.01},
{10^(0.1*(16.25959552233714 - 0.07183754681689*10 Log[10, d])), 
      0.01 < d <= 0.10049191769373},
{10^(0.1*(14.53478475606183 - 0.24468698876250*10 Log[10, d])), 
	   0.10049191769373 < d <= 0.59552868135092},
{10^(0.1*(13.94991560967719 - 0.50451644952389*10 Log[10, d])), 
	 d > 0.59552868135092}
}];
*/


double uwlib_BW(double d)
{
	if (d<0) {
		printf("BW: d < 0. FATAL!\n");
		return 0.0;
	}
	else if (d>=0 && d<=0.01)
		return 58.83484936520060;
	else if (d <= 0.10049191769373)
		return ( pow(10, 0.1*(16.25959552233714 - 0.07183754681689*10*log10(d))) );
	else if (d <= 0.59552868135092)
		return ( pow(10, 0.1*(14.53478475606183 - 0.24468698876250*10*log10(d))) );
	else
		return ( pow(10, 0.1*(13.94991560967719 - 0.50451644952389*10*log10(d))) );
	
}


/*
f0[d_] := Piecewise[{
{41.48133403684228, 0 <= d <= 0.01},
{10^(0.1*(15.38128486461887 - 0.03986211418015*10 Log[10, d])), 
      0.01 < d <= 0.14846215501071},
{10^(0.1*(13.49578267082813 - 0.26747414533695*10 Log[10, d])), 
	   0.14846215501071 < d <= 0.71183886240029},
{10^(0.1*(13.09081161692601 - 0.54181075428630*10 Log[10, d])), 
	 d > 0.71183886240029}
}];
*/
double uwlib_f0(double d)
{
	if (d<0) {
		printf("f0: d < 0. FATAL!\n");
		return 0.0;
	}
	else if (d>=0 && d<=0.01)
		return 41.48133403684228;
	else if (d <= 0.14846215501071)
		return ( pow(10, 0.1*(15.38128486461887 - 0.03986211418015*10*log10(d))) );
	else if (d <= 0.71183886240029)
		return ( pow(10, 0.1*(13.49578267082813 - 0.26747414533695*10*log10(d))) );
	else
		return ( pow(10, 0.1*(13.09081161692601 - 0.54181075428630*10*log10(d))) );
}


double uwlib_SNR(double d, double PtxdB)
{
	//printf("\nSNR: d=%g, PtxdB=%g, BW(d)=%g, f0(d)=%g, AInv(d,1.5,f0(d))=%g, Noise(0.5, 0, f0(d))=%g\n", d,PtxdB,BW(d),f0(d),AInv(d, 1.5, f0(d)),Noise(0.5,0,f0(d)));
	return ( pow(10, 0.1*PtxdB) / (1000.0*uwlib_BW(d))*uwlib_AInv(d,1.5,uwlib_f0(d))/uwlib_Noise(0.5,0,uwlib_f0(d)) ) ;
}


double uwlib_PER(double d, double PtxdB, int Ldata, double RxPenaltydB)
{
  double snr = uwlib_SNR(d, PtxdB)*pow(10, RxPenaltydB / 10.0);
  return ( 1.0 - pow(1.0 - 0.5*erfc( sqrt(snr)), Ldata) );
}
	
double uwlib_Ptx_for_PER(double d, double PERtgt, int Ldata, double RxPenaltydB)
{
  // Original version by PC
  //  	return ( 1000*uwlib_BW(d)*uwlib_Noise(0.5,0,uwlib_f0(d))/uwlib_AInv(d,1.5,uwlib_f0(d)) * pow(uwlib_ierfc(2.0-2.0*pow(1.0-PERtgt,1.0/Ldata)),2) );
  
  // Modified version by NB
  double snr =  pow(uwlib_ierfc(2.0-2.0*pow(1.0-PERtgt,1.0/Ldata)),2); // ideal SNR
  snr = snr / pow(10, RxPenaltydB / 10.0);  // account for receiver non-idealities
  double A = 1 / uwlib_AInv(d,1.5,uwlib_f0(d));
  double NB = uwlib_Noise(0.5,0,uwlib_f0(d)) * 1000*uwlib_BW(d);
  return ( NB * A * snr);
}


double uwlib_Ptx_for_PER_2(double d, double freq, double bwidth, double PERtgt, int Ldata, double RxPenaltydB)
{
  // Original version by PC
  //    return ( 1000*uwlib_BW(d)*uwlib_Noise(0.5,0,uwlib_f0(d))/uwlib_AInv(d,1.5,uwlib_f0(d)) * pow(uwlib_ierfc(2.0-2.0*pow(1.0-PERtgt,1.0/Ldata)),2) );
  
  // Modified version by NB
  double snr =  pow(uwlib_ierfc(2.0-2.0*pow(1.0-PERtgt,1.0/Ldata)),2); // ideal SNR
  snr = snr / pow(10, RxPenaltydB / 10.0);  // account for receiver non-idealities
  double A = 1 / uwlib_AInv(d,1.5,freq);
  double NB = uwlib_Noise(0.5,0,freq) * 1000.0*bwidth;
  return ( NB * A * snr);
}


double woss_uwlib_Ptx_for_PER(double freq, double bwidth, double attLinear, double PERtgt, int Ldata,  double RxPenaltydB)
{
  double snr = pow(uwlib_ierfc(2.0-2.0*pow(1.0-PERtgt,1.0/Ldata)),2.0); // ideal SNR
  snr = snr / pow(10.0, RxPenaltydB / 10.0);  // account for receiver non-idealities
  double NB = uwlib_Noise(0.5,0.0,freq)*1000.0*bwidth;
  
//   printf("freq:%f bwidth:%f att:%e per:%f Ldata:%d rxpen:%f Ptx:%e Ptxdb:%f \n", freq, bwidth, attLinear, PERtgt, Ldata, 
//                                                                                 RxPenaltydB, (NB * (1.0 / attLinear) * snr), 
//                                                                                 (10.0*log10(NB * (1.0 / attLinear) * snr)));

  return ( NB * (1.0 / attLinear) * snr);
}









/*************** UTILITIES ***************/

double uwlib_GammLn(double xx)
{
	double x,y,tmp,ser;
	static double cof[6] = {
		76.18009172947146, -86.50532032941677,
  24.01409824083091, -1.231739572450155,
  0.1208650973866179e-2, -0.5395239384953e-5
	};
	int j;
	
	y = x = xx;
	tmp = x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser = 1.000000000190015;
	for (j=0;j<=5;j++) 
		ser += cof[j]/++y;
	
	return ( -tmp+log(2.5066282746310005*ser/x) );
}

double uwlib_Factorial(int n)
{	
	static int ntop=10;
	static double a[161]={1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0, 5040.0, 40320.0, 362880.0, 3628800.0}; 
	int j;
			
	if (n < 0) {
		printf("uwlib_Factorial: negative factorial (%d)! requested! Exit...\n",n);
		exit(2);//return 0.0;
	}
	if (n > 32) return exp(uwlib_GammLn(n+1.0));
	while (ntop<n) {
		j = ntop++;
		a[ntop] = a[j]*ntop;
	}
	return a[n];
}

double uwlib_FactLn(int n)
{
	static double a[161];
	if (n < 0) {
		printf("uwlib_FactLn: negative factorial (%d)! requested! Exit...\n",n);
		exit(2);
	}
	if (n <= 1) return 0.0;
	if (n < 161) 
		return  ( a[n] ? a[n] : (a[n]=uwlib_GammLn(n+1.0)) );
	else
		return uwlib_GammLn(n+1.0);
}

double uwlib_Binomial(int n, int k)
{
	return floor(0.5+exp(uwlib_FactLn(n)-uwlib_FactLn(k)-uwlib_FactLn(n-k)));
}



double uwlib_ierfc(double y)
{
	double s, t, u, w, x, z;

	z = y;
	if (y > 1) {
		z = 2 - y;
	}
	w = 0.916461398268964 - log(z);
	u = sqrt(w);
	s = (log(u) + 0.488826640273108) / w;
	t = 1 / (u + 0.231729200323405);
	x = u * (1 - s * (s * 0.124610454613712 + 0.5)) -
		((((-0.0728846765585675 * t + 0.269999308670029) * t +
		0.150689047360223) * t + 0.116065025341614) * t +
		0.499999303439796) * t;
	t = 3.97886080735226 / (x + 3.97886080735226);
	u = t - 0.5;
	s = (((((((((0.00112648096188977922 * u +
			1.05739299623423047e-4) * u - 0.00351287146129100025) * u -
			7.71708358954120939e-4) * u + 0.00685649426074558612) * u +
			0.00339721910367775861) * u - 0.011274916933250487) * u -
			0.0118598117047771104) * u + 0.0142961988697898018) * u +
			0.0346494207789099922) * u + 0.00220995927012179067;
	s = ((((((((((((s * u - 0.0743424357241784861) * u -
		0.105872177941595488) * u + 0.0147297938331485121) * u +
		0.316847638520135944) * u + 0.713657635868730364) * u +
		1.05375024970847138) * u + 1.21448730779995237) * u +
		1.16374581931560831) * u + 0.956464974744799006) * u +
		0.686265948274097816) * u + 0.434397492331430115) * u +
		0.244044510593190935) * t -
		z * exp(x * x - 0.120782237635245222);
	x += s * (x * s + 1);
	if (y > 1) {
		x = -x;
	}
	return x;
}












