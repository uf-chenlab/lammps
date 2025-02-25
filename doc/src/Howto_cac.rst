CAC method
==========

**Overview:**

The Concurrent Atomistic Continuum (CAC) method, as implemented in
LAMMPS, is a concurrent multiscale method implemented using the Finite
Element Method (FEM) that consistently solves systems with both
coarsely-meshed, using finite elements, and atomistic regions. This is
achieved by defining a non-local force density at all finite elements,
as well as the typical particle force at atoms, using the interatomic
potentials that are ubiquitous in Molecular Dynamics. The theoretical
framework that makes this possible is an atomistic formulation of the
field representation of conservation laws; this formulation is
constructed around the concepts of discontinuous atomic level structures
and non-local atomic interaction as presented in
:ref:`(Chen) <USER-CAC-Chen>`.  Thus, with no additional treatment other than
the coarse graining of matter into finite element regions, a fully
dynamic and consistent solution is obtained with both coarse-grained and
atomistic regions.

The primary predictive features of the CAC method for material modeling
include long wavelength phonon dynamics and transport that includes all
phonon branches, the ability to model mobile defects such as
dislocations even in the coarse grained region, and exhibit both
phonon-phonon scattering and phonon-defect/interface scattering. These
features arise seamlessly once the mesh and interatomic potential are
defined to drive the system. The features involving defects do however
require the definition of an unconnected mesh; this process will be
discussed in detail in later sections of this Howto.

Once the typical LAMMPS settings such as units, dimension, and boundary
conditions have been specified in your input script,the CAC code can
first be invoked via the :doc:`atom\_style cac <atom_style>` command; in
addition to other CAC atom substyles that require charge etc. Please
note that CAC atom styles take two input arguments, these are described
in the atom style's specific page of the manual and later on.
Afterwards the comm style must be set to "cac" in your input script, not
doing so will generate an error message when using CAC pair styles.
Once the CAC atom and Comm style has been set the model to be simulated
must be read in with a typical :doc:`read\_data <read_data>` command in
your input script. What is not typical however is the format in which a
CAC model is declared in the data file to be read in. For an explicit
example, please look in the /examples/USER/CAC folder or read the next
sections of this Howto for more detailed input instructions.  Once the
model has been read, the set of CAC specific pair styles, fixes,
min\_styles, computes, and dumps listed in the USER-CAC section of
:doc:`package details <Packages_details>` can be invoked to achieve your
objectives; please see the example input scripts with several use cases
in the /examples/USER/CAC folder.


----------


**When to use the CAC method:**

The advantage of the CAC method is in its ability to exploit a reduction
in the level of detail for large systems that potentially only require
atomistic resolution in very small regions at any given time. The rest
of the model is coarse-grained due to the more predictably smooth
deformation that occurs there. This then allows one to interpolate what
would normally be an abundance of atoms exhibiting polynomial order
deformation in a region; i.e. one can fit a polynomial to describe how
the set of atoms is moving with time using a lagrangian description. In
such a scenario the use of the CAC method will remove the unnecessary
detail while providing a seamless non-local force description for both
the coarse-grained and the atomically resolved regions. Other
exploitations include the ability to simulate the nucleation and
propagation of dislocations and cracks between finite elements; in both
cases the displacement fields associated with the dislocation core or
crack tip are approximated. Nonetheless, such defects can be nucleated
and propagated with coarse-grained detail, see :ref:`(Xu et al) <USER-CAC-Xu-et-al>` for an example.

The CAC method is fundamentally a coarse graining method. One should be
cognizant of the level of detail they desire in their system since
coarse graining with CAC removes short wavelength spectral information
in the coarse grained region, has the potential to pin defects against
finite element surfaces if the model is not well oriented and detailed,
and can exhibit numerically induced instability on the system energy for
certain dynamic simulations.


----------


**The CAC data format:**

The header of the data file for all CAC atom styles remains very much
the same as that found in typical :doc:`LAMMPS use cases <read_data>`. One
only needs to replace the word "atoms" with "cac elements" in the
declaration of how many elements and atoms to be read. The denotation of
the number of atom types, box limits, and atom masses remains
standard. The section header for the finite elements and atoms to be
read in is then also "CAC Elements". The declaration of atoms and finite
elements can then commence according to the format to be discussed.

When declaring a CAC element or atom you must begin with the element header


.. parsed-literal::

   element_id  element_string  iDOF_count size_1  size_2  size_3

