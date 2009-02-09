
/**************************************************************************
 * Desc: Simple particle filter for localization.
 * Author: Andrew Howard
 * Date: 10 Dec 2002
 * CVS: $Id: pf.c 3332 2006-01-20 02:44:25Z gerkey $
 *************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <libplayercore/error.h>

#include "pf.h"
#include "pf_pdf.h"
#include "pf_kdtree.h"


// Compute the required number of samples, given that there are k bins
// with samples in them.
static int pf_resample_limit(pf_t *pf, int k);

// Re-compute the cluster statistics for a sample set
static void pf_cluster_stats(pf_t *pf, pf_sample_set_t *set);


// Create a new filter
pf_t *pf_alloc(int min_samples, int max_samples)
{
  int i, j;
  pf_t *pf;
  pf_sample_set_t *set;
  pf_sample_t *sample;
  
  pf = calloc(1, sizeof(pf_t));

  pf->min_samples = min_samples;
  pf->max_samples = max_samples;

  // Control parameters for the population size calculation.  [err] is
  // the max error between the true distribution and the estimated
  // distribution.  [z] is the upper standard normal quantile for (1 -
  // p), where p is the probability that the error on the estimated
  // distrubition will be less than [err].
  pf->pop_err = 0.01;
  pf->pop_z = 3;
  
  pf->current_set = 0;
  for (j = 0; j < 2; j++)
  {
    set = pf->sets + j;
      
    set->sample_count = max_samples;
    set->samples = calloc(max_samples, sizeof(pf_sample_t));

    for (i = 0; i < set->sample_count; i++)
    {
      sample = set->samples + i;
      sample->pose.v[0] = 0.0;
      sample->pose.v[1] = 0.0;
      sample->pose.v[2] = 0.0;
      sample->weight = 1.0 / max_samples;
    }

    // HACK: is 3 times max_samples enough?
    set->kdtree = pf_kdtree_alloc(3 * max_samples);

    set->cluster_count = 0;
    set->cluster_max_count = 100;
    set->clusters = calloc(set->cluster_max_count, sizeof(pf_cluster_t));
  }

  return pf;
}


// Free an existing filter
void pf_free(pf_t *pf)
{
  int i;
  
  for (i = 0; i < 2; i++)
  {
    free(pf->sets[i].clusters);
    pf_kdtree_free(pf->sets[i].kdtree);
    free(pf->sets[i].samples);
  }
  free(pf);
  
  return;
}


// Initialize the filter using a guassian
void pf_init(pf_t *pf, pf_vector_t mean, pf_matrix_t cov)
{
  int i;
  pf_sample_set_t *set;
  pf_sample_t *sample;
  pf_pdf_gaussian_t *pdf;
  
  set = pf->sets + pf->current_set;
  
  // Create the kd tree for adaptive sampling
  pf_kdtree_clear(set->kdtree);

  set->sample_count = pf->max_samples;

  pdf = pf_pdf_gaussian_alloc(mean, cov);
    
  // Compute the new sample poses
  for (i = 0; i < set->sample_count; i++)
  {
    sample = set->samples + i;
    sample->weight = 1.0 / pf->max_samples;
    sample->pose = pf_pdf_gaussian_sample(pdf);

    // Add sample to histogram
    pf_kdtree_insert(set->kdtree, sample->pose, sample->weight);
  }

  pf_pdf_gaussian_free(pdf);
    
  // Re-compute cluster statistics
  pf_cluster_stats(pf, set);

  return;
}


// Initialize the filter using some model
void pf_init_model(pf_t *pf, pf_init_model_fn_t init_fn, void *init_data)
{
  int i;
  pf_sample_set_t *set;
  pf_sample_t *sample;

  set = pf->sets + pf->current_set;

  // Create the kd tree for adaptive sampling
  pf_kdtree_clear(set->kdtree);

  set->sample_count = pf->max_samples;

  // Compute the new sample poses
  for (i = 0; i < set->sample_count; i++)
  {
    sample = set->samples + i;
    sample->weight = 1.0 / pf->max_samples;
    sample->pose = (*init_fn) (init_data);

    // Add sample to histogram
    pf_kdtree_insert(set->kdtree, sample->pose, sample->weight);
  }

  // Re-compute cluster statistics
  pf_cluster_stats(pf, set);
  
  return;
}


// Update the filter with some new action
void pf_update_action(pf_t *pf, pf_action_model_fn_t action_fn, void *action_data)
{
  pf_sample_set_t *set;

  set = pf->sets + pf->current_set;

  (*action_fn) (action_data, set);
  
  return;
}


// Update the filter with some new sensor observation
void pf_update_sensor(pf_t *pf, pf_sensor_model_fn_t sensor_fn, void *sensor_data)
{
  int i;
  pf_sample_set_t *set;
  pf_sample_t *sample;
  double total;

  set = pf->sets + pf->current_set;

  // Compute the sample weights
  total = (*sensor_fn) (sensor_data, set);
  
  if (total > 0.0)
  {
    // Normalize weights
    for (i = 0; i < set->sample_count; i++)
    {
      sample = set->samples + i;
      sample->weight /= total;
    }
  }
  else
  {
    PLAYER_WARN("pdf has zero probability");

    // Handle zero total
    for (i = 0; i < set->sample_count; i++)
    {
      sample = set->samples + i;
      sample->weight = 1.0 / set->sample_count;
    }
  }
  
  return;
}


// Resample the distribution
void pf_update_resample(pf_t *pf)
{
  int i;
  double total;
  double *randlist;
  pf_sample_set_t *set_a, *set_b;
  pf_sample_t *sample_a, *sample_b;
  pf_pdf_discrete_t *pdf;

  set_a = pf->sets + pf->current_set;
  set_b = pf->sets + (pf->current_set + 1) % 2;

  // Create the discrete distribution to sample from
  total = 0;
  randlist = calloc(set_a->sample_count, sizeof(double));
  for (i = 0; i < set_a->sample_count; i++)
  {
    total += set_a->samples[i].weight;
    randlist[i] = set_a->samples[i].weight;
  }

  //printf("resample total %f\n", total);

  // Initialize the random number generator
  pdf = pf_pdf_discrete_alloc(set_a->sample_count, randlist);

  // Create the kd tree for adaptive sampling
  pf_kdtree_clear(set_b->kdtree);

  // Draw samples from set a to create set b.
  total = 0;
  set_b->sample_count = 0;
  while (set_b->sample_count < pf->max_samples)
  {
    i = pf_pdf_discrete_sample(pdf);    
    sample_a = set_a->samples + i;

    //printf("%d %f\n", i, sample_a->weight);
    assert(sample_a->weight > 0);

    // Add sample to list
    sample_b = set_b->samples + set_b->sample_count++;
    sample_b->pose = sample_a->pose;
    sample_b->weight = 1.0;
    total += sample_b->weight;

    // Add sample to histogram
    pf_kdtree_insert(set_b->kdtree, sample_b->pose, sample_b->weight);

    //fprintf(stderr, "resample %d %d %d\n", set_b->sample_count, set_b->kdtree->leaf_count,
    //        pf_resample_limit(pf, set_b->kdtree->leaf_count));

    // See if we have enough samples yet
    if (set_b->sample_count > pf_resample_limit(pf, set_b->kdtree->leaf_count))
      break;
  }

  //fprintf(stderr, "\n\n");

  pf_pdf_discrete_free(pdf);
  free(randlist);

  // Normalize weights
  for (i = 0; i < set_b->sample_count; i++)
  {
    sample_b = set_b->samples + i;
    sample_b->weight /= total;
  }

  // Re-compute cluster statistics
  pf_cluster_stats(pf, set_b);

  // Use the newly created sample set
  pf->current_set = (pf->current_set + 1) % 2;
  
  return;
}


// Compute the required number of samples, given that there are k bins
// with samples in them.  This is taken directly from Fox et al.
int pf_resample_limit(pf_t *pf, int k)
{
  double a, b, c, x;
  int n;

  if (k <= 1)
    return pf->min_samples;

  a = 1;
  b = 2 / (9 * ((double) k - 1));
  c = sqrt(2 / (9 * ((double) k - 1))) * pf->pop_z;
  x = a - b + c;

  n = (int) ceil((k - 1) / (2 * pf->pop_err) * x * x * x);

  if (n < pf->min_samples)
    return pf->min_samples;
  if (n >= pf->max_samples)
    return pf->max_samples;
  
  return n;
}


// Re-compute the cluster statistics for a sample set
void pf_cluster_stats(pf_t *pf, pf_sample_set_t *set)
{
  int i, j, k, c;
  pf_sample_t *sample;
  pf_cluster_t *cluster;

  // Cluster the samples
  pf_kdtree_cluster(set->kdtree);
  
  // Initialize cluster stats
  set->cluster_count = 0;

  for (i = 0; i < set->cluster_max_count; i++)
  {
    cluster = set->clusters + i;
    cluster->count = 0;
    cluster->weight = 0;
    cluster->mean = pf_vector_zero();
    cluster->cov = pf_matrix_zero();

    for (j = 0; j < 4; j++)
      cluster->m[j] = 0.0;
    for (j = 0; j < 2; j++)
      for (k = 0; k < 2; k++)
        cluster->c[j][k] = 0.0;
  }
  
  // Compute cluster stats
  for (i = 0; i < set->sample_count; i++)
  {
    sample = set->samples + i;

    //printf("%d %f %f %f\n", i, sample->pose.v[0], sample->pose.v[1], sample->pose.v[2]);

    // Get the cluster label for this sample
    c = pf_kdtree_get_cluster(set->kdtree, sample->pose);
    assert(c >= 0);
    if (c >= set->cluster_max_count)
      continue;
    if (c + 1 > set->cluster_count)
      set->cluster_count = c + 1;
    
    cluster = set->clusters + c;

    cluster->count += 1;
    cluster->weight += sample->weight;

    // Compute mean
    cluster->m[0] += sample->weight * sample->pose.v[0];
    cluster->m[1] += sample->weight * sample->pose.v[1];
    cluster->m[2] += sample->weight * cos(sample->pose.v[2]);
    cluster->m[3] += sample->weight * sin(sample->pose.v[2]);

    // Compute covariance in linear components
    for (j = 0; j < 2; j++)
      for (k = 0; k < 2; k++)
        cluster->c[j][k] += sample->weight * sample->pose.v[j] * sample->pose.v[k];
  }

  // Normalize
  for (i = 0; i < set->cluster_count; i++)
  {
    cluster = set->clusters + i;
        
    cluster->mean.v[0] = cluster->m[0] / cluster->weight;
    cluster->mean.v[1] = cluster->m[1] / cluster->weight;
    cluster->mean.v[2] = atan2(cluster->m[3], cluster->m[2]);

    cluster->cov = pf_matrix_zero();

    // Covariance in linear compontents
    for (j = 0; j < 2; j++)
      for (k = 0; k < 2; k++)
        cluster->cov.m[j][k] = cluster->c[j][k] / cluster->weight -
          cluster->mean.v[j] * cluster->mean.v[k];

    // Covariance in angular components; I think this is the correct
    // formula for circular statistics.
    cluster->cov.m[2][2] = -2 * log(sqrt(cluster->m[2] * cluster->m[2] +
                                         cluster->m[3] * cluster->m[3]));

    //printf("cluster %d %d %f (%f %f %f)\n", i, cluster->count, cluster->weight,
    //       cluster->mean.v[0], cluster->mean.v[1], cluster->mean.v[2]);
    //pf_matrix_fprintf(cluster->cov, stdout, "%e");
  }

  return;
}


// Compute the CEP statistics (mean and variance).
void pf_get_cep_stats(pf_t *pf, pf_vector_t *mean, double *var)
{
  int i;
  double mn, mx, my, mrr;
  pf_sample_set_t *set;
  pf_sample_t *sample;
  
  set = pf->sets + pf->current_set;

  mn = 0.0;
  mx = 0.0;
  my = 0.0;
  mrr = 0.0;
  
  for (i = 0; i < set->sample_count; i++)
  {
    sample = set->samples + i;

    mn += sample->weight;
    mx += sample->weight * sample->pose.v[0];
    my += sample->weight * sample->pose.v[1];
    mrr += sample->weight * sample->pose.v[0] * sample->pose.v[0];
    mrr += sample->weight * sample->pose.v[1] * sample->pose.v[1];
  }

  mean->v[0] = mx / mn;
  mean->v[1] = my / mn;
  mean->v[2] = 0.0;

  *var = mrr / mn - (mx * mx / (mn * mn) + my * my / (mn * mn));

  return;
}


// Get the statistics for a particular cluster.
int pf_get_cluster_stats(pf_t *pf, int clabel, double *weight,
                         pf_vector_t *mean, pf_matrix_t *cov)
{
  pf_sample_set_t *set;
  pf_cluster_t *cluster;

  set = pf->sets + pf->current_set;

  if (clabel >= set->cluster_count)
    return 0;
  cluster = set->clusters + clabel;

  *weight = cluster->weight;
  *mean = cluster->mean;
  *cov = cluster->cov;

  return 1;
}


