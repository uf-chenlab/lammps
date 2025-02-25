/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing author: Axel Kohlmeyer (Temple U)
   Based on code by Paolo Raiteri (Curtin U) and Giovanni Bussi (SISSA)
------------------------------------------------------------------------- */

#include "fix_cac_temp_csvr.h"
#include <mpi.h>
#include <cstring>
#include <cmath>
#include <string>
#include "atom.h"
#include "force.h"
#include "comm.h"
#include "input.h"
#include "variable.h"
#include "group.h"
#include "update.h"
#include "modify.h"
#include "compute.h"
#include "random_mars.h"
#include "error.h"
#include "fmt/format.h"

using namespace LAMMPS_NS;
using namespace FixConst;

enum{NOBIAS,BIAS};
enum{CONSTANT,EQUAL};

double FixTempCSVRCAC::gamdev(const int ia)
{
  int j;
  double am,e,s,v1,v2,x,y;

  if (ia < 1) return 0.0;
  if (ia < 6) {
    x=1.0;
    for (j=1; j<=ia; j++)
      x *= random->uniform();

    // make certain, that -log() doesn't overflow.
    if (x < 2.2250759805e-308)
      x = 708.4;
    else
      x = -log(x);
  } else {
  restart:
    do {
      do {
        do {
          v1 = random->uniform();
          v2 = 2.0*random->uniform() - 1.0;
        } while (v1*v1 + v2*v2 > 1.0);

        y=v2/v1;
        am=ia-1;
        s=sqrt(2.0*am+1.0);
        x=s*y+am;
      } while (x <= 0.0);

      if (am*log(x/am)-s*y < -700 || v1<0.00001) {
        goto restart;
      }

      e=(1.0+y*y)*exp(am*log(x/am)-s*y);
    } while (random->uniform() > e);
  }
  return x;
}

/* -------------------------------------------------------------------
  returns the sum of n independent gaussian noises squared
  (i.e. equivalent to summing the square of the return values of nn
   calls to gasdev)
---------------------------------------------------------------------- */
double FixTempCSVRCAC::sumnoises(int nn) {
  if (nn == 0) {
    return 0.0;
  } else if (nn == 1) {
    const double rr = random->gaussian();
    return rr*rr;
  } else if (nn % 2 == 0) {
    return 2.0 * gamdev(nn / 2);
  } else {
    const double rr = random->gaussian();
    return  2.0 * gamdev((nn-1) / 2) + rr*rr;
  }
}

/* -------------------------------------------------------------------
  returns the scaling factor for velocities to thermalize
  the system so it samples the canonical ensemble
---------------------------------------------------------------------- */

double FixTempCSVRCAC::resamplekin(double ekin_old, double ekin_new){
  const double tdof = temperature->dof;
  const double c1 = exp(-update->dt/t_period);
  const double c2 = (1.0-c1)*ekin_new/ekin_old/tdof;
  const double r1 = random->gaussian();
  const double r2 = sumnoises(tdof - 1);

  const double scale = c1 + c2*(r1*r1+r2) + 2.0*r1*sqrt(c1*c2);
  return sqrt(scale);
}

/* ---------------------------------------------------------------------- */

