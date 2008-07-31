
/*****************************************************************************/
/*                                                                           */
/*  Fichero:     nd.c                                                        */
/*  Autor:       Javier Minguez  -- Javier Osuna                             */
/*  Creado:      17/10/2002                                                  */
/*  Modificado:  24/06/2003                                                  */
/*                                                                           */
/*  $Id: nd.cc 4041 2007-04-30 21:09:41Z gerkey $                                                                     */
/*                                                                           */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nd.h"
#include "nd2.h"
//#include <stdlib.h>

// ----------------------------------------------------------------------------
// CONSTANTES.
// ----------------------------------------------------------------------------

#define DISTANCIA_INFINITO 1e6F

// ----------------------------------------------------------------------------
// VARIABLES.
// ----------------------------------------------------------------------------
FILE *depuracion;
int iteracion=0;

// Esta variable NO debe declararse como estática.
TInfoRobot robot;

static TVelocities velocidades; // Resultado de IterarND().

// ----------------------------------------------------------------------------
// FUNCIONES.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Operaciones con sectores.
// ----------------------------------------------------------------------------

#define INCREMENTAR_SECTOR(s) (((s)+1)%SECTORES)
#define DECREMENTAR_SECTOR(s) (((s)+(SECTORES-1))%SECTORES)

// Esta función NO debe declararse como estática.
float sector2angulo(int sector) {
  // Sector debe estar entre 0 y SECTORES-1.

  #define FACTOR (-(2.0F*PI)/SECTORES)
  #define SUMANDO (-SECTORES/2)

  return FACTOR*(sector+SUMANDO);

  #undef SUMANDO
  #undef FACTOR
}

int angulo2sector(float angulo) {
  // Angulo debe estar normalizado.
  
  #define FACTOR (-SECTORES/(2.0F*PI))
  #define SUMANDO ((SECTORES+1.0F)/2.0F)

   return ((int)(FACTOR*angulo+SUMANDO))%SECTORES;

  #undef SUMANDO
  #undef FACTOR
}

static int ObtenerSectorP(TCoordenadasPolares p) {

  #define FACTOR (-SECTORES/(2.0F*PI))
  #define SUMANDO ((SECTORES+1.0F)/2.0F)

  return ((int)(FACTOR*p.a+SUMANDO))%SECTORES;

  #undef SUMANDO
  #undef FACTOR
}

static int DistanciaSectorialOrientada(int s1,int s2) { // Distancia de s1 a s2.
  return (s1<=s2) ? s2-s1 : ((s2+SECTORES)-s1)%SECTORES;
}

// ----------------------------------------------------------------------------
// InicializarND y sus funciones auxiliares.
// ----------------------------------------------------------------------------

static void InicializarE(void) {
  // Calcula la distancia desde el origen (punto de coordenadas 0.0F,0.0F)
  // hasta el perímetro (que contiene el origen) en la dirección de la bisectriz
  // de cada sector.

  TCoordenadasPolares limite;
  int li,ld,i;

  limite.a=ARCOTANGENTE(robot.Dimensiones[0],robot.Dimensiones[1]);
  li=angulo2sector(limite.a);
  if (sector2angulo(li)>limite.a)
    li++;

  ConstruirCoordenadasPxy(&limite,robot.Dimensiones[2],robot.Dimensiones[1]);
  ld=angulo2sector(limite.a);
  if (sector2angulo(ld)>limite.a)
    ld++;

  robot.E[0]=-robot.Dimensiones[0];

  for (i=1; i<li; i++)
    robot.E[i]=robot.Dimensiones[0]/(float)cos(sector2angulo(i));

  for (i=li; i<ld; i++)
    robot.E[i]=robot.Dimensiones[1]/(float)sin(sector2angulo(i));

  for (i=ld; i<=SECTORES/2; i++)
    robot.E[i]=limite.r;

  for (i=SECTORES/2+1; i<SECTORES; i++)
    robot.E[i]=robot.E[SECTORES-i]; // Por simetria respecto del eje X.
}

static void InicializarERedondo(void) {
  // Calcula la distancia desde el origen (punto de coordenadas 0.0F,0.0F)
  // hasta el perímetro (que contiene el origen) en la dirección de la bisectriz
  // de cada sector.
  int i;
  for (i = 0; i<SECTORES; i++)
    robot.E[i]=robot.R;
}

static void InicializarDSRedondo(float dmax) {
  // Calcula la distancia desde el origen (punto de coordenadas 0.0F,0.0F)
  // hasta el perímetro (que contiene el origen) en la dirección de la bisectriz
  // de cada sector.
  int i;
  for (i = 0; i<SECTORES; i++)
    robot.ds[i]=dmax;
}

static void InicializarDS(float dsmax,float dsmin) {
  TCoordenadas p1,p2;
  TCoordenadas q1,q2,q3;
  TCoordenadasPolares q4;
  float limite1,limite2,limite3,limite4;
  float coseno,seno;
  float a,b,c,m,n;
  float angulo,distancia;
  int i;

  ConstruirCoordenadasCxy(&p1,robot.Dimensiones[0],robot.Dimensiones[1]);
  ConstruirCoordenadasCxy(&p2,robot.Dimensiones[2],robot.Dimensiones[1]);

  b=dsmax-dsmin;
  c=p2.x-p1.x;
  a=(float)sqrt(CUADRADO(c)-CUADRADO(b));
  coseno=a/c;
  seno=b/c;

  SumarCoordenadasCxyC(p1,-dsmin,0.0F,&q1);
  SumarCoordenadasCxyC(p1,-dsmin*seno,dsmin*coseno,&q2);
  SumarCoordenadasCxyC(p2,-dsmax*seno,dsmax*coseno,&q3);
  ConstruirCoordenadasPC(&q4,p2);
  q4.r+=dsmax;

  limite1=ARCOTANGENTE(q1.x,q1.y);
  limite2=ARCOTANGENTE(q2.x,q2.y);
  limite3=ARCOTANGENTE(q3.x,q3.y);
  limite4=q4.a;

  robot.ds[0]=-q1.x-robot.E[0]; // = q1.x/(float)cos(PI) - ...;

  m=CUADRADO(p1.x)+CUADRADO(p1.y)-CUADRADO(dsmin);
  n=CUADRADO(p2.x)+CUADRADO(p2.y)-CUADRADO(dsmax);

  b=q3.x-q2.x;
  c=q3.y-q2.y;
  a=b*q2.y-c*q2.x;

  for (i=1; i<SECTORES/2; i++) {
    angulo=sector2angulo(i);

    // Cálculo de la distancia de seguridad correspondiente a la bisectriz del sector i.

    if (angulo>=limite1)
     
      // r1
      distancia=q1.x/(float)cos(angulo);

    else if (angulo>=limite2) {
     
      // r2
      distancia=p1.x*(float)cos(angulo)+p1.y*(float)sin(angulo);
      distancia=distancia+(float)sqrt(CUADRADO(distancia)-m);

    } else if (angulo>=limite3)
    
      // r3
      distancia=a/(b*(float)sin(angulo)-c*(float)cos(angulo));

    else if (angulo>=limite4) {
      
      // r4
      distancia=p2.x*(float)cos(angulo)+p2.y*(float)sin(angulo);
      distancia=distancia+(float)sqrt(CUADRADO(distancia)-n);

    } else
     
      // r5
      distancia=q4.r;

    // Fin del cálculo de la distancia de seguridad correspondiente a la bisectriz del sector i.

    robot.ds[i]=distancia-robot.E[i];
    robot.ds[SECTORES-i]=robot.ds[i]; // El robot es simétrico respecto del eje X.
  }

//  robot.ds[SECTORES/2]=q4.x-robot.E[SECTORES/2]; // = q4.x/(float)cos(0.0F) - ...;
  robot.ds[SECTORES/2]=q4.r-robot.E[SECTORES/2]; // = q4.x/(float)cos(0.0F) - ...;
}