Where the element id is typically a global number 1-N (N being the total
number of elements in your system), the element\_string denotes what kind
of element is being declared with respect to its interpolation field and
geometry. Currently only the "Eight\_Node and Atom" strings are accepted
here until other element types are implemented. The next number is the
count of internal degrees of freedom this element possesses. The CAC
method models materials according to the principles of lattice dynamics
and solid state physics, thus every material point is convoluted with a
basis that is intended to describe a material with a polyatomic
crystalline configuration. As a result, every finite element has a total
number of degrees of freedom equal to the number of nodes multiplied by
the number of internal degrees of freedom (atoms in the unit cell of
such a material). The next three numbers denote the size of the finite
element in terms of how many unit cells it is supposed to represent
(note that this is very representative for an Eight\_Node element but one
or more of these may be a dummy input in the future for other element
shapes like a sphere etc.). By convention, when specifying an atom with
"Atom" as the element type the internal dof count and scale numbers are
set to one.

After the element header is specified the information for each node/DOF
of this element must be supplied. These specifications do depend on the
CAC atom style used since you may have to specify something additional
such as the charge associated with that DOF. We'll start with the
example of declaring a subset of the nodes for an "Eight\_Node" element:


.. parsed-literal::

   1 Eight_Node 4 10 10 10
   1  1  1  -18.07500000  -18.07500000  -18.07500000
   2  1  1   18.07500000  -18.07500000  -18.07500000
   3  1  1   18.07500000   18.07500000  -18.07500000
   4  1  1  -18.07500000   18.07500000  -18.07500000
   5  1  1  -18.07500000  -18.07500000   18.07500000
   6  1  1   18.07500000  -18.07500000   18.07500000
   7  1  1   18.07500000   18.07500000   18.07500000
   8  1  1  -18.07500000   18.07500000   18.07500000
   1  2  1  -16.26750000  -18.07500000  -16.26750000
   2  2  1   19.88250000  -18.07500000  -16.26750000
   3  2  1   19.88250000   18.07500000  -16.26750000
   4  2  1  -16.26750000   18.07500000  -16.26750000
   5  2  1  -16.26750000  -18.07500000   19.88250000
   6  2  1   19.88250000  -18.07500000   19.88250000
   7  2  1   19.88250000   18.07500000   19.88250000
   8  2  1  -16.26750000   18.07500000   19.88250000

The node/DOF format above follows the pattern:


.. parsed-literal::

   node_index  iDOF_index  atom_type  *atom_type_properties*  x_pos  y_pos  z_pos

where node index ranges from 1:M, with M being the number of nodes or
DOF the interpolation scheme of your element type requires. The internal
DOF index denotes which internal degree of freedom this node belongs to,
and the atom type index denotes which material this nodal degree of
freedom is interpolating for. With other CAC atom styles, such as
cac/charge, the charge of the respective material for that node/DOF will
be declared before the position of that node/DOF.  In the example above
this is blank since there were no additional material properties to
declare.

.. note::

   The order in which the nodes for each internal DOF are supplied is
   not specific, you must however make sure to supply all of them in order
   to avoid an error message. This means you can for example specify the
   first node for the range of internal DOFs and the second for the range
   etc. instead of the sorting used in the above example. Any other
   combination is also acceptable as long as you have supplied
   number\_of\_nodes\*number\_of\_iDOF entries.

While this establishes how to provide the information there remains a
couple of details to explain about how to create CAC model geometries
that work as intended.


----------


**CAC atom styles list:**

Currently all CAC atom styles have the same input arguments as show in
the CAC atom style documented in :doc:`atom style <atom_style>`. The
current list of CAC atom styles is:


.. parsed-literal::

   *cac*
   *cac/charge*

**The CAC input geometry:**

A CAC model can involve a collection of atoms and finite elements in a
discontinuous mesh. This comes with the need to clarify what constitutes
a reasonable mesh with respect to how forces are computed from its
definition; the word mesh here is being used loosely in that it includes
the set of atoms as well.

One of the most recurring initial inputs for a CAC simulation is that of
a finite crystal for the material in question; with perhaps several
defects or surfaces defined initially as well. More complex cases will
typically involved superpositions of several crystalline subsets with
defect surfaces between them; we will thus describe some of the more
obscure details needed to define a crystalline input when it consists of
both finite elements and atoms.

