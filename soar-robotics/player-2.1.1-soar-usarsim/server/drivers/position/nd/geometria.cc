
/*****************************************************************************/
/*                                                                           */
/*  Fichero:     geometria.c                                                 */
/*  Autor:       Javier C. Osuna Sanz                                        */
/*  Creado:      17/10/2002                                                  */
/*  Modificado:  02/07/2003                                                  */
/*                                                                           */
/*****************************************************************************/

#include "geometria.h"
#include "nd.h"

/* ------------------------------------------------------------------------- */
/* Cotas.                                                                    */
/* ------------------------------------------------------------------------- */

void AplicarCotas(float *n,float i,float s) {
  if (*n<i) {
    *n=i;
    return;
  }

  if (*n>s)
    *n=s;
}

/* ------------------------------------------------------------------------- */
/* Construcción de coordenadas.                                              */
/* ------------------------------------------------------------------------- */

void ConstruirCoordenadasCP(TCoordenadas *p,TCoordenadasPolares q) {
  p->x=q.r*(float)cos(q.a);
  p->y=q.r*(float)sin(q.a);
}

void ConstruirCoordenadasCxy(TCoordenadas *p,float x,float y) {
  p->x=x;
  p->y=y;
}

void ConstruirCoordenadasCra(TCoordenadas *p,float r,float a) {
  p->x=r*(float)cos(a);
  p->y=r*(float)sin(a);
}

void ConstruirCoordenadasPC(TCoordenadasPolares *p,TCoordenadas q) {
  p->r=(float)sqrt(q.x*q.x+q.y*q.y);
  p->a=(float)atan2(q.y,q.x);
}

void ConstruirCoordenadasPxy(TCoordenadasPolares *p,float x,float y) {
  p->r=(float)sqrt(x*x+y*y);
  p->a=(float)atan2(y,x);
}

void ConstruirCoordenadasPra(TCoordenadasPolares *p,float r,float a) {
  p->r=r;
  p->a=a;
}

// Paso de cartesianas a polares, pero con el módulo al cuadrado.
void ConstruirCoordenadasPcC(TCoordenadasPolares *p,TCoordenadas q) {
  p->r=q.x*q.x+q.y*q.y;
  p->a=(float)atan2(q.y,q.x);
}

/* ------------------------------------------------------------------------- */
/* Suma y resta de coordenadas.                                              */
/* ------------------------------------------------------------------------- */

void SumarCoordenadasCxy(TCoordenadas *p,float x,float y) {
  p->x+=x;
  p->y+=y;
}

void SumarCoordenadasCxyC(TCoordenadas p,float x,float y,TCoordenadas *q) {
  q->x=p.x+x;
  q->y=p.y+y;
}

void SumarCoordenadasCra(TCoordenadas *p,float r,float a) {
  p->x+=r*(float)cos(a);
  p->y+=r*(float)sin(a);
}

void SumarCoordenadasCraC(TCoordenadas p,float r,float a,TCoordenadas *q) {
  q->x=p.x+r*(float)cos(a);
  q->y=p.y+r*(float)sin(a);
}

/* ------------------------------------------------------------------------- */
/* Transformaciones entre sistemas de coordenadas.                           */
/* ------------------------------------------------------------------------- */

void TransformacionDirecta(TSR *SR,TCoordenadas *p) {
  float Dx=p->x-SR->posicion.x;
  float Dy=p->y-SR->posicion.y;

  float   seno=(float)sin(SR->orientacion);
  float coseno=(float)cos(SR->orientacion);

  p->x= Dx*coseno+Dy*  seno;
  p->y=-Dx*  seno+Dy*coseno;
}

void TransformacionInversa(TSR *SR,TCoordenadas *p) {
  float   seno=(float)sin(SR->orientacion);
  float coseno=(float)cos(SR->orientacion);

  float x=SR->posicion.x+p->x*coseno-p->y*  seno;
  p->y   =SR->posicion.y+p->x*  seno+p->y*coseno;
  p->x   =x;
}

/* ------------------------------------------------------------------------- */
/* Normalización de ángulos.                                                 */
/* ------------------------------------------------------------------------- */