void InicializarND(TParametersND *parametros) {

  /* printf("geom %d\n",parametros->geometriaRect); */
  robot.geometriaRect = parametros->geometryRect;
  robot.holonomo=parametros->holonomic;

  if (parametros->geometryRect==1){
    // Cuadrado
    robot.Dimensiones[0]=-parametros->back;
    robot.Dimensiones[1]=parametros->left;
    robot.Dimensiones[2]=parametros->front;
    robot.Dimensiones[3]=-robot.Dimensiones[1];

    robot.enlarge=parametros->enlarge;

    InicializarE();
    InicializarDS(parametros->dsmax,parametros->dsmin);

  }
  else{
    // Redondo
    robot.R=parametros->R;
    InicializarERedondo();
    InicializarDSRedondo(parametros->dsmax);

  }


  robot.velocidad_lineal_maxima=parametros->vlmax;
  robot.velocidad_angular_maxima=parametros->vamax;

  robot.aceleracion_lineal_maxima=parametros->almax;

  robot.aceleracion_angular_maxima=parametros->aamax;

  robot.discontinuidad=parametros->discontinuity;

  robot.T=parametros->T;

  if (!robot.holonomo){
    robot.H[0][0]=(float)exp(-parametros->almax*parametros->T/parametros->vlmax);
    robot.H[0][1]=0.0F; // Se tiene en cuenta más adelante y no se incluye en las ecuaciones.
    robot.H[1][0]=0.0F; // Se tiene en cuenta más adelante y no se incluye en las ecuaciones.
    robot.H[1][1]=(float)exp(-parametros->aamax*parametros->T/parametros->vamax);
    
    robot.G[0][0]=(1.0F-(float)exp(-parametros->almax*parametros->T/parametros->vlmax))*(parametros->vlmax/parametros->almax);
    robot.G[0][1]=0.0F; // Se tiene en cuenta más adelante y no se incluye en las ecuaciones.
    robot.G[1][0]=0.0F; // Se tiene en cuenta más adelante y no se incluye en las ecuaciones.
    robot.G[1][1]=(1.0F-(float)exp(-parametros->aamax*parametros->T/parametros->vamax))*(parametros->vamax/parametros->almax /* Y no "aamax". */ );
  }
}

// ----------------------------------------------------------------------------
// IterarND y sus funciones auxiliares.
// ----------------------------------------------------------------------------

// IterarND / SectorizarMapa

static void SectorizarMapa(TInfoEntorno *mapa,TInfoND *nd) {
  TCoordenadas p;
  TCoordenadasPolares pp; // Módulos al cuadrado para evitar raíces innecesarias.
  int i,j;

  for (i=0; i<SECTORES; i++)
    nd->d[i].r=-1.0F;

  for (i=0; i<mapa->longitud; i++) {
    p=mapa->punto[i];
    TRANSFORMACION01(&(nd->SR1),&p)
    ConstruirCoordenadasPcC(&pp,p);

    j=ObtenerSectorP(pp);
    if ((nd->d[j].r<0.0F) || (pp.r<nd->d[j].r))
      nd->d[j]=pp;
  }

  for (i=0; i<SECTORES; i++)
    if (nd->d[i].r>=0.0F) {
      nd->d[i].r=RAIZ(nd->d[i].r);
      if ((i!=SECTORES/2) && (nd->d[i].r<robot.E[i]+0.01F))
        nd->d[i].r=robot.E[i]+0.01F;
    }
}

// ----------------------------------------------------------------------------

// IterarND / ParadaEmergencia

static int ParadaEmergencia(TInfoND *nd) {
  // Devuelve 1 si hay peligro de colisión y hay que hacer una parada de emergencia;
  // devuelve 0 en caso contrario.
  // En la detección de colisión se tiene en cuenta que el robot es simétrico respecto del eje X.

  TCoordenadas p;
  TCoordenadasPolares pp;
  int i;

  // Detecta si obstaculo en la parte delantera
  ConstruirCoordenadasCxy(&p,robot.Dimensiones[2],robot.Dimensiones[1]);
  ConstruirCoordenadasPC(&pp,p);

  for (i=angulo2sector(pp.a); i<=angulo2sector(-pp.a); i++)
    if ((nd->d[i].r>=0.0F) && (nd->d[i].r<=pp.r) && ((float)fabs(nd->d[i].a)<=pp.a))
     return 1;

  return 0;
}

// ----------------------------------------------------------------------------

// IterarND / SeleccionarRegiones / SiguienteDiscontinuidad

static void SiguienteDiscontinuidad(TInfoND *nd,int principio,int izquierda,int *discontinuidad,int *ascendente) {
  // Se busca desde "principio" en la dirección indicada por "izquierda".

  int i,j;
  float distancia_i,distancia_j;
  int no_obstaculo_i,no_obstaculo_j;

  j=principio;
  distancia_j=nd->d[j].r;
  no_obstaculo_j=(distancia_j<0.0F);

  do {
    i=j;
    distancia_i=distancia_j;
    no_obstaculo_i=no_obstaculo_j;

    j=(izquierda ? DECREMENTAR_SECTOR(i) : INCREMENTAR_SECTOR(i));
    distancia_j=nd->d[j].r;
    no_obstaculo_j=(distancia_j<=0.0F);

    if (no_obstaculo_i && no_obstaculo_j)
      continue;

    if (no_obstaculo_i || no_obstaculo_j) {
      *discontinuidad=i;
      *ascendente=no_obstaculo_i;
      return;
    }

    if ((float)fabs(distancia_i-distancia_j)>=robot.discontinuidad) {
      *discontinuidad=i;
      *ascendente=(distancia_i>distancia_j);
      return;
    }

  } while (j!=principio);

  *discontinuidad=-1;
}