Recall that the mesh input MUST be discontinuous. In other words, Finite
elements own a unique set of nodes that no other finite element owns. In
the case of a crystalline input the nodes of adjacent finite elements
can however overlap where their positions are concerned.  The image
denotes the mesh geometry for a 1D lattice chain depicted in mixed
resolution.

.. image:: JPG/1D-CAC-chain.jpg

Thus, if we wished to resolve a crystalline input with a mix of finite
elements and atoms we must be careful to place the nodes and atoms in
the correct place. Nodes overlap with nodes of adjacent finite elements
in a mesh representing a crystal, and atoms are usually spaced apart by
a (the unit cell length) with respect to other atoms. Thus, whenever one
transitions between resolving a crystal with finite elements and atoms
they must leave a space of a/2 between the adjacent node and atom. This
would then simulate a lattice with a mix of atoms and finite elements.
In the case of polyatomic unit cells the same logic applies to your unit
cell origin in place of the atom; one then simply convolutes the
internal basis around that point.

.. note::

   The CAC algorithm assumes for the sake of symmetry that the unit
   cell's atom or convolution point is at the center of the unit cell
   volume. Thus if you are accustomed to introducing a basis with an atom
   or convolution point at the corner of the unit cell, this practice will
   not be consistent with the CAC algorithm.

.. warning::

   Ensure that your simulation box dimensions include the
   extent of your finite elements when using periodic boundary conditions;
   Otherwise you will create overlapping material between the model and its
   images.


----------


**CAC elements and group definitions**

It is important to note that if you wish to group CAC elements through a
spatial command, such as defining a group through a region, then you
must make sure the group's spatial extent includes the CAC element's
centroid; this is the average of the element's nodal positions.

.. note::

   Elements are also considered owned by a task's subbox if the
   centroid is in that subbox.


----------


**CAC Pair Styles, and Fixes:**

The CAC method is implemented to run with its own version of Pair Styles
and Fixes that come with the package.  The Pair Styles are named
according to the interatomic potential that is used to define the force
field in the convention of cac/potential\_name. Likewise fixes, such as
the typical nve integrator, are labeled cac/fix\_name.

When invoking CAC Pair styles one can declare the potential parameters
using :doc:`pair\_coeff <pair_coeff>` just as they would for non-CAC pair
styles of the respective potential. Examples of CAC pair styles
currently implemented include :doc:`cac/buck <pair_cac_buck>`,
:doc:`cac/eam <pair_cac_eam>`, and :doc:`cac/sw <pair_cac_sw>`.


----------


**Running Multiple Resolutions in Parallel:**

In order to obtain good performance running CAC models with multiple
resolutions one must invoke the :doc:`fix balance <fix_balance>`
command. This ensures that your simulation has the capability to
dynamically reassign each computing task's burden according to a set of
weights. The weights in question are provided by the compute :doc:`compute cac/quad/count <compute_cac_quad_count>` through a variable
command. For specific details of syntax, please see the example input
scripts in /examples/USER/CAC of your LAMMPS directory or refer to the
online documentation of fix balance and creating variables that call on
computes.


----------


**CAC output:**

Obtaining simulation output with CAC can currently be done with both
thermodynamic data (such as the kinetic energy of the nodal information
in your model) with :doc:`compute cac/nodal/temp <compute_cac_nodal_temp>`
or outputting a list of nodal information at specified times with dumps
such as `dump cac/nodal/positions <dump_cac_nodal_positions>`_.  This nodal
information can then be converted to the user's preferred visualization
format for software such as Paraview (which is open source) or
Tecplot. It is worth noting that Paraview interprets many formats. The
Tecplot and VTK formats are among these. The following is an example
Tecplot output of four fold phonon focusing in Silicon.

.. image:: JPG/four_fold_focusing_Si.jpg

----------

.. _USER-CAC-Chen:

**(USER-CAC-Chen)** Chen, Y. Reformulation of microscopic balance equations for multiscale materials modeling.
The Journal of Chemical Physics 130, 134706, (2009).

.. _USER-CAC-Xu-et-al:

**(USER-CAC-Xu-et-al)** Xu, S., Xiong, L., Chen, Y. & McDowell, D. L. Sequential slip transfer of mixed-character dislocations across S3 coherent
twin boundary in FCC metals: a concurrent atomistic-continuum study. npj Computational Materials 2, 15016 (2016).


.. _lws: http://lammps.sandia.gov
.. _ld: Manual.html
.. _lc: Commands_all.html