FixTempCSVRCAC::FixTempCSVRCAC(LAMMPS *lmp, int narg, char **arg) :
  Fix(lmp, narg, arg),
  tstr(NULL), id_temp(NULL), random(NULL)
{
  if (narg != 7) error->all(FLERR,"Illegal fix temp/csvr command");

  // CSVR thermostat should be applied every step

  nevery = 1;
  scalar_flag = 1;
  global_freq = nevery;
  dynamic_group_allow = 1;
  extscalar = 1;

  tstr = NULL;
  if (strstr(arg[3],"v_") == arg[3]) {
    int n = strlen(&arg[3][2]) + 1;
    tstr = new char[n];
    strcpy(tstr,&arg[3][2]);
    tstyle = EQUAL;
  } else {
    t_start = utils::numeric(FLERR,arg[3],false,lmp);
    t_target = t_start;
    tstyle = CONSTANT;
  }

  t_stop = utils::numeric(FLERR,arg[4],false,lmp);
  t_period = utils::numeric(FLERR,arg[5],false,lmp);
  int seed = utils::inumeric(FLERR,arg[6],false,lmp);

  // error checks

  if (t_period <= 0.0) error->all(FLERR,"Illegal fix cac/temp/csvr command");
  if (seed <= 0) error->all(FLERR,"Illegal fix cac/temp/csvr command");

  random = new RanMars(lmp,seed + comm->me);

  // create a new compute temp style
  // id = fix-ID + temp, compute group = fix group

  std::string cmd = id + std::string("_temp");
  id_temp = new char[cmd.size()+1];
  strcpy(id_temp,cmd.c_str());

  cmd += fmt::format(" {} temp",group->names[igroup]);
  modify->add_compute(cmd);
  tflag = 1;

  nmax = -1;
  energy = 0.0;
}

/* ---------------------------------------------------------------------- */

FixTempCSVRCAC::~FixTempCSVRCAC()
{
  delete [] tstr;

  // delete temperature if fix created it

  if (tflag) modify->delete_compute(id_temp);
  delete [] id_temp;

  delete random;
  nmax = -1;
}

/* ---------------------------------------------------------------------- */

int FixTempCSVRCAC::setmask()
{
  int mask = 0;
  mask |= END_OF_STEP;
  return mask;
}

/* ---------------------------------------------------------------------- */

void FixTempCSVRCAC::init()
{

  // check variable
  if (!atom->CAC_flag) error->all(FLERR,"CAC fix styles require a CAC atom style");
  if (tstr) {
    tvar = input->variable->find(tstr);
    if (tvar < 0)
      error->all(FLERR,"Variable name for fix cac/temp/csvr does not exist");
    if (input->variable->equalstyle(tvar)) tstyle = EQUAL;
    else error->all(FLERR,"Variable for fix temp/csvr is invalid style");
  }

  int icompute = modify->find_compute(id_temp);
  if (icompute < 0)
    error->all(FLERR,"Temperature ID for fix cac/temp/csvr does not exist");
  temperature = modify->compute[icompute];

  if (temperature->tempbias) which = BIAS;
  else which = NOBIAS;
}

/* ---------------------------------------------------------------------- */