// IterarND / SeleccionarRegiones / ObjetivoAlcanzable

static int ObjetivoAlcanzable(TInfoND *nd,TRegion *region,int direccion_tipo) {
  // "direccion_tipo" puede tomar los siguientes valores declarados en 'nd2.h':
  // - DIRECCION_OBJETIVO
  // - DIRECCION_DISCONTINUIDAD_INICIAL
  // - DIRECCION_DISCONTINUIDAD_FINAL

  TCoordenadas FL[SECTORES],FR[SECTORES];
  int nl,nr;

  TCoordenadasPolares objetivo_intermedio_polares; // Respecto de SR1.
  TCoordenadas objetivo_intermedio;                // Respecto de un SR con origen en el origen de SR1 y girado hasta que el semieje positivo de abscisas coincide con la dirección al objetivo intermedio.

  int sector_auxiliar;
  float limite;

  TCoordenadas p1,p2,p;
  int i,j;

  region->direccion_tipo=direccion_tipo;

  if (region->direccion_tipo==DIRECCION_OBJETIVO) {

    region->direccion_sector=nd->objetivo.s;
    objetivo_intermedio_polares=nd->objetivo.p1;

  } else {

    if (region->direccion_tipo==DIRECCION_DISCONTINUIDAD_INICIAL) {
      region->direccion_sector=region->principio;
      sector_auxiliar=DECREMENTAR_SECTOR(region->direccion_sector);
    } else {
      region->direccion_sector=region->final;
      sector_auxiliar=INCREMENTAR_SECTOR(region->direccion_sector);
    }

    if (nd->d[region->direccion_sector].r<0.0F)
      ConstruirCoordenadasPra(&objetivo_intermedio_polares,nd->d[sector_auxiliar].r+DISTANCIA_INFINITO,
          BisectrizAnguloNoOrientado(sector2angulo(region->direccion_sector),nd->d[sector_auxiliar].a));
    else {
      ConstruirCoordenadasCP(&p1,nd->d[region->direccion_sector]);
      ConstruirCoordenadasCP(&p2,nd->d[sector_auxiliar]);
      ConstruirCoordenadasPxy(&objetivo_intermedio_polares,(p1.x+p2.x)/2.0F,(p1.y+p2.y)/2.0F);
    }

  }

  region->direccion_angulo=objetivo_intermedio_polares.a;
  ConstruirCoordenadasCxy(&objetivo_intermedio,objetivo_intermedio_polares.r,0.0F);

  // Determinación de si el objetivo está dentro de un C-Obstáculo y
  // construcción de las listas de puntos FL y FR.

  limite=CUADRADO(robot.discontinuidad/2.0F); // Para no hacer raíces cuadradas dentro del bucle.
  nl=0;
  nr=0;
  for (i=0; i<SECTORES; i++) {
    if (nd->d[i].r<0.0F) // Si no existe un obstáculo en el sector actual, pasamos al siguiente sector.
      continue;

    ConstruirCoordenadasCra(&p,nd->d[i].r,nd->d[i].a-region->direccion_angulo);
    if ((p.x<0.0F) || (p.x>=objetivo_intermedio.x) || ((float)fabs(p.y)>robot.discontinuidad)) // Si el obstáculo no está en el rectángulo que consideramos, pasamos al siguiente sector.
      continue;

    if (DISTANCIA_CUADRADO2(p,objetivo_intermedio)<limite) // Si el objetivo intermedio está en colisión con el obstáculo, es inalcanzable.
      return 0; // Objetivo intermedio inalcanzable.

    if (p.y>0.0F)
      FL[nl++]=p;
    else
      FR[nr++]=p;
  }

  // Determinación de si los obstáculos nos impiden alcanzar el objetivo intermedio.

  limite=CUADRADO(robot.discontinuidad); // Para no hacer raíces cuadradas dentro de los bucles.
  for (i=0; i<nl; i++)
    for (j=0; j<nr; j++)
      if (DISTANCIA_CUADRADO2(FL[i],FR[j])<limite)
        return 0; // Objetivo intermedio inalcanzable.

  return 1; // Objetivo intermedio alcanzable.
}

// IterarND / SeleccionarRegion

