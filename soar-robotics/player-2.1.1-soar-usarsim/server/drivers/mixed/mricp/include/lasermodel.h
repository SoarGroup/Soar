#ifndef LASERMODEL_H_
#define LASERMODEL_H_
// Info for a single range measurement
typedef struct
{
	double range, bearing;
}	laser_range_t;

class LaserModel
{
	private:
	  	mapgrid * * map; 	// Pointer to the OG map
	  	double range_cov;	// Covariance in the range reading
	  	double range_bad;	// Probability of spurious range readings
	  	// Pre-computed laser sensor model
	  	int lut_size;
	  	double  lut_res;
	  	double *lut_probs;
	  	int range_count;
	  	laser_range_t *ranges;
	public :
		void 	ClearRanges();
		void 	AddRange(double,double);
		void 	PreCompute();
		double 	RangeProb(double,double);
		double 	PoseProb();
				LaserModel();
		 		~LaserModel();
				LaserModel(mapgrid * * );
};
#endif /*LASERMODEL_H_*/
