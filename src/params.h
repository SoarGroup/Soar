#ifndef PARAMS_H
#define PARAMS_H

/* Initial lambda value for ridge regression */
#define RLAMBDA 1e-7

/* Maximum acceptable average squared error for a linear model */
#define MODEL_ERROR_THRESH 1e-8

/*
 A local model does not need to be refit to data if its training error
 increases by less than this factor with a new data point, assuming the
 error is less than MODEL_ERROR_THRESH
*/
#define REFIT_MUL_THRESH 1.0001

/*
 When solving linear systems, columns whose min and max values are within
 this factor of each other are removed.
*/
#define SAME_THRESH (1 + 1e-10)

/*
 When solving linear systems, elements whose absolute value is smaller
 than this are zeroed.
*/
#define ZERO_THRESH 1e-15

/*
 Maximum number of times Leave One Out cross-validation will run for a
 single PCR fitting
*/
#define LOO_NTEST 30

/*
 In PCR, don't use a beta vector with a norm larger than this.
*/
#define MAX_BETA_NORM 1.0e3

/* Standard deviation of a linear model's error */
#define MODEL_STD 0.001

/* Probability that any instance is just noise */
#define PNOISE 0.0001

/* What's this for? */
#define EPSILON 0.001

/*
 Number of noise instances required before EM tries to create a new
 model out of them
*/
#define K 10

/*
 Number of times EM tries to create a model from the same set of noise
 instances
*/
#define SEL_NOISE_MAX_TRIES 10

/*
 Number of noise instances to seed a new linear model with
*/
#define MODEL_INIT_N 5

#define MODEL_ADD_THRESH 1e-5

/*
 When trying to unify a new model with an existing one, the training
 error of the resulting model must be within this factor of the training
 error of the original
*/
#define UNIFY_MUL_THRESH 1.00001

/*
 In the controller, if the new prediction for the final state of a cached
 trajectory differs at least this much from the original prediction,
 then discard the cached trajectory, because it was computed using an
 outdated model.
*/
#define STATE_DIFF_THRESH 0.001

/*
 Number of initial rows to give dynamic matrices that hold model
 training data.
*/
#define INIT_NDATA 1000

#endif
