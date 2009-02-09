/***************************************************/
/* Last Revised: 
$Id: calcul.c 4129 2007-08-21 23:16:24Z gerkey $
*/
/***************************************************/


#include "calcul.h"



void transfor_directa_p(float x, float y, 
			Tsc *sistema, Tpf *sol){

  /* Esta funcion transforma el punto x,y en el sistema de coordenadas mas global*/
  /* Es decir las coordenadas x y son vistas desde el sistema de coordenadas sistema*/
  /* Y se las quiere transformar en el sistema de ref desde el que se sistema*/
  /* Es la transformacion directa */

  float SinT,CosT;

  SinT=(float)sin(sistema->tita);
  CosT=(float)cos(sistema->tita);
 
  sol->x=x*CosT-y*SinT+sistema->x;
  sol->y=x*SinT+y*CosT+sistema->y;

  //fprintf(stderr,"input:<%f,%f> sis:<%f %f %f> sol:<%f %f>\n",x,y,sistema->x, sistema->y, sistema->tita,sol->x, sol->y);

}

void transfor_directa_pt0(float x, float y, 
			Tsc *sistema, Tpf *sol){

  /* Esta funcion transforma el punto x,y en el sistema de coordenadas mas global*/
  /* Es decir las coordenadas x y son vistas desde el sistema de coordenadas sistema*/
  /* Y se las quiere transformar en el sistema de ref desde el que se sistema*/
  /* Es la transformacion directa */

  sol->x=x+sistema->x;
  sol->y=y+sistema->y;

}


void transfor_inversa_p(float x,float y,
			Tsc *sistema, Tpf *sol){

  /* Esta funcion transforma el punto x,y en el sistema de coordenadas que entra*/
  /* Las coordenadas x y se ven desde el sistema de coordenadas desde el que se tienen las */
  /* las coordenadas de sistema */
  /* Es la transformacion directa */
 
  float a13, a23;
  float SinT,CosT;

  SinT=(float)sin(sistema->tita);
  CosT=(float)cos(sistema->tita);


  a13=-sistema->y*SinT-sistema->x*CosT;
  a23=-sistema->y*CosT+sistema->x*SinT;
    
  sol->x=x*CosT+y*SinT+a13;
  sol->y=-x*SinT+y*CosT+a23;
}  

float NormalizarPI(float ang){

  return (float)(ang+(2*M_PI)*floor((M_PI-ang)/(2*M_PI)));
}

void inversion_sis(Tsc *sisIn, Tsc *sisOut){

  float c,s;

  c=(float)cos(sisIn->tita);
  s=(float)sin(sisIn->tita);
  sisOut->x =-c*sisIn->x-s*sisIn->y;
  sisOut->y = s*sisIn->x-c*sisIn->y;
  sisOut->tita = NormalizarPI(-sisIn->tita);
}

void composicion_sis(Tsc *sis1,Tsc *sis2,Tsc *sisOut){

  Tpf sol;

  transfor_directa_p(sis2->x, sis2->y, 
		     sis1, &sol);
  sisOut->x=sol.x;
  sisOut->y=sol.y;
  sisOut->tita = NormalizarPI(sis1->tita+sis2->tita);
  
}

void car2pol(Tpf *in, Tpfp *out){
  
  out->r=(float)sqrt(in->x*in->x+in->y*in->y);
  out->t=(float)atan2(in->y,in->x);
}

void pol2car(Tpfp *in, Tpf *out){
  
  out->x=in->r*(float)cos(in->t);
  out->y=in->r*(float)sin(in->t);
}




int corte_segmentos(float x1,float y1,float x2,float y2,
		    float x3,float y3,float x4,float y4,
		    Tpf *sol){
/* corte de segmentos */
/* TE DEVUELVE EL PUNTO DE CORTE EN EL SISTEMA QUE ESTEN LOS SEGMENTOS */
 
  float a1,a2,b1,b2,c1,c2,xm,ym,denominador,max1_x,max1_y,min1_x,min1_y;
  float xerr,yerr;
  int si1;
  float error_redondeo;

  error_redondeo=(float)0.00001F;

  /* primera recta */
  a1=y2-y1;
  b1=x1-x2;
  c1=y1*(-b1)-x1*a1; 

  /* segunda recta */
  a2=y4-y3;
  b2=x3-x4;
  c2=y3*(-b2)-x3*a2;

  
  denominador=a1*b2-a2*b1;
  if (denominador==0)
    return 0;
  else{
    xm=(b1*c2-b2*c1)/denominador;
    ym=(c1*a2-c2*a1)/denominador;

    xerr=xm+error_redondeo;
    yerr=ym+error_redondeo;

    /* Comprobamos que cae entre los segmantos */
    if (x1>x2){
      max1_x=x1; min1_x=x2;
    }
    else{
      max1_x=x2; min1_x=x1;
    }
    if (y1>y2){
      max1_y=y1; min1_y=y2;
    }
    else{
      max1_y=y2; min1_y=y1;
    }
    si1=0;
    if (max1_x+error_redondeo>=xm && xerr>=min1_x &&  max1_y+error_redondeo>=ym && yerr>=min1_y)
      si1=1;
    

    if (si1){

      if (x3>x4){
	max1_x=x3; min1_x=x4;
      }
      else{
	max1_x=x4; min1_x=x3;
      }
      if (y3>y4){
	max1_y=y3; min1_y=y4;
      }
      else{
	max1_y=y4; min1_y=y3;
      }
      
      if (max1_x+error_redondeo>=xm && xerr>=min1_x &&  max1_y+error_redondeo>=ym && yerr>=min1_y){
	sol->x=xm;
	sol->y=ym;
	return 1;
      }
    }
    return 0;
  }
}