static void SeleccionarRegion(TInfoND *nd) {

  #define IZQUIERDA VERDADERO
  #define DERECHA FALSO

  int objetivo_a_la_vista=(nd->d[nd->objetivo.s].r<0.0F) || (nd->objetivo.p1.r<=nd->d[nd->objetivo.s].r);
  TRegion *region,*region_izquierda,*region_derecha,*region_auxiliar;
  int indice,indice_izquierda,indice_derecha,indice_auxiliar;
  int distancia_izquierda,distancia_derecha;

  // Inicializamos el vector de regiones.
  
  nd->regiones.longitud=0;

  indice=nd->regiones.longitud++;
  region=&(nd->regiones.vector[indice]);
  region->descartada=FALSO;

  nd->region=-1;

  // Buscamos la primera discontinuidad.
  
  SiguienteDiscontinuidad(nd,nd->objetivo.s,IZQUIERDA,&(region->principio),&(region->principio_ascendente));
  if (region->principio==-1) {

    // No hay discontinuidades.

    region->principio=0;
    region->final=SECTORES-1;

    if (objetivo_a_la_vista) {
      region->direccion_tipo=DIRECCION_OBJETIVO;
      region->direccion_sector=nd->objetivo.s;
      region->direccion_angulo=nd->objetivo.p1.a;
      nd->region=indice;
      return;
    }

    // Objetivo inalcanzable.
    region->descartada=VERDADERO;
    return;
  }

  // Existe al menos una discontinuidad.

  SiguienteDiscontinuidad(nd,nd->objetivo.s,DERECHA,&(region->final),&(region->final_ascendente));
  if (region->final==DECREMENTAR_SECTOR(region->principio)) {

    // Hay una sola discontinuidad.

    if (objetivo_a_la_vista) {
      region->direccion_tipo=DIRECCION_OBJETIVO;
      region->direccion_sector=nd->objetivo.s;
      region->direccion_angulo=nd->objetivo.p1.a;
      nd->region=indice;
      return;
    }

    if (ObjetivoAlcanzable(nd,region,&(region->principio_ascendente) ? DIRECCION_DISCONTINUIDAD_INICIAL : DIRECCION_DISCONTINUIDAD_FINAL)) {
      nd->region=indice;
      return;
    }
    
    // Objetivo inalcanzable.
    region->descartada=VERDADERO;
    return;
  }

  // Hay dos o más discontinuidades.

  if (objetivo_a_la_vista) {

    // Región del objetivo.

    if (!region->principio_ascendente && !region->final_ascendente) {

      // Región artificial.

      indice_auxiliar=nd->regiones.longitud; // No incrementamos la longitud del vector: Nuestra región auxiliar es ilegal.
      region_auxiliar=&(nd->regiones.vector[indice_auxiliar]);

      *region_auxiliar=*region; // Utilizamos la región auxiliar para almacenar el contenido de la región que vamos a modificar.

      region->principio=nd->objetivo.s;
      region->final=nd->objetivo.s;

      if (ObjetivoAlcanzable(nd,region,DIRECCION_OBJETIVO)) {
        nd->region=indice;
        return;
      }

      nd->regiones.longitud++; // Regularizamos la situación de nuestra región auxiliar y
      indice=indice_auxiliar;  // la escogemos como región a examinar.
      region=region_auxiliar;

    } else if (ObjetivoAlcanzable(nd,region,DIRECCION_OBJETIVO)) {

      // Región "natural".

      nd->region=indice;
      return;
    }

  }

  indice_izquierda=indice;
  region_izquierda=region;

  indice_derecha=indice;
  region_derecha=region;

  do {

    distancia_izquierda=DistanciaSectorialOrientada(region_izquierda->principio,nd->objetivo.s);
    distancia_derecha=DistanciaSectorialOrientada(nd->objetivo.s,region_derecha->final);

    if (distancia_izquierda<=distancia_derecha) {
     
      // Probamos por la región izquierda.
      
      if (region_izquierda->principio_ascendente) {

        if (ObjetivoAlcanzable(nd,region_izquierda,DIRECCION_DISCONTINUIDAD_INICIAL)) {
          if (region_derecha->principio_ascendente || region_derecha->final_ascendente) {
            nd->region=indice_izquierda;
            return;
          }

          if (indice_derecha>indice_izquierda) {
            nd->regiones.longitud--;
            nd->region=indice_izquierda;
            return;
          }

          *region_derecha=*region_izquierda;
          nd->regiones.longitud--;
          nd->region=indice_derecha;
          return;
        }

        if (indice_izquierda!=indice_derecha)
          region_izquierda->descartada=VERDADERO;

        region_auxiliar=region_izquierda;

        indice_izquierda=nd->regiones.longitud++;
        region_izquierda=&(nd->regiones.vector[indice_izquierda]);
        region_izquierda->descartada=FALSO;

        region_izquierda->final=DECREMENTAR_SECTOR(region_auxiliar->principio);
        region_izquierda->final_ascendente=!region_auxiliar->principio_ascendente;

        SiguienteDiscontinuidad(nd,region_izquierda->final,IZQUIERDA,&(region_izquierda->principio),&(region_izquierda->principio_ascendente));

      } else { // Principio descendente: Será un final ascendente en la siguiente región izquierda.

        if (indice_izquierda!=indice_derecha) {

          region_izquierda->final=DECREMENTAR_SECTOR(region_izquierda->principio);
          region_izquierda->final_ascendente=!region_izquierda->principio_ascendente;

        } else {

          region_auxiliar=region_izquierda;

          indice_izquierda=nd->regiones.longitud++;
          region_izquierda=&(nd->regiones.vector[indice_izquierda]);
          region_izquierda->descartada=FALSO;

          region_izquierda->final=DECREMENTAR_SECTOR(region_auxiliar->principio);
          region_izquierda->final_ascendente=!region_auxiliar->principio_ascendente;

        }

        SiguienteDiscontinuidad(nd,region_izquierda->final,IZQUIERDA,&(region_izquierda->principio),&(region_izquierda->principio_ascendente));

        if (ObjetivoAlcanzable(nd,region_izquierda,DIRECCION_DISCONTINUIDAD_FINAL)) {
          if (region_derecha->principio_ascendente || region_derecha->final_ascendente) {
            nd->region=indice_izquierda;
            return;
          }

          if (indice_derecha>indice_izquierda) {
            nd->regiones.longitud--;
            nd->region=indice_izquierda;
            return;
          }

          *region_derecha=*region_izquierda;
          nd->regiones.longitud--;
          nd->region=indice_derecha;
          return;
        }

      }
       
    } else {

      // Probamos por la región derecha.

      if (region_derecha->final_ascendente) {

        if (ObjetivoAlcanzable(nd,region_derecha,DIRECCION_DISCONTINUIDAD_FINAL)) {
          if (region_izquierda->principio_ascendente || region_izquierda->final_ascendente) {
            nd->region=indice_derecha;
            return;
          }

          if (indice_izquierda>indice_derecha) {
            nd->regiones.longitud--;
            nd->region=indice_derecha;
            return;
          }

          *region_izquierda=*region_derecha;
          nd->regiones.longitud--;
          nd->region=indice_izquierda;
          return;
        }

        if (indice_derecha!=indice_izquierda)
          region_derecha->descartada=VERDADERO;

        region_auxiliar=region_derecha;

        indice_derecha=nd->regiones.longitud++;
        region_derecha=&(nd->regiones.vector[indice_derecha]);
        region_derecha->descartada=FALSO;

        region_derecha->principio=INCREMENTAR_SECTOR(region_auxiliar->final);
        region_derecha->principio_ascendente=!region_auxiliar->final_ascendente;

        SiguienteDiscontinuidad(nd,region_derecha->principio,DERECHA,&(region_derecha->final),&(region_derecha->final_ascendente));

      } else { // Final descendente: Será un principio ascendente en la siguiente región derecha.

        if (indice_derecha!=indice_izquierda) {

          region_derecha->principio=INCREMENTAR_SECTOR(region_derecha->final);
          region_derecha->principio_ascendente=!region_derecha->final_ascendente;

        } else {

          region_auxiliar=region_derecha;

          indice_derecha=nd->regiones.longitud++;
          region_derecha=&(nd->regiones.vector[indice_derecha]);
          region_derecha->descartada=FALSO;

          region_derecha->principio=INCREMENTAR_SECTOR(region_auxiliar->final);
          region_derecha->principio_ascendente=!region_auxiliar->final_ascendente;

        }

        SiguienteDiscontinuidad(nd,region_derecha->principio,DERECHA,&(region_derecha->final),&(region_derecha->final_ascendente));

        if (ObjetivoAlcanzable(nd,region_derecha,DIRECCION_DISCONTINUIDAD_INICIAL)) {
          if (region_izquierda->principio_ascendente || region_izquierda->final_ascendente) {
            nd->region=indice_derecha;
            return;
          }

          if (indice_izquierda>indice_derecha) {
            nd->regiones.longitud--;
            nd->region=indice_derecha;
            return;
          }

          *region_izquierda=*region_derecha;
          nd->regiones.longitud--;
          nd->region=indice_izquierda;
          return;
        }

      }

    }

  } while ((distancia_izquierda<SECTORES/2) || (distancia_derecha<SECTORES/2));

  // *region_izquierda == *region_derecha (al menos los campos que determinan la region) y son las dos últimas del vector.
  nd->regiones.longitud--;
  nd->regiones.vector[nd->regiones.longitud-1].descartada=VERDADERO;

  #undef IZQUIERDA
  #undef DERECHA
}

