#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "map.h"
#include "lasermodel.h"
#define LASER_MAX_RANGES 401
// Create an sensor model
LaserModel::LaserModel()
{
	//Empty Constructor
};
LaserModel::LaserModel(mapgrid * * map)
{
  	this->map = map;
  	this->range_cov = 0.10 * 0.10;
  	this->range_bad = 0.50;
  	PreCompute();  
  	this->range_count = 0;
  	this->ranges = (laser_range_t *)calloc(LASER_MAX_RANGES, sizeof(laser_range_t));
};
// Free an sensor model
LaserModel::~LaserModel()
{
	if(this->lut_probs)
  		free(this->lut_probs);
  	if(this->ranges)
  		free(this->ranges);
  	return;
}
// Clear all existing range readings
void LaserModel::ClearRanges()
{
  	this->range_count = 0;
  	return;
}
// Set the laser range readings that will be used.
void LaserModel::AddRange(double range, double bearing)
{
  	laser_range_t *beam;
  	assert(this->range_count < LASER_MAX_RANGES);
  	beam = this->ranges + this->range_count++;
  	beam->range = range;
  	beam->bearing = bearing;
  	return;
}
// Pre-compute the range sensor probabilities.
// We use a two-dimensional array over the O.G
void LaserModel::PreCompute()
{
  	double max;
  	double c, z, p;
  	double mrange, orange;
  	int i, j;
  	// Laser max range and resolution
  	max = 8.00;
  	this->lut_res = 0.01;
  	this->lut_size = (int) ceil(max / this->lut_res);
  	this->lut_probs = (double *)malloc(this->lut_size * this->lut_size * sizeof(this->lut_probs[0]));
  	for (i = 0; i < this->lut_size; i++)
  	{
    	mrange = i * this->lut_res;
   		for (j = 0; j < this->lut_size; j++)
    	{
      		orange = j * this->lut_res;
	      	// TODO: proper sensor model (using Kolmagorov?)
	      	// Simple gaussian model
	      	c = this->range_cov;
	      	z = orange - mrange;
	      	p = this->range_bad + (1 - this->range_bad) * exp(-(z * z) / (2 * c));
	      	//printf("%f %f %f\n", orange, mrange, p);
	      	//assert(p >= 0 && p <= 1.0);
	      	this->lut_probs[i + j * this->lut_size] = p;
    	}
    //printf("\n");
  	}
  	return;
}
// Determine the probability for the given range reading
double LaserModel::RangeProb(double obs_range, double map_range)
{
  	int i, j;
  	double p;
  	i = (int) (map_range / this->lut_res + 0.5);
  	j = (int) (obs_range / this->lut_res + 0.5);
  	assert(i >= 0);
  	if (i >= this->lut_size)
    	i = this->lut_size - 1;
  	assert(j >= 0);
  	if (j >= this->lut_size)
    	j = this->lut_size - 1;
  	p = this->lut_probs[i + j * this->lut_size];
  	//assert(p >= 0 && p <= 1.0);
  	return p;
}
// Determine the probability for the given pose
double LaserModel::PoseProb()
{
  	int i;
  	double p;
  	double map_range;
  	laser_range_t *obs;
  	p = 1.0; 
  	for (i = 0; i < this->range_count; i++)
  	{
    	obs = this->ranges + i;
    	map_range = 1; //TODO
    	if (obs->range >= 8.0 && map_range >= 8.0)
      		p *= 1.0;
    	else if (obs->range >= 8.0 && map_range < 8.0)
      		p *= this->range_bad;
    	else if (obs->range < 8.0 && map_range >= 8.0)
      		p *= this->range_bad;
   	 	else
      		p *= RangeProb(obs->range, map_range);
  	}
  //printf("%e\n", p);
  assert(p >= 0);
  return p;
}

