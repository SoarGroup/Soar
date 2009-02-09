
/*****************************************************************************/
/*                                                                           */
/*  Fichero:     nd2.h                                                       */
/*  Autor:       Javier Minguez                                              */
/*  Creado:      28/05/2003                                                  */
/*  Modificado:  21/06/2003                                                  */
/*                                                                           */
/*****************************************************************************/

#ifndef nd2_h
#define nd2_h

#include "geometria.h"

// ----------------------------------------------------------------------------
// CONSTANTES.
// ----------------------------------------------------------------------------

// Número de sectores: múltiplo de 4.
#define SECTORES 180

#define VERDADERO 1
#define FALSO 0
#define NO_SIGNIFICATIVO -1

// ----------------------------------------------------------------------------
// TIPOS.
// ----------------------------------------------------------------------------

// Información acerca del robot.

// Dimensiones del robot.
//   Consideramos el robot definido por un rectángulo. Numeramos sus
//   dimensiones, medidas a partir de su centro en las direcciones principales,
//   siguiendo la misma convención que para los sectores:
//     Dimension[0]: distancia desde el centro a la trasera del robot.
//     Dimension[1]: distancia desde el centro a la izquierda del robot.
//     Dimension[2]: distancia desde el centro al frontal del robot.
//     Dimension[3]: distancia desde el centro a la derecha del robot.
typedef float TDimensiones[4];

typedef float TMatriz2x2[2][2];

typedef struct {

  TDimensiones Dimensiones;
  float enlarge;

  short int geometriaRect; // Si es cuadrado o no

  float R; // radio del robot por si es circular

  short int holonomo;

  float E[SECTORES]; // Distancia desde el origen de SR2 al perímetro del robot.
  float ds[SECTORES];  // Distancia de seguridad: desde el perímetro del robot al perímetro de seguridad.

  float velocidad_lineal_maxima;
  float velocidad_angular_maxima;

  float aceleracion_lineal_maxima;
  float aceleracion_angular_maxima;

  float discontinuidad; // Espacio mínimo por el que cabe el robot.

  float T; // Período.

  TMatriz2x2 H; // Generador de movimientos: "Inercia" del robot.
  TMatriz2x2 G; // Generador de movimientos: "Fuerza" aplicada sobre el robot.

} TInfoRobot;

// Información acerca del objetivo.

typedef struct {
  TCoordenadas c0;
  TCoordenadas c1;
  TCoordenadasPolares p1;
  int s; // Sector.
} TObjetivo;

// Información acerca de la región escogida.

#define DIRECCION_OBJETIVO                0
#define DIRECCION_DISCONTINUIDAD_INICIAL  1
#define DIRECCION_DISCONTINUIDAD_FINAL    2

typedef struct {
  int principio;
  int final;

  int principio_ascendente;
  int final_ascendente;

  int descartada;

  int direccion_tipo;
  int direccion_sector;
  float direccion_angulo;
} TRegion;

typedef struct {
  int longitud;
  TRegion vector[SECTORES];
} TVRegiones;

// Información interna del método de navegación.

typedef struct {

  TObjetivo objetivo;

  TSR SR1;                  // Estado actual del robot: posición y orientación.
  TVelocities velocidades; // Estado actual del robot: velocidades lineal y angular.

  TCoordenadasPolares d[SECTORES]; // Distancia desde el centro del robot al obstáculo más próximo en cada sector (con ángulos).
  float dr[SECTORES]; // Distancia desde el perímetro del robot al obstáculo más próximo en cada sector.

  TVRegiones regiones; // Sólo como información de cara al exterior: Lista de todas las regiones encontradas en el proceso de selección.
  int region;          // Como almacenamos más de una región debemos indicar cuál es la escogida.

  int obstaculo_izquierda,obstaculo_derecha;

  float angulosin;    // Sólo como información de cara al exterior: Ángulo antes de tener en cuenta los obstáculos más próximos.
  float angulocon;    // Sólo como información de cara al exterior: Ángulo después de tener en cuenta los obstáculos más próximos.
  char situacion[20]; // Sólo como información de cara al exterior: Situación en la que se encuentra el robot.
  char cutting[20];   // Sólo como información de cara al exterior: Cutting aplicado al movimiento del robot.

  float angulo;    // Salida del algoritmo de navegación y entrada al generador de movimientos: dirección de movimiento deseada.
  float velocidad; // Salida del algoritmo de navegación y entrada al generador de movimientos: velocidad lineal deseada.

} TInfoND;

// ----------------------------------------------------------------------------
// VARIABLES.
// ----------------------------------------------------------------------------

extern TInfoRobot robot;

// ----------------------------------------------------------------------------
// FUNCIONES.
// ----------------------------------------------------------------------------

extern float sector2angulo(int sector);

extern int angulo2sector(float angulo);


#endif 