// ----------------------------------------------------------------------------

// IterarND / ConstruirDR

static void ConstruirDR(TInfoND *nd) {
  int i;

  for (i=0; i<SECTORES; i++)
    nd->dr[i]=(nd->d[i].r<0.0F) ? -1.0F : nd->d[i].r-robot.E[i];
}

// ----------------------------------------------------------------------------

// IterarND / control_angulo / ObtenerObstaculos / ActualizarMinimo

static void ActualizarMinimo(int *sector_minimo,float *valor_minimo,int sector,float valor) {
  if ((*sector_minimo==-1) || (valor<*valor_minimo)) {
    *sector_minimo=sector;
    *valor_minimo=valor;
  }
}

// IterarND / control_angulo / ObtenerObstaculos

static void ObtenerObstaculos(TInfoND *nd,float beta) {
  // Buscamos todos los obstáculos que estén dentro de la distancia de seguridad y nos quedamos
  // con el más cercano por la izquierda y el más cercano por la derecha.
  // El obstáculo más cercano es el de menor dr/ds.
  // Un obstáculo es por la izquierda si nos limita el giro a la izquierda (nos obliga a
  // rectificar el ángulo de partida hacia la derecha para esquivarlo).

  #define ACTUALIZAR_OBSTACULO_IZQUIERDA ActualizarMinimo(&(nd->obstaculo_izquierda),&min_izq,i,nd->dr[i]/robot.ds[i]);
  #define ACTUALIZAR_OBSTACULO_DERECHA ActualizarMinimo(&(nd->obstaculo_derecha),&min_der,i,nd->dr[i]/robot.ds[i]);

  TCoordenadas p;
  TCoordenadasPolares pp;
  float alfa,angulo,min_izq,min_der;
  int i;

  ConstruirCoordenadasCxy(&p,robot.Dimensiones[0],robot.Dimensiones[1]);
  ConstruirCoordenadasPcC(&pp,p);
  alfa=pp.a;

  nd->obstaculo_izquierda=-1;
  nd->obstaculo_derecha=-1;
  for (i=0; i<SECTORES; i++)
    if ((nd->dr[i]>=0.0F) && (nd->dr[i]<=robot.ds[i])) {
      angulo=nd->d[i].a;
/*       if (AnguloNormalizado(angulo-beta)>=0) */
      if (angulo>=beta)
        ACTUALIZAR_OBSTACULO_IZQUIERDA
      else
        ACTUALIZAR_OBSTACULO_DERECHA

	  }

  #undef ACTUALIZAR_OBSTACULO_IZQUIERDA
  #undef ACTUALIZAR_OBSTACULO_DERECHA
}

// IterarND / control_angulo / solHSGR

static float solHSGR(TInfoND *nd) {
  return nd->regiones.vector[nd->region].direccion_angulo;
}

// IterarND / control_angulo / solHSNR

static float solHSNR(TInfoND *nd) {
  TRegion *region=&(nd->regiones.vector[nd->region]);
  int final=region->final;
  if (region->principio>region->final)
    final+=SECTORES;

  return sector2angulo(((region->principio+final)/2)%SECTORES);
}

// IterarND / control_angulo / solHSWR

static float solHSWR(TInfoND *nd) {
  TRegion *region=&(nd->regiones.vector[nd->region]);

  if (region->direccion_tipo==DIRECCION_DISCONTINUIDAD_INICIAL)
    return(nd->d[DECREMENTAR_SECTOR(region->principio)].a
        -(float)atan2((robot.discontinuidad/2.0F+robot.ds[SECTORES/2]),nd->d[DECREMENTAR_SECTOR(region->principio)].r));
  else
    return(nd->d[INCREMENTAR_SECTOR(region->final)].a
        +(float)atan2((robot.discontinuidad/2.0F+robot.ds[SECTORES/2]),nd->d[INCREMENTAR_SECTOR(region->final)].r));
}

// IterarND / control_angulo / solLS1

static float solLS1(TInfoND *nd) {
  TRegion *region=&(nd->regiones.vector[nd->region]);
  //float angulo_objetivo=nd->regiones.vector[nd->region].direccion_angulo;
  float anguloPrueba;

  float angulo_parcial,dist_obs_dsegur,angulo_cota;
  int final=region->final;
  if (region->principio>region->final)
    final+=SECTORES;

  if (final - nd->regiones.vector[nd->region].principio > SECTORES/4) {
    if (region->direccion_tipo==DIRECCION_DISCONTINUIDAD_INICIAL)
      angulo_parcial=nd->d[DECREMENTAR_SECTOR(region->principio)].a
        -(float)atan2((robot.discontinuidad/2+robot.ds[SECTORES/2]),nd->d[DECREMENTAR_SECTOR(region->principio)].r);
    else
      angulo_parcial=nd->d[INCREMENTAR_SECTOR(region->final)].a
        +(float)atan2((robot.discontinuidad/2+robot.ds[SECTORES/2]),nd->d[INCREMENTAR_SECTOR(region->final)].r);
  } else
    angulo_parcial=sector2angulo(((region->principio+final)/2)%SECTORES);

  if (nd->obstaculo_izquierda!=-1) {
    angulo_cota=AnguloNormalizado(nd->d[nd->obstaculo_izquierda].a+PI-angulo_parcial)+angulo_parcial;
    dist_obs_dsegur=nd->dr[nd->obstaculo_izquierda]/robot.ds[nd->obstaculo_izquierda];
  } 
  else{
    angulo_cota=AnguloNormalizado(nd->d[nd->obstaculo_derecha].a+PI-angulo_parcial)+angulo_parcial;
    dist_obs_dsegur=nd->dr[nd->obstaculo_derecha]/robot.ds[nd->obstaculo_derecha];
  }

  // Codigo Osuna
  //return AnguloNormalizado(angulo_parcial * dist_obs_dsegur  + angulo_cota * (1-dist_obs_dsegur));

  // Codigo Minguez
  anguloPrueba=angulo_parcial * dist_obs_dsegur  + angulo_cota * (1-dist_obs_dsegur);

  if (anguloPrueba>M_PI)
    anguloPrueba=(float)(M_PI-0.01);
  else if (anguloPrueba<-M_PI)
    anguloPrueba=-(float)(M_PI+0.01);

  return AnguloNormalizado(anguloPrueba);

}

