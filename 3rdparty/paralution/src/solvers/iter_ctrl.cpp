// *************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2014 Dimitar Lukarski
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************



// PARALUTION version 0.7.0 


#include "iter_ctrl.hpp"
#include "../utils/log.hpp"
#include "../utils/math_functions.hpp"

#include <math.h>
#include <cstdlib>
#include <assert.h>
#include <fstream>
#include <limits>

namespace paralution {

template <typename ValueType>
IterationControl<ValueType>::IterationControl() {
  
  this->Clear();

}

template <typename ValueType>
IterationControl<ValueType>::~IterationControl() {

  this->Clear();

}

template <typename ValueType>
void IterationControl<ValueType>::Clear(void) {

  this->residual_history_.clear();
  this->iteration_ = 0 ;

  this->init_res_ = false;
  this->rec_ = false;
  this->verb_ = 1;
  this->reached_ = 0 ;

  this->current_res_ = 0.0f ;
  this->current_index_ = -1;

  this->absolute_tol_   = ValueType(1e-15);
  this->relative_tol_   = ValueType(1e-6);
  this->divergence_tol_ = ValueType(1e+8);
  this->maximum_iter_   = 1000000;
}

template <typename ValueType>
void IterationControl<ValueType>::Init(const ValueType abs,
                                       const ValueType rel,
                                       const ValueType div,
                                       const int max) {

  this->InitTolerance(abs, rel, div);
  this->InitMaximumIterations(max);
}


template <typename ValueType>
void IterationControl<ValueType>::InitResidual(const ValueType res) {

  this->init_res_ = true;
  this->initial_residual_ = res;

  this->reached_ = 0 ;
  this->iteration_ = 0 ;

  if (this->verb_ > 0)
    LOG_INFO("IterationControl initial residual = " << res);
    
}

template <typename ValueType>
void IterationControl<ValueType>::InitTolerance(const ValueType abs,
                                                const ValueType rel,
                                                const ValueType div) {

  this->absolute_tol_ = abs;
  this->relative_tol_ = rel;
  this->divergence_tol_ = div;
  
}

template <typename ValueType>
void IterationControl<ValueType>::InitMaximumIterations(const int max) {

  this->maximum_iter_ = max;

}

template <typename ValueType>
int IterationControl<ValueType>::GetMaximumIterations(void) const {

  return this->maximum_iter_;

}


template <typename ValueType>
int IterationControl<ValueType>::GetIterationCount(void) {

  return this->iteration_;

}

template <typename ValueType>
ValueType IterationControl<ValueType>::GetCurrentResidual(void) {

  return this->current_res_;

}

template <typename ValueType>
int IterationControl<ValueType>::GetAmaxResidualIndex(void) {

  return this->current_index_;

}


template <typename ValueType>
int IterationControl<ValueType>::GetSolverStatus(void) {

  return this->reached_;

}

template <typename ValueType>
bool IterationControl<ValueType>::CheckResidual(const ValueType res) {

  assert(this->init_res_ == true);

  this->iteration_++;
  this->current_res_ = res;

  if (this->verb_ > 1)
    LOG_INFO("IterationControl iter=" << this->iteration_ <<  "; residual=" << res);

  if (this->rec_ == true) 
    this->residual_history_.push_back(res);
  
  if (( paralution_abs(res) == std::numeric_limits<ValueType>::infinity()) || // infinity
      ( res != res ) ) { // not a number (NaN)

    LOG_INFO("Resdual = " << res << " !!!");
    return true;

  }
  
  if ( fabs(res) <= this->absolute_tol_ ) {
    
    this->reached_ = 1 ;
    return true;

    }

  if ( res / this->initial_residual_ <= this->relative_tol_ ) {

    this->reached_ = 2 ;
    return true;

  }


  if ( res / this->initial_residual_ >= this->divergence_tol_ ) {

    this->reached_ = 3 ;
    return true;
 
 }

  if ( this->iteration_ >= this->maximum_iter_ ) {

    this->reached_ = 4 ;
    return true;
 
 }

  return false;

}

template <typename ValueType>
bool IterationControl<ValueType>::CheckResidual(const ValueType res, const int index) {

  this->current_index_ = index;
  return this->CheckResidual(res);

}

template <typename ValueType>
void IterationControl<ValueType>::RecordHistory(void) {

  this->rec_ = true;

}

template <typename ValueType>
void IterationControl<ValueType>::Verbose(const int verb) {

  this->verb_ = verb;

}


template <typename ValueType>
void IterationControl<ValueType>::WriteHistoryToFile(const std::string filename) const {

  std::ofstream file;
  std::string line;

  assert(this->residual_history_.size() > 0);
  assert(this->iteration_ > 0);

  LOG_INFO("Writing residual history to filename = "<< filename << "; writing...");


  file.open(filename.c_str(), std::ifstream::out );
  
  if (!file.is_open()) {
    LOG_INFO("Can not open file [write]:" << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }
      
  file.setf(std::ios::scientific);

  for (int n=0; n<this->iteration_; n++)
    file << this->residual_history_[n] << std::endl;

  file.close();

  LOG_INFO("Writing residual history to filename = "<< filename << "; done");

}

template <typename ValueType>
void IterationControl<ValueType>::PrintInit(void) {

  LOG_INFO("IterationControl criteria: "
           << "abs tol=" << this->absolute_tol_ << "; "
           << "rel tol=" << this->relative_tol_ << "; "
           << "div tol=" << this->divergence_tol_ << "; "
           << "max iter=" << this->maximum_iter_);

}

template <typename ValueType>
void IterationControl<ValueType>::PrintStatus(void) {

  switch (reached_) {
    
  case 1:
    LOG_INFO("IterationControl ABSOLUTE criteria has been reached: "
             << "res norm=" << fabs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  case 2:
    LOG_INFO("IterationControl RELATIVE criteria has been reached: "
             << "res norm=" << fabs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  case 3:
    LOG_INFO("IterationControl DIVERGENCE criteria has been reached: "
             << "res norm=" << fabs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  case 4:
    LOG_INFO("IterationControl MAX ITER criteria has been reached: "
             << "res norm=" << fabs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;



  default:
    LOG_INFO("IterationControl NO criteria has been reached: "
             << "res norm=" << fabs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);

  }

}


template class IterationControl<double>;
template class IterationControl<float>;

}

