#ifndef __DISCRETE_H
#define __DISCRETE_H

#include "common.h"
#include "solution.h"
#include "matrix.h"

class Space;
class PrecalcShapeset;
class RefMap;
struct Page;
struct EdgePos;


typedef scalar (*BiFormFnVol) (RealFunction*, RealFunction*, RefMap*, RefMap*);
typedef scalar (*BiFormFnSurf)(RealFunction*, RealFunction*, RefMap*, RefMap*, EdgePos*);
typedef scalar (*LiFormFnVol) (RealFunction*, RefMap*);
typedef scalar (*LiFormFnSurf)(RealFunction*, RefMap*, EdgePos*);

const BiFormFnVol BF_SYM     = (BiFormFnVol) 1;
const BiFormFnVol BF_ANTISYM = (BiFormFnVol) 2;


struct BiForm
{
  BiFormFnVol  unsym, sym;
  BiFormFnSurf surf;
};

struct LiForm
{
  LiFormFnVol  lf;
  LiFormFnSurf surf;
};


/// THIS CLASS IS DEPRECATED
///
class DiscreteProblem
{
public:

  DiscreteProblem();
  ~DiscreteProblem() { free(); }

  void copy(DiscreteProblem* ep);
  void free();

  void set_num_equations(int neq);
  void set_spaces(int n, ...);
  void set_spaces(Space** spaces);
  void set_pss(int n, ...);
  void set_external_fns(int n, ...);

  void set_bilinear_form(int i, int j, BiFormFnVol unsym, BiFormFnVol sym = NULL, BiFormFnSurf surf = NULL);
  void set_linear_form(int i, LiFormFnVol linear_form, LiFormFnSurf linear_form_surf = NULL);

  void create_matrix();
  void assemble_matrix_and_rhs(bool rhsonly = false);
  void assemble_rhs() { assemble_matrix_and_rhs(true); }
  bool solve_system(int n, ...);
  bool solve_again(int n, ...);


  void save_matrix_matlab(const char* filename, const char* varname);
  void save_matrix_coo(const char* filename);
  void save_rhs_matlab(const char* filename, const char* varname);
  void save_matrix_bin(const char* filename);
  void save_rhs_bin(const char* filename);

  void precalc_equi_coefs();
  void equilibrate_matrix();
  bool is_equilibrated() const { return is_equi; }

  void set_quiet(bool quiet) { this->quiet = quiet; }

  int get_num_equations() const { return neq; }
  Space** get_spaces() const { return spaces; }
  PrecalcShapeset** get_pss() const { return pss; }
  scalar* get_solution_vector() const { return vec; }
  BiForm* get_bilinear_form(int i, int j) const { return &(biform[i][j]); }
  LiForm* get_linear_form(int i) const { return &(liform[i]); }

  int get_num_dofs() const { return ndofs; };
  void get_matrix(int*& Ap, int*& Ai, scalar*& Ax, int& size) const
    { Ap = this->Ap; Ai = this->Ai; Ax = this->Ax; size = ndofs; }
  int get_matrix_size() const;

  void free_matrix() { free_matrix_indices(); free_matrix_values(); }


protected:
  
  int neq;
    
  BiForm** biform;
  LiForm*  liform;
  
  Space** spaces;
  PrecalcShapeset** pss;
  MeshFunction** extern_fns;
  int num_extern, num_user_pss;

  int ndofs;
  
  int* Ap;    ///< indices of column beginnings
  int* Ai;    ///< element positions within columns
  scalar* Ax; ///< matrix values

  scalar* RHS;  ///< assembled right-hand side
  scalar* Dir;  ///< contributions to the RHS from Dirichlet DOFs
  scalar* vec;  ///< solution vector
  double* equi; ///< equilibration vector
  
  void* Symbolic; ///< UMFPACK symbolic analysis result
  void* Numeric;  ///< UMFPACK numeric analysis result

  void precalculate_sparse_structure(Page** pages);
  void insert_matrix(scalar** mat, int* iidx, int* jidx, int ilen, int jlen);
  void alloc_matrix_values();
  void free_solution_vector();
  void free_matrix_indices();
  void free_matrix_values();
  
  scalar** get_matrix_buffer(int n)
  {
    if (n <= mat_size) return buffer;
    if (buffer != NULL) delete buffer;
    return (buffer = new_matrix<scalar>(mat_size = n));
  }

  scalar** buffer;
  int mat_size;

  bool quiet;
  bool is_equi;
  
};



// can be called to set a custom order limiting table
/*extern void set_order_limit_table(int* tri_table, int* quad_table, int n);

// limit_order is used in integrals
extern int  g_max_order;
extern int* g_order_table;

#ifndef DEBUG_ORDER
  #define limit_order(o) \
    if (o > g_max_order) { o = g_max_order; warn_order(); } \
    o = g_order_table[o];
  #define limit_order_nowarn(o) \
    if (o > g_max_order) o = g_max_order; \
    o = g_order_table[o];
#else
  #define limit_order(o) \
    if (o > g_max_order) warn_order(); \
    o = g_max_order;
  #define limit_order_nowarn(o) \
    o = g_max_order;
#endif*/

extern void warn_order();
extern void update_limit_table(int mode);


#endif
