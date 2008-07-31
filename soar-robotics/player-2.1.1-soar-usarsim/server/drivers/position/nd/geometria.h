
/*****************************************************************************/
/*                                                                           */
/*  Fichero:     geometria.h                                                 */
/*  Autor:       Javier C. Osuna Sanz                                        */
/*  Creado:      17/10/2002                                                  */
/*  Modificado:  11/07/2003                                                  */
/*                                                                           */
/*****************************************************************************/

#ifndef geometria_h
#define geometria_h

#include <math.h>
#include "nd.h"

/* ------------------------------------------------------------------------- */
/* Declaración de constantes y macros.                                       */
/* ------------------------------------------------------------------------- */

// Declaración de la constante PI.

#ifdef PI
#undef PI
#endif

#define PI 3.1415926535F
#ifndef M_PI
#define M_PI PI
#endif

// Declaración de operaciones básicas.

#define CUADRADO(x) (float)((x)*(x))
#define RAIZ(x) (float)sqrt(x)

#define ARCOTANGENTE(x,y) (float)atan2(y,x)
#define ARCOCOSENO(x,r) (float)acos((x)/(r))
#define ARCOSENO(y,r) (float)asin((y)/(r))

#define MINIMO(a,b) ( (a)<=(b) ? (a) : (b) )
#define MAXIMO(a,b) ( (a)>=(b) ? (a) : (b) )

/* ------------------------------------------------------------------------- */
/* Cotas.                                                                    */
/* ------------------------------------------------------------------------- */

extern void AplicarCotas(float *n,float i,float s);

/* ------------------------------------------------------------------------- */
/* Declaración de tipos y macros relacionadas.                               */
/* ------------------------------------------------------------------------- */

// Coordenadas cartesianas. Espacio real (y pantalla).

#define DISTANCIA_CUADRADO2(p,q) (((p).x-(q).x)*((p).x-(q).x)+((p).y-(q).y)*((p).y-(q).y))

// Coordenadas polares. Espacio real.

typedef struct {
  float r; // Radio.
  float a; // Ángulo.
} TCoordenadasPolares;

/* ------------------------------------------------------------------------- */
/* Construcción de coordenadas.                                              */
/* ------------------------------------------------------------------------- */

extern void ConstruirCoordenadasCP(TCoordenadas *p,TCoordenadasPolares q);
extern void ConstruirCoordenadasCxy(TCoordenadas *p,float x,float y);
extern void ConstruirCoordenadasCra(TCoordenadas *p,float r,float a);

extern void ConstruirCoordenadasPC(TCoordenadasPolares *p,TCoordenadas q);
extern void ConstruirCoordenadasPxy(TCoordenadasPolares *p,float x,float y);
extern void ConstruirCoordenadasPra(TCoordenadasPolares *p,float r,float a);

// Paso de cartesianas a polares, pero con el módulo al cuadrado.
extern void ConstruirCoordenadasPcC(TCoordenadasPolares *p,TCoordenadas q);

/* ------------------------------------------------------------------------- */
/* Suma y resta de coordenadas.                                              */
/* ------------------------------------------------------------------------- */

extern void SumarCoordenadasCxy(TCoordenadas *p,float x,float y);
extern void SumarCoordenadasCxyC(TCoordenadas p,float x,float y,TCoordenadas *q);
extern void SumarCoordenadasCra(TCoordenadas *p,float r,float a);
extern void SumarCoordenadasCraC(TCoordenadas p,float r,float a,TCoordenadas *q);

/* ------------------------------------------------------------------------- */
/* Transformaciones entre sistemas de coordenadas.                           */
/* ------------------------------------------------------------------------- */

// Transformaciones directas.

extern void TransformacionDirecta(TSR *SR,TCoordenadas *p);

#define TRANSFORMACION01(SR1,p) TransformacionDirecta(SR1,p);
#define TRANSFORMACION12(SR2,p) TransformacionDirecta(SR2,p);
#define TRANSFORMACION23(SR3,p) TransformacionDirecta(SR3,p);

#define TRANSFORMACION02(SR1,SR2,p) \
    { \
      TRANSFORMACION01(SR1,p) \
      TRANSFORMACION12(SR2,p) \
    }

// Transformaciones inversas.

extern void TransformacionInversa(TSR *SR,TCoordenadas *p);

#define TRANSFORMACION32(SR3,p) TransformacionInversa(SR3,p);
#define TRANSFORMACION21(SR2,p) TransformacionInversa(SR2,p);
#define TRANSFORMACION10(SR1,p) TransformacionInversa(SR1,p);

#define TRANSFORMACION20(SR2,SR1,p) \
    { \
      TRANSFORMACION21(SR2,p) \
      TRANSFORMACION10(SR1,p) \
    }

// Transformaciones mixtas.

#define TRANSFORMACION101(SR1a,SR1b,p) \
    { \
      TRANSFORMACION10(SR1a,p) \
      TRANSFORMACION01(SR1b,p) \
    }

/* ------------------------------------------------------------------------- */
/* Ángulos e intervalos de ángulos.                                          */
/* ------------------------------------------------------------------------- */

extern float AnguloNormalizado(float angulo);

extern int AnguloPerteneceIntervaloOrientadoCerrado(float angulo,float limite1,float limite2);
  // Esta función devuelve 1 si el ángulo está entre los límites; 0 en caso contrario.
  // Todos los parámetros deben pertenecer al intervalo (-PI,PI].

extern float BisectrizAnguloOrientado(float limite1,float limite2);
  // Devuelve la bisectriz del ángulo de "limite1" a "limite2" en sentido contrario a las agujas del reloj.

extern float BisectrizAnguloNoOrientado(float limite1,float limite2);
  // Devuelve la bisectriz del menor ángulo formado por "limite1" y "limite2", ya sea en el sentido de las agujas del reloj o en el opuesto.

extern float AmplitudAnguloOrientado(float limite1,float limite2);
  // Devuelve la amplitud del ángulo de "limite1" a "limite2" en sentido contrario a las agujas del reloj.

extern float AmplitudAnguloNoOrientado(float limite1,float limite2);
  // Devuelve la amplitud del menor ángulo formado por "limite1" y "limite2", ya sea en el sentido de las agujas del reloj o en el opuesto.

/* ------------------------------------------------------------------------- */
/* Cortes entre dos segmentos, uno de los cuales tiene como uno de sus       */
/* extremos el origen.                                                       */
/* ------------------------------------------------------------------------- */

void MinimaDistanciaCuadradoCorte(TCoordenadasPolares pp1,TCoordenadasPolares pp2,float angulo,float *distancia);
  // Mediante su aplicación reiterada obtenemos el más próximo de entre los puntos de corte de un
  // grupo de segmentos con una dirección determinada.
  // "p1" y "p2" son los extremos de un segmento.
  // "angulo" es la dirección de corte (desde el origen).
  // "distancia" es la menor distancia obtenida hasta el momento.

#endif //geometria_h