void FixTempCSVRCAC::end_of_step()
{

  // set current t_target
  // if variable temp, evaluate variable, wrap with clear/add

  double delta = update->ntimestep - update->beginstep;

  if (delta != 0.0) delta /= update->endstep - update->beginstep;
  if (tstyle == CONSTANT)
    t_target = t_start + delta * (t_stop-t_start);
  else {
    modify->clearstep_compute();
    t_target = input->variable->compute_equal(tvar);
    if (t_target < 0.0)
      error->one(FLERR,
                 "Fix cac/temp/csvr variable returned negative temperature");
    modify->addstep_compute(update->ntimestep + nevery);
  }

  const double t_current = temperature->compute_scalar();
  const double efactor = 0.5 * temperature->dof * force->boltz;
  const double ekin_old = t_current * efactor;
  const double ekin_new = t_target * efactor;

  // there is nothing to do, if there are no degrees of freedom

  if (temperature->dof < 1) return;

  // compute velocity scaling factor on root node and broadcast

  double lamda;
  if (comm->me == 0) {
    lamda = resamplekin(ekin_old, ekin_new);
  }
  MPI_Bcast(&lamda,1,MPI_DOUBLE,0,world);

  double * const * const * const * const nodal_velocities = atom->nodal_velocities;
  const int * const element_type = atom->element_type;
  const int * const poly_count = atom->poly_count;
  const int * const * const node_types = atom->node_types;
  const int * const nodes_count_list = atom->nodes_per_element_list;	
  int nodes_per_element; 

  double * const * const v = atom->v;  
  const int * const mask = atom->mask;
  const int nlocal = atom->nlocal;

  if (which == NOBIAS) {
    for (int i = 0; i < nlocal; i++) {
      if (mask[i] & groupbit) {
        v[i][0] = 0;
        v[i][1] = 0;
        v[i][2] = 0;
        nodes_per_element = nodes_count_list[element_type[i]];
        for (int l = 0; l < poly_count[i]; l++) {
          for (int j = 0; j < nodes_per_element; j++) {
          nodal_velocities[i][l][j][0] *= lamda;
          nodal_velocities[i][l][j][1] *= lamda;
          nodal_velocities[i][l][j][2] *= lamda;
          v[i][0] += nodal_velocities[i][l][j][0];
          v[i][1] += nodal_velocities[i][l][j][1];
          v[i][2] += nodal_velocities[i][l][j][2];
          }
        }
        v[i][0] = v[i][0] / nodes_per_element / poly_count[i];
        v[i][1] = v[i][1] / nodes_per_element / poly_count[i];
        v[i][2] = v[i][2] / nodes_per_element / poly_count[i];
      }
    }
  } else {
    for (int i = 0; i < nlocal; i++) {
      if (mask[i] & groupbit) {
        temperature->remove_bias(i,v[i]);
        v[i][0] = 0;
        v[i][1] = 0;
        v[i][2] = 0;
        nodes_per_element = nodes_count_list[element_type[i]];
        for (int l = 0; l < poly_count[i]; l++) {
          for (int j = 0; j < nodes_per_element; j++) {
          //temperature->remove_bias(i,v[i]);
          nodal_velocities[i][l][j][0] *= lamda;
          nodal_velocities[i][l][j][1] *= lamda;
          nodal_velocities[i][l][j][2] *= lamda;
          // temperature->restore_bias(i,v[i]);
          v[i][0] += nodal_velocities[i][l][j][0];
          v[i][1] += nodal_velocities[i][l][j][1];
          v[i][2] += nodal_velocities[i][l][j][2];
          }
        }
        v[i][0] = v[i][0] / nodes_per_element / poly_count[i];
        v[i][1] = v[i][1] / nodes_per_element / poly_count[i];
        v[i][2] = v[i][2] / nodes_per_element / poly_count[i];

        temperature->restore_bias(i,v[i]);
      }
    }
  }

  // tally the kinetic energy transferred between heat bath and system

  energy += ekin_old * (1.0 - lamda*lamda);
}

/* ---------------------------------------------------------------------- */

int FixTempCSVRCAC::modify_param(int narg, char **arg)
{
  if (strcmp(arg[0],"cac/nodal/temp") == 0) {
    if (narg < 2) error->all(FLERR,"Illegal fix_modify command");
    if (tflag) {
      modify->delete_compute(id_temp);
      tflag = 0;
    }
    delete [] id_temp;
    int n = strlen(arg[1]) + 1;
    id_temp = new char[n];
    strcpy(id_temp,arg[1]);

    int icompute = modify->find_compute(id_temp);
    if (icompute < 0)
      error->all(FLERR,"Could not find fix_modify temperature ID");
    temperature = modify->compute[icompute];

    if (temperature->tempflag == 0)
      error->all(FLERR,
                 "Fix_modify temperature ID does not compute temperature");
    if (temperature->igroup != igroup && comm->me == 0)
      error->warning(FLERR,"Group for fix_modify temp != fix group");
    return 2;
  }
  return 0;
}

/* ---------------------------------------------------------------------- */

void FixTempCSVRCAC::reset_target(double t_new)
{
  t_target = t_start = t_stop = t_new;
}

/* ---------------------------------------------------------------------- */

double FixTempCSVRCAC::compute_scalar()
{
  return energy;
}

/* ----------------------------------------------------------------------
   extract thermostat properties
------------------------------------------------------------------------- */

void *FixTempCSVRCAC::extract(const char *str, int &dim)
{
  dim=0;
  if (strcmp(str,"t_target") == 0) {
    return &t_target;
  }
  return NULL;
}
