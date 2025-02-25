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

#ifdef DUMP_CLASS

DumpStyle(cac/flux,DumpCACNodalFlux)

#else

#ifndef LMP_DUMP_CACNODALFLUX_H
#define LMP_DUMP_CACNODALFLUX_H

#include "dump.h"

namespace LAMMPS_NS {

class DumpCACNodalFlux : public Dump {
 public:
  DumpCACNodalFlux(class LAMMPS *, int, char**);
  virtual ~DumpCACNodalFlux();

 protected:
  int ntypes;
  int cac_flux_flag;
  char **typenames;
  bigint ptimestep;
  int total_node_count, total_element_count;
  double box_size[3], box_center[3];

  void init_style();
  void write_header(bigint);
  void pack(tagint *);
  int convert_string(int, double *);
  void write_data(int, double *);
  int count();
  int modify_param(int, char **);

  typedef void (DumpCACNodalFlux::*FnPtrWrite)(int, double *);
  FnPtrWrite write_choice;              // ptr to write data functions

  void write_string(int, double *);
  void write_lines(int, double *);
};

}

#endif
#endif

/* ERROR/WARNING messages:

E: Illegal ... command

Self-explanatory.  Check the input script syntax and compare to the
documentation for the command.  You can use -echo screen as a
command-line option when running LAMMPS to see the offending line.

E: Invalid dump xyz filename

Filenames used with the dump xyz style cannot be binary or cause files
to be written by each processor.

E: CAC dump styles require a CAC atom style

Self-explanatory

E: Dump modify element names do not match atom types

Number of element names must equal number of atom types.

*/
