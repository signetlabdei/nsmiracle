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


 #ifdef __cplusplus
 extern "C" {
 #endif

/** 
 * 
 * 
 * @param f frequency in kHz
 * 
 * @return 
 */
double uwlib_Thorp(double f);
double uwlib_AInv(double d, double k, double f);
double uwlib_Noise(double s, double w, double f);
double uwlib_BW(double d);
double uwlib_ierfc(double d);
/** 
 * 
 * 
 * @param d distance in km
 * 
 * @return central frequency in kHz
 */
double uwlib_f0(double d);

/** 
 * 
 * 
 * @param d distance in km
 * @param PtxdB power in dB re uPa
 * 
 * @return SNR in linear
 */
double uwlib_SNR(double d, double PtxdB);

/** 
 * 
 * 
 * @param d distance in km
 * @param PtxdB power in dB re uPa
 * @param Ldata data length in bits
 * @param RxPenaltydB power penalty in reception [db re uPa] 
 * @return packet error rate
 */
   double uwlib_PER(double d, double PtxdB, int Ldata, double RxPenaltydB);

/** 
 * 
 * 
 * @param d distance in km
 * @param PERtgt target packet error rate
 * @param Ldata data length in bits
 * @param RxPenaltydB power penalty in reception [db re uPa] 
 * @return TX power in uPa
 */
   double uwlib_Ptx_for_PER(double d, double PERtgt, int Ldata,  double RxPenaltydB);
 
/** 
 * 
 * 
 * @param d distance in km
 * @param f0 central frequency in kHz
 * @param b bandwith in kHz
 * @param PERtgt target packet error rate
 * @param Ldata data length in bits
 * @param RxPenaltydB power penalty in reception [db re uPa] 
 * @return TX power in uPa
 */
   double uwlib_Ptx_for_PER_2(double d, double f0, double b, double PERtgt, int Ldata,  double RxPenaltydB);   

/** 
 * 
 * 
 * @param freq_kHz central frequency in kHz
 * @param bwidth_kHz bandwidth in kHz
 * @param attLinear linear attenuation
 * @param PERtgt target packet error rate
 * @param Ldata data length in bits
 * @param RxPenaltydB power penalty in reception [db re uPa] 
 * @return TX power in uPa
 */
   double woss_uwlib_Ptx_for_PER(double freq_kHz, double bwidth_kHz, double attLinear, double PERtgt, int Ldata,  double RxPenaltydB );


  double uwlib_GammLn(double xx);

  double uwlib_Factorial(int n);

  double uwlib_FactLn(int n);

  double uwlib_Binomial(int n, int k);

   
 #ifdef __cplusplus
 }
 #endif
