#include "ILPSolverIf.h"
#include "OsiClpSolverInterface.hpp"
#include "CbcModel.hpp"
#include "symphony.h"

ILPSolverIf::ILPSolverIf(const SOLVER_ENUM& se) : _se(se), _t{-1}, _nvar{0}, _nrow{0}, _sol{nullptr}
{
  if (se == SOLVER_ENUM::Cbc) {
    _solver = new OsiClpSolverInterface;
  } else {
    _solver = sym_open_environment();
  }
}

ILPSolverIf::~ILPSolverIf()
{
  if (_se == SOLVER_ENUM::Cbc) {
    delete static_cast<OsiClpSolverInterface*>(_solver);
  } else {
    sym_close_environment(static_cast<sym_environment *>(_solver));
  }
  _solver = nullptr;
  delete[] _sol;
  _sol = nullptr;
}

double ILPSolverIf::getInfinity() const
{
  if (_se == SOLVER_ENUM::Cbc) return static_cast<OsiClpSolverInterface*>(_solver)->getInfinity();
  return sym_get_infinity();
}

void ILPSolverIf::loadProblem(const int nvar, const int nrow, const int* starts,
        const int* indices, const double* values, const double* varlb, const double* varub,
        const double* obj, const double* rowlb, const double* rowub, const int* intvars)
{
  if (_solver) {
    if (_se == SOLVER_ENUM::Cbc) {
      _nvar = nvar;
      _nrow = nrow;
      auto sl = static_cast<OsiClpSolverInterface*>(_solver);
      sl->loadProblem(nvar, nrow, starts, indices,
          values, varlb, varub, obj, rowlb, rowub);
      for (int i = 0; i < static_cast<int>(nvar); ++i) {
        if (intvars[i]) {
          sl->setInteger(i);
        }
      }
    }
  }
}

void ILPSolverIf::loadProblemSym(int nvar, int nrow, int* start,
    int* indices, double* values, double* varlb, double* varub,
    char *intvars, double* obj, char *sens, double *rhs)
{
  if (_solver) {
    if (_se == SOLVER_ENUM::SYMPHONY) {
      _nvar = nvar;
      _nrow = nrow;
      auto sl = static_cast<sym_environment*>(_solver);
      sym_explicit_load_problem(sl,
          nvar, nrow, start, indices,
          values, varlb, varub,
          intvars, obj, NULL, sens, rhs, NULL, TRUE);
      sym_set_int_param(sl, "verbosity", -2);
    }
  }
}

int ILPSolverIf::solve(const int num_threads)
{
  int status{0};
  if (_se == SOLVER_ENUM::Cbc) {
    CbcModel model(*static_cast<OsiClpSolverInterface*>(_solver));
    model.setLogLevel(0);
    model.setMaximumSolutions(1000);
    model.setMaximumSavedSolutions(1000);
    if (_t > 0) model.setMaximumSeconds(_t);
    if (num_threads > 1 && CbcModel::haveMultiThreadSupport()) {
      model.setNumberThreads(num_threads);
      if (_t > 0) model.setMaximumSeconds(_t * num_threads);
      const char* argv[] = {"", "-log", "0", "-threads", std::to_string(num_threads).c_str(), "-solve"};
      status = CbcMain(6, argv, model);
    } else {
      const char* argv[] = {"", "-log", "0", "-solve"};
      status = CbcMain(4, argv, model);
    }
    status = model.secondaryStatus();
    double* var = model.bestSolution();
    if (var) {
      _sol = new double[model.getNumCols()];
      for (int i = 0; i < model.getNumCols(); ++i) _sol[i] = var[i];
    }
  } else {
    auto sl = static_cast<sym_environment*>(_solver);
    sym_solve(sl);
    status = sym_get_status(sl);
    if (_t > 0) sym_set_int_param(sl, "time_limit", _t);
    if (status == TM_OPTIMAL_SOLUTION_FOUND || status == TM_FOUND_FIRST_FEASIBLE)  {
      status = 0;
      int n{0};
      sym_get_num_cols(sl, &n);
      if (n > 0) {
        _sol = new double[n];
        sym_get_col_solution(sl, _sol);
      }
    }
  }
  return status;
}


void ILPSolverIf::writelp(char *filename, char **varnames, char **colnames)
{
  if (_se == SOLVER_ENUM::Cbc) {
    auto osiclp = static_cast<OsiClpSolverInterface*>(_solver);
    if (varnames) for (int i = 0; i < _nvar; ++i) osiclp->setColName(i, varnames[i]);
    if (colnames) for (int i = 0; i < _nrow; ++i) osiclp->setRowName(i, colnames[i]);
    osiclp->writeLp(filename);
  } else {
    auto sl = static_cast<sym_environment*>(_solver);
    if (varnames) sym_set_col_names(sl, varnames);
    sym_write_lp(sl, filename);
  }
}