// IterarND / control_angulo / solLSG

static float solLSG(TInfoND *nd){

  float angulo_parcial,dist_obs_dsegur,angulo_cota,anguloPrueba;

  angulo_parcial= nd->regiones.vector[nd->region].direccion_angulo;
  
  if (nd->obstaculo_izquierda!=-1) {
    angulo_cota = AnguloNormalizado(nd->d[nd->obstaculo_izquierda].a+PI-angulo_parcial)+angulo_parcial;
    dist_obs_dsegur = nd->dr[nd->obstaculo_izquierda]/robot.ds[nd->obstaculo_izquierda];
  }
  else{
    angulo_cota = AnguloNormalizado(nd->d[nd->obstaculo_derecha].a+PI-angulo_parcial)+angulo_parcial;
    dist_obs_dsegur = nd->dr[nd->obstaculo_derecha]/robot.ds[nd->obstaculo_derecha];
  }

  // Codigo Osuna
  // return AnguloNormalizado(angulo_parcial * dist_obs_dsegur  + angulo_cota * (1-dist_obs_dsegur)); 

  anguloPrueba=angulo_parcial * dist_obs_dsegur  + angulo_cota * (1-dist_obs_dsegur);


  // Codigo Minguez
  if (anguloPrueba>M_PI)
    anguloPrueba=(float)(M_PI-0.01);
  else if (anguloPrueba<-M_PI)
    anguloPrueba=-(float)(M_PI+0.01);

  return AnguloNormalizado(anguloPrueba);
}


// IterarND / control_angulo / solLS2

static float solLS2(TInfoND *nd) {
  float ci = nd->dr[nd->obstaculo_izquierda]/robot.ds[nd->obstaculo_izquierda];
  float cd = nd->dr[nd->obstaculo_derecha]/robot.ds[nd->obstaculo_derecha];
  float ad,ai; // Ángulos cota izquierdo y derecho.
  float ang_par = nd->regiones.vector[nd->region].direccion_angulo;

/*
  ad = AnguloNormalizado(nd->d[nd->obstaculo_derecha].a+PI-ang_par)+ang_par;

  ai = AnguloNormalizado(nd->d[nd->obstaculo_izquierda].a-PI-ang_par)+ang_par;

  return AnguloNormalizado((ad+ai)/2.0F+(ci-cd)/(ci+cd)*(ad-ai)/2.0F);
*/
  ad=M_PI/2.0F;
  ai=-M_PI/2.0F;
  if (ci<=cd)
    return AnguloNormalizado(ang_par+(ci-cd)/(ci+cd)*(ang_par-ai));
  else
    return AnguloNormalizado(ang_par+(ci-cd)/(ci+cd)*(ad-ang_par));
}

// IterarND / control_angulo

static void control_angulo(TInfoND *nd) {
  // Cálculo del ángulo de movimiento en función de la región escogida para el movimiento del robot, la situación del objetivo y,
  // en su caso, la distancia a los obstáculos más próximos. 

  TRegion *region=&(nd->regiones.vector[nd->region]);
  int final=region->final;
  if (region->principio>region->final)
    final+=SECTORES;

  ObtenerObstaculos(nd,nd->regiones.vector[nd->region].direccion_angulo);

  if (nd->obstaculo_izquierda == -1 && nd->obstaculo_derecha == -1 ) {
    if (region->direccion_tipo==DIRECCION_OBJETIVO) {
      sprintf(nd->situacion,"HSGR");
      nd->angulosin= solHSGR(nd);
      nd->angulo=nd->angulosin;
    }
    else if (final - nd->regiones.vector[nd->region].principio > SECTORES/4) {
      sprintf(nd->situacion,"HSWR");
      nd->angulosin= solHSWR(nd);
      nd->angulo=nd->angulosin;
    }
    else {
      sprintf(nd->situacion,"HSNR");
      nd->angulosin= solHSNR(nd);
      nd->angulo=nd->angulosin;
    }
  }
  else {
    if ( nd->obstaculo_izquierda!=-1 && nd->obstaculo_derecha!=-1) {
      sprintf(nd->situacion,"LS2");
      nd->angulo=solLS2(nd);
      nd->angulosin=nd->angulo;
    }
    else if (region->direccion_tipo==DIRECCION_OBJETIVO ) {
      sprintf(nd->situacion,"LSG");
      nd->angulo=solLSG(nd);
      nd->angulosin=nd->angulo;
    }
    else {
      sprintf(nd->situacion,"LS1");
      nd->angulo=solLS1(nd);
      nd->angulosin=nd->angulo;
    }
  }

  AplicarCotas(&(nd->angulo),-PI/2.0F,PI/2.0F);
}

// ----------------------------------------------------------------------------

// IterarND / control_velocidad

static void control_velocidad(TInfoND *nd) {

  // Velocidad lineal del robot.

  float ci=(nd->obstaculo_izquierda!=-1) ? nd->dr[nd->obstaculo_izquierda]/robot.ds[nd->obstaculo_izquierda] : 1.0F; // Coeficiente de distancia por la izquierda.
  float cd=(nd->obstaculo_derecha!=-1) ? nd->dr[nd->obstaculo_derecha]/robot.ds[nd->obstaculo_derecha] : 1.0F; // Coeficiente de distancia por la derecha.

  nd->velocidad=robot.velocidad_lineal_maxima*MINIMO(ci,cd);
}

// ----------------------------------------------------------------------------

// Cutting / GenerarMovimientoFicticio

