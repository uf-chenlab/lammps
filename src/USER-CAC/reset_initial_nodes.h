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

#ifdef COMMAND_CLASS
// clang-format off
CommandStyle(reset_initial_nodes,ResetInitialNodes)
// clang-format on
#else

#ifndef LMP_RESET_INITIAL_NODES_H
#define LMP_RESET_INITIAL_NODES_H

#include "command.h"

namespace LAMMPS_NS {

class ResetInitialNodes : public Command {
 public:
  ResetInitialNodes(class LAMMPS *);
  void command(int, char **);

 private:
};

}

#endif
#endif

/* ERROR/WARNING messages:

E: Reset_ids command before simulation box is defined

UNDOCUMENTED

E: Illegal ... command

UNDOCUMENTED

E: Cannot use reset_ids unless atoms have IDs

UNDOCUMENTED

E: Reset_ids missing %d bond topology atom IDs - use comm_modify cutoff

UNDOCUMENTED

*/
