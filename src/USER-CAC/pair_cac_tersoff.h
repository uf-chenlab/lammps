/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifdef PAIR_CLASS

PairStyle(cac/tersoff,PairCACTersoff)

#else

#ifndef LMP_PAIR_TERSOFF_CAC_H
#define LMP_PAIR_TERSOFF_CAC_H

#include "pair_cac.h"

namespace LAMMPS_NS {

class PairCACTersoff : public PairCAC {
 public:
  PairCACTersoff(class LAMMPS *);
  virtual ~PairCACTersoff();

  void coeff(int, char **);
  virtual void init_style();
  virtual double init_one(int, int);

  static const int NPARAMS_PER_LINE = 17;

	struct Param {
    double lam1,lam2,lam3;
    double c,d,h;
    double gamma,powerm;
    double powern,beta;
    double biga,bigb,bigd,bigr;
    double cut,cutsq;
    double c1,c2,c3,c4;
    int ielement,jelement,kelement;
    int powermint;
    double Z_i,Z_j;              // added for TersoffZBL
    double ZBLcut,ZBLexpscale;
    double c5,ca1,ca4;           // added for TersoffMOD
    double powern_del;
    double c0;                   // added for TersoffMODC
  };

 protected:
	//potential params
  int nelements;                // # of unique elements
  char **elements;              // names of unique elements
  int ***elem2param;            // mapping from element triplets to parameters
  int *map;                     // mapping from atom types to elements
  int nparams;                  // # of stored parameter sets
  int maxparam;                 // max # of parameter sets
  Param *params;                // parameter set for an I-J-K interaction
  double cutmax;                // max cutoff for all elements
  int **cluster_neighbors;      // stores neighbors of neighbors within cutshort for the given quadrature point
  int *cluster_neighbor_counts; // number of neighbors withing cutshort for each neighbor within cutshort
  int flux_max;                     // array storage maximum for additional cluster neighbor array for flux calculation
  int add_ncluster;                 // number of additional sites to store neighbors around for the flux calculation
  int **add_cluster_neighbors;      // stores neighbors of neighbors for flux calculation around the quadrature point
  int *add_cluster_neighbor_counts; // stores neighbors of neighbors counts for flux calculation around the quadrature point
  int origin_type;
 
  void allocate();
  void read_file(char *);
  virtual void setup_params();
  virtual void repulsive(Param *, double, double &, int, double &);
  virtual double zeta(Param *, double, double, double *, double *);
  virtual void force_zeta(Param *, double, double, double &,
                          double &, int, double &);
  void attractive(Param *, double, double, double, double *, double *,
                  double *, double *, double *);

  virtual double ters_fc(double, Param *);
  virtual double ters_fc_d(double, Param *);
  virtual double ters_fa(double, Param *);
  virtual double ters_fa_d(double, Param *);
  virtual double ters_bij(double, Param *);
  virtual double ters_bij_d(double, Param *);

  virtual void ters_zetaterm_d(double, double *, double, double *, double,
                               double *, double *, double *, Param *);
  void costheta_d(double *, double, double *, double,
                  double *, double *, double *);
  void force_densities(int, double, double, double, double, double
    &fx, double &fy, double &fz);
  virtual void quad_neigh_flux();
  virtual void plane_intersections(double *pi, double *pj, double, double, double, double, double, double);

  // inlined functions for efficiency

  inline double ters_gijk(const double costheta,
                          const Param * const param) const {
    const double ters_c = param->c * param->c;
    const double ters_d = param->d * param->d;
    const double hcth = param->h - costheta;

    return param->gamma*(1.0 + ters_c/ters_d - ters_c / (ters_d + hcth*hcth));
  }

  inline double ters_gijk_d(const double costheta,
                            const Param * const param) const {
    const double ters_c = param->c * param->c;
    const double ters_d = param->d * param->d;
    const double hcth = param->h - costheta;
    const double numerator = -2.0 * ters_c * hcth;
    const double denominator = 1.0/(ters_d + hcth*hcth);
    return param->gamma*numerator*denominator*denominator;
  }

  inline double vec3_dot(const double x[3], const double y[3]) const {
    return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];
  }

  inline void vec3_add(const double x[3], const double y[3],
                       double * const z) const {
    z[0] = x[0]+y[0];  z[1] = x[1]+y[1];  z[2] = x[2]+y[2];
  }

  inline void vec3_scale(const double k, const double x[3],
                         double y[3]) const {
    y[0] = k*x[0];  y[1] = k*x[1];  y[2] = k*x[2];
  }

  inline void vec3_scaleadd(const double k, const double x[3],
                            const double y[3], double * const z) const {
    z[0] = k*x[0]+y[0];
    z[1] = k*x[1]+y[1];
    z[2] = k*x[2]+y[2];
  }
};

}

#endif
#endif

/* ERROR/WARNING messages:

E: Illegal ... command

Self-explanatory.  Check the input script syntax and compare to the
documentation for the command.  You can use -echo screen as a
command-line option when running LAMMPS to see the offending line.

E: Incorrect args for pair coefficients

Self-explanatory.  Check the input script or data file.

E: Pair style cac/sw requires atom IDs

This is a requirement to use the SW potential.

E: All pair coeffs are not set

All pair coefficients must be set in the data file or by the
pair_coeff command before running a simulation.

E: Cannot open Stillinger-Weber potential file %s

The specified SW potential file cannot be opened.  Check that the path
and name are correct.

E: Incorrect format in Stillinger-Weber potential file

Incorrect number of words per line in the potential file.

E: Illegal Stillinger-Weber parameter

One or more of the coefficients defined in the potential file is
invalid.

E: Potential file has duplicate entry

The potential file has more than one entry for the same element.

E: Potential file is missing an entry

The potential file does not have a needed entry.

*/