static void GenerarMovimientoFicticio(TInfoND *nd,float angulo,TVelocities *velocidades) {
  float ci=(nd->obstaculo_izquierda!=-1) ? nd->dr[nd->obstaculo_izquierda]/robot.ds[nd->obstaculo_izquierda] : 1.0F; // Coeficiente de distancia por la izquierda.
  float cd=(nd->obstaculo_derecha!=-1) ? nd->dr[nd->obstaculo_derecha]/robot.ds[nd->obstaculo_derecha] : 1.0F; // Coeficiente de distancia por la derecha.
  float cvmax=MAXIMO(0.2F,MINIMO(ci,cd));
  velocidades->v=robot.velocidad_lineal_maxima*cvmax*(float)cos(nd->angulo); // Calculada en SR2C.
  velocidades->w=robot.velocidad_angular_maxima*cvmax*(float)sin(nd->angulo); // Calculada en SR2C.



//  fprintf(depuracion,"%d: <a,ci,cd,cvmax,v,w>=<%f,%f,%f,%f,%f,%f>\n",++iteracion,nd->angulo,ci,cd,cvmax,velocidades->v,velocidades->w);
  AplicarCotas(&(velocidades->v),0.0F,robot.velocidad_lineal_maxima);
  AplicarCotas(&(velocidades->w),-robot.velocidad_angular_maxima,robot.velocidad_angular_maxima);
/*

  #define FMAX robot.aceleracion_lineal_maxima

  TCoordenadas F;

  ConstruirCoordenadasCra(&F,FMAX*nd->velocidad/robot.velocidad_lineal_maxima,angulo);

  velocidades->v=robot.H[0][0]*nd->velocidades.v+robot.G[0][0]*F.x;
  velocidades->w=robot.H[1][1]*nd->velocidades.w+robot.G[1][1]*F.y;

  #undef FMAX
*/
}

// GiroBrusco

static void GiroBrusco(TInfoND *nd,TVelocities *velocidades) {
  TCoordenadasPolares esquina;
  int derecha,izquierda;

  ConstruirCoordenadasPxy(&esquina,robot.Dimensiones[2],robot.Dimensiones[1]);
  derecha=(nd->obstaculo_derecha!=-1) && ((float)fabs(nd->d[nd->obstaculo_derecha].a)<=esquina.a) && (nd->d[nd->obstaculo_derecha].r<=esquina.r+robot.enlarge);
  izquierda=(nd->obstaculo_izquierda!=-1) && ((float)fabs(nd->d[nd->obstaculo_izquierda].a)<=esquina.a) && (nd->d[nd->obstaculo_izquierda].r<=esquina.r+robot.enlarge);

  if (derecha && izquierda) {
    velocidades->w=0.0F;
//    printf("giro brusco central\n");
    return;
  }

  if (derecha || izquierda) {
    velocidades->v=0.0F;
//    printf("giro brusco lateral\n");
  }
}

// Cutting / ObtenerSituacionCutting

#define CUTTING_NINGUNO   0
#define CUTTING_IZQUIERDA 1
#define CUTTING_DERECHA   2
#define CUTTING_AMBOS     3

static int ObtenerSituacionCutting(TInfoND *nd,float w) {
  TCoordenadas p;
  int resultado=CUTTING_NINGUNO;
  int obstaculo_izquierda=0;
  int obstaculo_derecha=0;
  int i;

  resultado=CUTTING_NINGUNO;

  i=0;
  while (i<SECTORES) {
    if (((nd->d[i].a<-PI/2.0F) || (nd->d[i].a>PI/2.0F)) && (nd->d[i].r>=0.0F) && (nd->dr[i]<=robot.enlarge/2.0F)) {

      ConstruirCoordenadasCP(&p,nd->d[i]);

      if (p.y>=robot.Dimensiones[1]) { // Obstáculo a la izquierda.
	
	if (obstaculo_derecha)
	  return CUTTING_AMBOS;
	
	obstaculo_izquierda=1;
	resultado=CUTTING_IZQUIERDA;

      } else if (p.y<=robot.Dimensiones[3]) { // Obstáculo a la derecha.
	
	if (obstaculo_izquierda)
	  return CUTTING_AMBOS;
	
	obstaculo_derecha=1;
	resultado=CUTTING_DERECHA;

      } else if (p.x<=robot.Dimensiones[0]) // Obstáculo detrás.
	return CUTTING_AMBOS;
    }
    
    i++;
    if (i==SECTORES/4+1)
      i=3*SECTORES/4;
  }

  return resultado;
}

// Cutting / AnguloSinRotacion

static float AnguloSinRotacion(TInfoND *nd,TVelocities *velocidades) {
  TCoordenadas F;
  float angulo;

  if (robot.aceleracion_angular_maxima*robot.T<fabs(nd->velocidades.w)){
    velocidades->w = (nd->velocidades.w>0) ? nd->velocidades.w-robot.aceleracion_angular_maxima*robot.T : nd->velocidades.w+robot.aceleracion_angular_maxima*robot.T;
    if (robot.aceleracion_lineal_maxima*robot.T<nd->velocidades.v)
      velocidades->v = nd->velocidades.v-robot.aceleracion_lineal_maxima*robot.T;
    else
      velocidades->v=0.0;
  }  
  else{
    velocidades->v=nd->velocidad;
    velocidades->w=0.0F;
  }
  
  F.x=(velocidades->v-robot.H[0][0]*nd->velocidades.v)/robot.G[0][0];
  F.y=(velocidades->w-robot.H[1][1]*nd->velocidades.w)/robot.G[1][1];
  angulo=ARCOTANGENTE(F.x,F.y);
  
  
  return angulo;
}

// Cutting

static void Cutting(TInfoND *nd, TVelocities *velocidades) {
  switch (ObtenerSituacionCutting(nd,velocidades->w)) {
    case CUTTING_NINGUNO:
      sprintf(nd->cutting,"NINGUNO");
      return;

    case CUTTING_IZQUIERDA:
      sprintf(nd->cutting,"IZQUIERDA");
      if (velocidades->w>=0.0F)
	return;
      break;

    case CUTTING_DERECHA:
      sprintf(nd->cutting,"DERECHA");
      if (velocidades->w<=0.0F)
	return;
      break;

    case CUTTING_AMBOS:
      sprintf(nd->cutting,"AMBOS");
  }

  nd->angulo=AnguloSinRotacion(nd,velocidades);
}

#undef CUTTING_NINGUNO
#undef CUTTING_IZQUIERDA
#undef CUTTING_DERECHA
#undef CUTTING_AMBOS

// ----------------------------------------------------------------------------

// IterarND / GenerarMovimiento

