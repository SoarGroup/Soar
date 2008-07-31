/***************************************************/
/* Last Revised: 
$Id: TData.h 4129 2007-08-21 23:16:24Z gerkey $
*/
/***************************************************/

#ifndef TData
#define TData

#ifdef __cplusplus
extern "C" {
#endif

/* 
   Este fichero contiene los tipos de datos utilizados por todos 
*/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAXLASERPOINTS 361

#define RADIO 0.4F  /* Radio del robot */

typedef struct {
  float x;
  float y;
}Tpf;


typedef struct {
  float r;
  float t;
}Tpfp;

typedef struct {
  int x;
  int y;
}Tpi;

typedef struct {
  float x;
  float y;
  float tita;
}Tsc;

typedef struct {
  int numPuntos;
  Tpf laserC[MAXLASERPOINTS];  // Cartesian coordinates
  Tpfp laserP[MAXLASERPOINTS]; // Polar coordinates
}Tscan;




// Associations information
typedef struct{
  float rx,ry,nx,ny,dist;				// Point (nx,ny), static corr (rx,ry), dist 
  int numDyn;							// Number of dynamic associations
  float unknown;						// Unknown weight
  int index;							// Index within the original scan
  int L,R;
}TAsoc;

#ifdef __cplusplus
}
#endif

#endif
