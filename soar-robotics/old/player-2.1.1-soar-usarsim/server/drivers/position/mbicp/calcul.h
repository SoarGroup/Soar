/***************************************************/
/* Last Revised: 
$Id: calcul.h 4129 2007-08-21 23:16:24Z gerkey $
*/
/***************************************************/

#ifndef Calcul
#define Calcul

#include <stdio.h>
#include <math.h>
#include "TData.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
   Este fichero tiene operaciones de transformacion de sistemas de referencia, 
   transformaciones de puntos entre sistemas, de paso de coordenadadas polares,
   a cartesianas y de corte de segmentos

*/

/* --------------------------------------------------------------------------------------- */
/* TRANSFORMACIONES DE PUNTO DE UN SISTEMA DE REFERENCIA A OTRO                            */
/* --------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------- */
/* transfor_directa_p                                                                      */ 
/*  .... Hace la transformacion directa de un punto a un sistema a otro                    */ 
/*  .... In: (x,y) las coordenadas del punto, sistema es el sistema de referencia          */
/*  .... Out: en sol se devuelve las coordenadas del punto en el nuevo sistema             */

void transfor_directa_p ( float x, float y, Tsc *sistema, Tpf *sol );

/* --------------------------------------------------------------------------------------- */
/* transfor_directa_p                                                                      */ 
/*  .... Hace la transformacion directa de un punto a un sistema a otro                    */ 
/*  .... La diferencia es que aqui el punto de entrada es el (0,0) (optimiza la anterior)  */
/*  .... In: (x,y) las coordenadas del punto, sistema es el sistema de referencia          */
/*  .... Out: en sol se devuelve las coordenadas del punto en el nuevo sistema             */

void transfor_directa_pt0(float x, float y, 
			  Tsc *sistema, Tpf *sol);
  
/* --------------------------------------------------------------------------------------- */
/* transfor_inversa_p                                                                      */ 
/*  .... Hace la transformacion inversa de un punto a un sistema a otro                    */ 
/*  .... In: (x,y) las coordenadas del punto, sistema es el sistema de referencia          */
/*  .... Out: en sol se devuelve las coordenadas del punto en el nuevo sistema             */

void transfor_inversa_p ( float x, float y, Tsc *sistema, Tpf *sol );

/* --------------------------------------------------------------------------------------- */
/* TRANSFORMACIONES DE COMPOSICION E INVERSION DE SISTEMAS DE REFERENCIA                   */
/* --------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------- */
/* composicion_sis                                                                         */ 
/*  .... Realiza la composicion de sistemas de referencia en otro sistema                  */ 
/*  .... In: compone sis1 y sis2                                                           */
/*  .... Out: la salida sisOut es el resultado de la composicion de los sistemas           */
/*  .... Nota: resulta muy importante el orden de las entradas en la composicion           */

void composicion_sis(Tsc *sis1,Tsc *sis2,Tsc *sisOut);

/* --------------------------------------------------------------------------------------- */
/* inversion_sis                                                                           */ 
/*  .... Realiza la inversion de un sistema de referencia                                  */ 
/*  .... In: sisIn es el sistema a invertir                                                */
/*  .... Out: sisOut es el sistema invertido                                               */

void inversion_sis(Tsc *sisIn, Tsc *sisOut);

/* --------------------------------------------------------------------------------------- */
/* TRANSFORMACIONES DE PUNTO DE UN SISTEMA DE REFERENCIA A OTRO                            */
/* --------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------- */
/* car2pol                                                                                 */ 
/*  .... Transforma un punto de coordenadas cartesianas a polares                          */ 
/*  .... In: el punto en coordenadas cartesianas a transformar                             */
/*  .... Out: el punto salida en coordenadas polares                                       */

void car2pol(Tpf *in, Tpfp *out);

/* --------------------------------------------------------------------------------------- */
/* pol2car                                                                                 */ 
/*  .... Transforma un punto de coordenadas polares a cartesianas                          */ 
/*  .... In: el punto entrada en coordenadas polares a transformar                         */
/*  .... Out: el punto en coordenadas cartesianas transformado                             */

void pol2car(Tpfp *in, Tpf *out);

/* --------------------------------------------------------------------------------------- */
/* TRANSFORMACIONES DE PUNTO DE UN SISTEMA DE REFERENCIA A OTRO                            */
/* --------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------- */
/* corte_segmentos                                                                         */ 
/*  .... Calcula el punto de corte entre dos segmentos                                     */ 
/*  .... In: las coordenadas de los puntos extremos (x1,y1)-(x2,y2) y (x3,y3)-(x4,y4)      */
/*  .... Out: sol son las coordenadas del punto de corte. return --> 1 si hay corte. -->0 no */

int corte_segmentos ( float x1, float y1, float x2, float y2, 
		      float x3, float y3, float x4, float y4,
		      Tpf *sol );


/* Normaliza el angulo entre [-PI, PI] */
float NormalizarPI(float ang);

#ifdef __cplusplus
}
#endif

#endif 