/* Unused
static void GenerarMovimiento(TInfoND *nd,TVelocities *velocidades) {
  #define FMAX robot.aceleracion_lineal_maxima

  TCoordenadas F;

  ConstruirCoordenadasCra(&F,FMAX*nd->velocidad/robot.velocidad_lineal_maxima,nd->angulo);

  velocidades->v=robot.H[0][0]*nd->velocidades.v+robot.G[0][0]*F.x;
  velocidades->w=robot.H[1][1]*nd->velocidades.w+robot.G[1][1]*F.y;

//   printf("v= %f \n w= %f \n",velocidades->v,velocidades->w);


  AplicarCotas(&(velocidades->v),0.0F,robot.velocidad_lineal_maxima);
  AplicarCotas(&(velocidades->w),-robot.velocidad_angular_maxima,robot.velocidad_angular_maxima);

  #undef FMAX
}
*/

// ----------------------------------------------------------------------------

// IterarND

TVelocities *IterarND(TCoordenadas objetivo,
                      float goal_tol,
                      TInfoMovimiento *movimiento,
                      TInfoEntorno *mapa,void *informacion) 
{

  // Devuelve NULL si se requiere una parada de emergencia o si no encuentra una región por la que hacer avanzar el robot.
  // Devuelve un puntero a (0.0F,0.0F) si se ha alcanzado el objetivo.

  TInfoND nd;

  // Valgrind says that some of the values in this nd structure are
  // uninitialized when it's accessed in ObtenerSituacionCutting(), so I'm
  // zeroing it here.  - BPG
  memset(&nd, 0, sizeof(TInfoND));

//depuracion=fopen("depuracion.txt","at");
  // Tratamiento de los parámetros "objetivo" y "movimiento".

  nd.objetivo.c0=objetivo;
  nd.SR1=movimiento->SR1;
  nd.velocidades=movimiento->velocidades;

  nd.objetivo.c1=nd.objetivo.c0;
  TRANSFORMACION01(&(nd.SR1),&(nd.objetivo.c1))

  ConstruirCoordenadasPC(&(nd.objetivo.p1),nd.objetivo.c1);

  nd.objetivo.s=ObtenerSectorP(nd.objetivo.p1);

  // Sectorización del mapa.

  SectorizarMapa(mapa,&nd);

  // Evaluación de la necesidad de una parada de emergencia.
  // Solo en el caso de robot rectangular
  if (robot.geometriaRect==1)
	  if (ParadaEmergencia(&nd)) {
		  printf("ND -> Parada Emergencia\n");
		  return 0;
	  }

  // Selección de la región por la cual avanzará el robot.

  SeleccionarRegion(&nd);
  if (nd.region<0) {
	  printf("ND -> No encuentra region\n");
	  return 0;
  }

  // Construcción de la distancia desde el perímetro del robot al obstáculo más cercano en cada sector.

  ConstruirDR(&nd);

  // Deteccion de fin de trayecto. -- Después de considerar la necesidad de una parada de emergencia.
  // Caso geometria rectangular
  if (robot.geometriaRect==1){
    // Replaced this check with the user-specified goal tolerance - BPG
    /*
    // Cuadrado
    if ((nd.objetivo.c1.x>=robot.Dimensiones[0]) && (nd.objetivo.c1.x<=robot.Dimensiones[2]) &&
	(nd.objetivo.c1.y>=robot.Dimensiones[3]) && (nd.objetivo.c1.y<=robot.Dimensiones[1])) { 
        */
    if(hypot(objetivo.x - movimiento->SR1.posicion.x,
             objetivo.y - movimiento->SR1.posicion.y) < goal_tol)
    {
      // Ya hemos llegado.
      velocidades.v=0.0F;
      velocidades.w=0.0F;
      return &velocidades;
    }
  }
  else if ( (CUADRADO(nd.objetivo.c1.x) + CUADRADO(nd.objetivo.c1.y))< CUADRADO(robot.R) ){
    // Redondo
    velocidades.v=0.0F;
    velocidades.w=0.0F;
    return &velocidades;
  }
  


  // Cálculo del movimiento del robot.
  control_angulo(&nd); // Obtención de la dirección de movimiento.
  control_velocidad(&nd); // Obtención de la velocidad de movimiento.
//  if (nd.velocidad<0.05F)
//    nd.velocidad=0.05F;
  nd.velocidad=robot.velocidad_lineal_maxima;
  // Hasta aqui es el ND standart


  // En funcion del tipo de robot.
  if (robot.holonomo){ // ya se han aplicado cotas al angulo
//    printf("Movimiento Holonomo\n");
//    velocidades.v= nd.velocidad*fabs(DistanciaAngular(fabs(nd.angulo),M_PI/2))/(M_PI/2);
    velocidades.v= nd.velocidad*(float)fabs(AmplitudAnguloNoOrientado((float)fabs(nd.angulo),M_PI/2))/(M_PI/2);
/*     velocidades.v= nd.velocidad; */
/*     velocidades.w=0.4; */
//    velocidades.w= (M_PI/2-DistanciaAngular(fabs(nd.angulo),M_PI/2))/(M_PI/2)*robot.velocidad_angular_maxima;
    velocidades.w= nd.angulo/(M_PI/2)*robot.velocidad_angular_maxima;
	velocidades.v_theta=nd.angulo;
/*    printf("w= %f dist=%f\n",velocidades.w,DistanciaAngular(fabs(nd.angulo),M_PI/2)); */
/*    if (nd.angulo<0) {
      velocidades.w=-velocidades.w; 
	  printf("vdsdñf\n");
	}
*/
	
  }
  else{
	  	velocidades.v_theta=0.0F;
//   printf("Movimiento No Holonomo\n");
    // Calculo del movimiento Generador de movimientos
//**/printf("<Vnd,And>=<%f,%f>\n",nd.velocidad,nd.angulo);
    GenerarMovimientoFicticio(&nd,nd.angulo,&velocidades);
//**/printf("<Vr,Wr>=<%f,%f>\n",velocidades.v,velocidades.w);
    
    // Aplicar correcciones al movimiento calculado.
    if (robot.geometriaRect==1){
      // Cuadrado
      GiroBrusco(&nd,&velocidades);
/*       printf("Entra en cutting %d\n",robot.geometriaRect); */
      Cutting(&nd,&velocidades); // Evitar colisión en zona posterior.
    }
  }

  AplicarCotas(&velocidades.v,
	       0.0F,
	       robot.velocidad_lineal_maxima);
  AplicarCotas(&velocidades.w,
	       -robot.velocidad_angular_maxima,
	       robot.velocidad_angular_maxima);

/*     printf("w = %f\n",velocidades.w);   */
  
  // Copia (si se requiere) de la información interna de ND para que quede accesible desde el exterior.
  
  if (informacion)
    *(TInfoND*)informacion=nd;
  
  // Devolución de resultados.

//fclose(depuracion);
  return &velocidades;
}
