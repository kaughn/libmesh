// The libMesh Finite Element Library.
// Copyright (C) 2002-2020 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#ifndef LIBMESH_EIGEN_SYSTEM_H
#define LIBMESH_EIGEN_SYSTEM_H

#include "libmesh/libmesh_config.h"

// Currently, the EigenSystem should only be available
// if SLEPc support is enabled.
#if defined(LIBMESH_HAVE_SLEPC)

// Local Includes
#include "libmesh/system.h"
#include "libmesh/eigen_solver.h"

namespace libMesh
{

// Forward Declarations
template <typename T> class SparseMatrix;
template <typename T> class ShellMatrix;


/**
 * \brief Manages consistently variables, degrees of freedom, and coefficient
 * vectors for eigenvalue problems.
 *
 * Currently, this class is able to handle standard eigenvalue
 * problems \p A*x=lambda*x and generalized eigenvalue problems \p
 * A*x=lambda*B*x.
 *
 * The matrices EigenSystem::matrix_A and EigenSystem::matrix_B should be
 * filled during assembly.
 *
 * \author Steffen Peterson
 * \date 2005
 * \brief Base class for defining systems of equations for eigenproblems.
 */
class EigenSystem : public System
{
public:

  /**
   * Constructor.  Optionally initializes required
   * data structures.
   */
  EigenSystem (EquationSystems & es,
               const std::string & name_in,
               const unsigned int number_in);

  /**
   * Destructor.
   */
  virtual ~EigenSystem ();

  /**
   * The type of system.
   */
  typedef EigenSystem sys_type;

  /**
   * The type of the parent
   */
  typedef System Parent;

  /**
   * \returns A reference to *this.
   */
  sys_type & system () { return *this; }

  /**
   * Clear all the data structures associated with
   * the system.
   */
  virtual void clear () override;

  /**
   * Reinitializes the member data fields associated with
   * the system, so that, e.g., \p assemble() may be used.
   */
  virtual void reinit () override;

  /**
   * Assembles & solves the eigen system.
   */
  virtual void solve () override;

  /**
   * Assembles the system matrix.
   */
  virtual void assemble () override;

  /**
   * \returns Real and imaginary part of the ith eigenvalue and copies
   * the respective eigen vector to the solution vector.
   */
  virtual std::pair<Real, Real> get_eigenpair (dof_id_type i);

  /**
   * \returns Real and imaginary part of the ith eigenvalue but
   * does not copy the respective eigen vector to the solution vector.
   */
  virtual std::pair<Real, Real> get_eigenvalue (dof_id_type i);

  /**
   * \returns \p "Eigen".  Helps in identifying
   * the system type in an equation system file.
   */
  virtual std::string system_type () const override { return "Eigen"; }

  /**
   * \returns The number of matrices handled by this system
   */
  virtual unsigned int n_matrices () const override;

  /**
   * \returns The number of converged eigenpairs.
   */
  unsigned int get_n_converged () const {return _n_converged_eigenpairs;}

  /**
   * \returns The number of eigen solver iterations.
   */
  unsigned int get_n_iterations () const {return _n_iterations;}

  /**
   * Sets the type of the current eigen problem.
   */
  void set_eigenproblem_type (EigenProblemType ept);

  /**
   * \returns The eigen problem type.
   */
  EigenProblemType get_eigenproblem_type () const {return _eigen_problem_type;}

  /**
   * Sets an initial eigen vector
   */
  void set_initial_space(NumericVector<Number> & initial_space_in);

  /**
   * \returns \p true if the underlying problem is generalized
   * , false otherwise.
   */
  bool generalized () const { return _is_generalized_eigenproblem; }

  /**
   * \returns \p true if the shell matrices are used
   */
  bool use_shell_matrices() const { return _use_shell_matrices; }

  /**
   * Set a flag to use shell matrices
   */
  void use_shell_matrices(bool use_shell_matrices) { _use_shell_matrices = use_shell_matrices; }

  /**
   * \returns \p true if a shell preconditioning matrix is used
   */
  bool use_shell_precond_matrix() const { return _use_shell_precond_matrix; }