float AnguloNormalizado(float angulo) {
  // Debe pertenecer a (-PI,PI].
  // Todos los ángulos que proceden de un "atan2" pertenecen a ese intervalo.

  return (float)atan2(sin(angulo),cos(angulo));
}

int AnguloPerteneceIntervaloOrientadoCerrado(float angulo,float limite1,float limite2) {
  // Intervalo orientado.
  // Esta función devuelve 1 si el ángulo está entre los límites; 0 en caso contrario.
  // Todos los parámetros deben pertenecer al intervalo (-PI,PI].
  // Si limite1==limite2, entonces el intervalo es de longitud 0.

  return (limite2>=limite1) ? ((angulo>=limite1) && (angulo<=limite2)) : ((angulo>=limite1) || (angulo<=limite2));
}

float BisectrizAnguloOrientado(float limite1,float limite2) {
  // Devuelve la bisectriz del ángulo de "limite1" a "limite2" en sentido contrario a las agujas del reloj.

  float resultado=(limite1+limite2)/2.0F;
  
  return (limite1<=limite2) ? resultado : AnguloNormalizado(resultado+PI);
}

float BisectrizAnguloNoOrientado(float limite1,float limite2) {
  // Devuelve la bisectriz del menor ángulo formado por "limite1" y "limite2", ya sea en el sentido de las agujas del reloj o en el opuesto.

  float resultado=(limite1+limite2)/2.0F;

  return ((float)fabs(limite1-limite2)<=PI) ? resultado : AnguloNormalizado(resultado+PI);
}

float AmplitudAnguloOrientado(float limite1,float limite2) {
  // Devuelve la amplitud del ángulo de "limite1" a "limite2" en sentido contrario a las agujas del reloj.

  float amplitud=limite2-limite1;

  return (limite1<=limite2) ? amplitud : 2.0F*PI-amplitud;
}

float AmplitudAnguloNoOrientado(float limite1,float limite2) {
  // Devuelve la amplitud del menor ángulo formado por "limite1" y "limite2", ya sea en el sentido de las agujas del reloj o en el opuesto.

  float amplitud=(float)fabs(limite1-limite2);

  return (amplitud<=PI) ? amplitud : 2.0F*PI-amplitud;
}

/* ------------------------------------------------------------------------- */
/* Cortes entre dos segmentos, uno de los cuales tiene como uno de sus       */
/* extremos el origen.                                                       */
/* ------------------------------------------------------------------------- */

void MinimaDistanciaCuadradoCorte(TCoordenadasPolares pp1,TCoordenadasPolares pp2,float angulo,float *distancia) {
  // Mediante su aplicación reiterada obtenemos el más próximo de entre los puntos de corte de un
  // grupo de segmentos con una dirección determinada.
  // "p1" y "p2" son los extremos de un segmento.
  // "angulo" es la dirección de corte (desde el origen).
  // "distancia" es la menor distancia obtenida hasta el momento.

  TCoordenadas p1,p2;
  float x;

  pp1.a=AnguloNormalizado(pp1.a-angulo);
  pp2.a=AnguloNormalizado(pp2.a-angulo);

  ConstruirCoordenadasCP(&p1,pp1);
  ConstruirCoordenadasCP(&p2,pp2);

  if ((p1.y*p2.y>0.0F) || ((p1.y==p2.y) && (p1.y!=0.0F))) // No hay punto de corte.
    return;

  if ((p1.y==0.0F) && (p2.y==0.0F) && (p1.x*p2.x<=0.0F)) {
    (*distancia)=0.0F;
    return;
  }

  if ((p1.y==0.0F) || (p2.y==0.0F)) {
    if ((p1.y==0.0F) && (p1.x>=0.0F) && (p1.x<=(*distancia)))
      (*distancia)=p1.x;

    if ((p2.y==0.0F) && (p2.x>=0.0F) && (p2.x<=(*distancia)))
      (*distancia)=p2.x;

    return;
  }

  x=(p1.x*p2.y-p2.x*p1.y)/(p2.y-p1.y);
  if ((x>=0.0F) && (x<(*distancia)))
    (*distancia)=x;
}