  /**
   * Set a flag to use a shell preconditioning matrix
   */
  void use_shell_precond_matrix(bool use_shell_precond_matrix) { _use_shell_precond_matrix = use_shell_precond_matrix; }

  /**
   * Adds the additional matrix \p mat_name to this system.
   *
   * @param type The serial/parallel/ghosted type of the matrix
   * @param mat_build_type The matrix type to build
   *
   */
  Matrix & add_matrix (const std::string & mat_name,
                       ParallelType type = PARALLEL,
                       MatrixBuildType mat_build_type = MatrixBuildType::AUTOMATIC) override;

  /**
   * \returns \p true if this \p System has a matrix associated with the
   * given name, \p false otherwise.
   */
  bool have_matrix (const std::string & mat_name) const override;

  /**
   * \returns A const reference to this system's additional matrix
   * named \p mat_name.
   *
   * None of these matrices is involved in the solution process.
   */
  const Matrix & get_matrix (const std::string & mat_name) const override;

  /**
   * \returns A writable reference to this system's additional matrix
   * named \p mat_name.
   *
   * None of these matrices is involved in the solution process.
   */
  Matrix & get_matrix (const std::string & mat_name) override;


  /**
   * The system matrix for standard eigenvalue problems.
   */
  std::unique_ptr<SparseMatrix<Number>> matrix_A;

  /**
   * A second system matrix for generalized eigenvalue problems.
   */
  std::unique_ptr<SparseMatrix<Number>> matrix_B;

  /**
   * The system shell matrix for standard eigenvalue problems.
   */
  std::unique_ptr<ShellMatrix<Number>> shell_matrix_A;

  /**
   * A second system shell matrix for generalized eigenvalue problems.
   */
  std::unique_ptr<ShellMatrix<Number>> shell_matrix_B;

  /**
   * A preconditioning matrix
   */
  std::unique_ptr<SparseMatrix<Number>> precond_matrix;

  /**
   * A preconditioning shell matrix
   */
  std::unique_ptr<ShellMatrix<Number>> shell_precond_matrix;

  /**
   * The EigenSolver, defining which interface, i.e solver
   * package to use.
   */
  std::unique_ptr<EigenSolver<Number>> eigen_solver;


protected:


  /**
   * Initializes the member data fields associated with
   * the system, so that, e.g., \p assemble() may be used.
   */
  virtual void init_data () override;

  /**
   * Initializes the matrices associated with the system
   */
  virtual void init_matrices ();

  /**
   * Set the _n_converged_eigenpairs member, useful for
   * subclasses of EigenSystem.
   */
  void set_n_converged (unsigned int nconv)
  { _n_converged_eigenpairs = nconv; }

  /**
   * Set the _n_iterations member, useful for subclasses of
   * EigenSystem.
   */
  void set_n_iterations (unsigned int its)
  { _n_iterations = its;}


private:

  /**
   * The number of converged eigenpairs.
   */
  unsigned int _n_converged_eigenpairs;

  /**
   * The number of iterations of the eigen solver algorithm.
   */
  unsigned int _n_iterations;

  /**
   * A boolean flag to indicate whether we are dealing with
   * a generalized eigenvalue problem.
   */
  bool _is_generalized_eigenproblem;

  /**
   * The type of the eigenvalue problem.
   */
  EigenProblemType _eigen_problem_type;

  /**
   * A boolean flag to indicate whether or not to use shell matrices
   */
  bool _use_shell_matrices;

  /**
   * A boolean flag to indicate whether or not to use a shell preconditioning matrix
   */
  bool _use_shell_precond_matrix;

  /**
   * Some systems need an arbitrary number of matrices.
   */
  std::map<std::string, Matrix *> _matrices;
};



// ------------------------------------------------------------
// EigenSystem inline methods
inline
unsigned int EigenSystem::n_matrices () const
{
  if (_is_generalized_eigenproblem)
    return 2;

  return 1;
}


inline
bool EigenSystem::have_matrix (const std::string & mat_name) const
{
  return (_matrices.count(mat_name));
}

} // namespace libMesh

#endif // LIBMESH_HAVE_SLEPC

#endif // LIBMESH_EIGEN_SYSTEM_H
