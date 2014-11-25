//  Copyright (C) 2000--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#include "bcs.h"
#include <iostream>
#include <limits>
#include <more/math/math.h>


namespace more {
namespace phys {

  template<typename T>
    void
    BCS_solver<T>::write_to(std::ostream& os) const
    {
	os << "BCS Solver\n"
	   << "  N = " << m_n_occ << ", G = " << m_G;
	if (m_do_smooth_cut) {
	    os << ", cutoff = ["
		 << m_cut_low << " ± " << m_cut_width << ", "
		 << m_cut_high << " ± " << m_cut_width << "]";
	}
	os << '\n';
	if (m_is_solved) {
	    os << "  Δ = " << m_Delta << ", λ = " << m_lambda
	       << ", E_pair = " << m_E_pair << " ("
	       << m_E_pair_nocorr << " uncor)\n";
	}
    }

  template<typename T>
    void
    BCS_solver<T>::solve()
    {
	size_type n_space = m_energies.size();
	if (m_n_occ%2 != 0)
	    throw std::invalid_argument("more::phys::BCS_solver: "
					"N must be even");
	if (m_n_occ <= 0)
	    throw std::invalid_argument("more::phys::BCS_solver: "
					"N must be positive");
	if (m_n_occ > n_space*2)
	    throw std::invalid_argument(
		"more::phys::BCS_solver: "
		"N must be less or equal to the number of states");

	// Assign an initial value to λ and Δ.
	{
	    energy_iterator it_above = m_energies.begin();
	    advance(it_above, m_n_occ/2);
	    energy_iterator it_below = it_above;
	    --it_below;
	    m_lambda = *it_above + *it_below;
	    m_lambda /= 2;
	}
	real_type dE_avg = (*--m_energies.end() - *m_energies.begin())
	    / m_energies.size();
	m_Delta = m_G;
	m_Delta2 = m_Delta*m_Delta;
	if (m_is_verbose)
	    std::cerr << "Starting with  Δ = " << m_Delta << ", λ = "
		      << m_lambda << '\n';

	// Update Δ and λ iteratively.
	int n_iter = 0;
	real_type gap;
	real_type num;
	real_type dlambda;
	real_type dDelta;
	do {
	    real_type sum_eml = 0;	// ∑ (ε - λ)⋅((ε - λ)↑2 + Δ↑2)↑(-1/2)
	    real_type sum3 = 0;		// ∑ ((ε - λ)↑2 + Δ^2)↑(-3/2)
	    real_type sum3_eml = 0;	// ∑ (ε - λ)⋅((ε - λ)↑2 + Δ↑2)↑(-3/2)
	    real_type sum_f = 0;	// ∑ f⋅((ε - λ)↑2 + Δ↑2)↑(-1/2)
	    real_type sum3_f = 0;	// ∑ f⋅((ε - λ)↑2 + Δ↑2)↑(-3/2)
	    real_type sum3_eml_f = 0;	// ∑ f⋅(ε - λ)⋅((ε - λ)↑2 + Δ↑2)↑(-3/2)
	    for (energy_iterator it = m_energies.begin();
		 it != m_energies.end(); ++it) {
		real_type eml = *it - m_lambda;
		real_type f = internal_cut_function(eml);
		real_type term = 1.0/sqrt(eml*eml + m_Delta2);
		real_type term3 = term*term*term;
		sum_eml += term*eml;
		sum3 += term3;
		sum3_eml += term3*eml;
		sum_f += term*f;
		sum3_f += term3*f;
		sum3_eml_f += term3*eml*f;
	    }

	    gap = sum_f - 2/m_G;		// The gap equation
	    num = n_space - m_n_occ - sum_eml;	// The number equation
	    real_type dgap_dDelta = -m_Delta*sum3_f;		// dgap/dΔ
	    real_type dgap_dlambda = sum3_eml_f;		// dgap/dλ
	    real_type dnum_dDelta = m_Delta*sum3_eml;		// dnum/dΔ
	    real_type dnum_dlambda = m_Delta2*sum3;		// dnum/dλ
	    real_type fac
		= 1.0/(dgap_dDelta*dnum_dlambda - dgap_dlambda*dnum_dDelta);
	    m_lambda += dlambda =  (gap*dnum_dDelta - num*dgap_dDelta)*fac;
	    m_Delta  += dDelta  = -(gap*dnum_dlambda - num*dgap_dlambda)*fac;
	    m_Delta2 = m_Delta*m_Delta;

	    ++n_iter;
	    if (!math::is_finite(m_Delta) || m_Delta < 0.0
		|| n_iter >= m_n_iter_max) {
		std::cerr
		    << "*** solve_gap_equation() did not converge after "
		    << n_iter << " iterations.\n"
		    << "*** λ = " << m_lambda << " ± "<< std::fabs(dlambda)
		    << ", Δ = " << m_Delta << " ± " << std::fabs(dDelta)
		    << "\n*** gap = " << gap << ", num = " << num << '\n';
		break;
	    }
	    if (std::fabs(m_Delta) < dE_avg*m_prec) {
		m_Delta = m_Delta2 = 0;
		break;
	    }
	    if (m_is_verbose)
		std::cerr << "    iteration: Δ = " << m_Delta << ", λ = "
			  << m_lambda << '\n';
	}
	while (math::pow2(m_G*gap) + math::pow2(num/m_n_occ) > m_prec ||
	       std::fabs(dlambda)/(std::fabs(m_lambda) + m_prec) > m_prec ||
	       std::fabs(dDelta)/(std::fabs(m_Delta) + m_prec) > m_prec);
	m_is_solved = true;

	// Determine pairing energy
	m_E_pair = 0;
	m_E_pair_nocorr = 0;
	if (m_Delta != 0) {
	    for (energy_iterator it = m_energies.begin();
		 it != m_energies.end(); it++) {
		real_type w = sqr_v(*it);
		real_type f = internal_cut_function(*it - m_lambda);
		m_E_pair += f*sqrt(w*(1-w));
		m_E_pair_nocorr += sqrt(w*(1-w));
	    }
	    m_E_pair *= -m_G*m_E_pair;
	    m_E_pair_nocorr *= -m_G*m_E_pair_nocorr;
	}
    }

  template <typename T>
    void
    BCS_solver<T>::set_smooth_cutoff(real_type cut_low, real_type cut_high,
				     real_type cut_width)
    {
	m_cut_low = cut_low;
	m_cut_high = cut_high;
	m_cut_width = cut_width;
	m_do_smooth_cut = true;
    }

  template <typename T>
    bool
    BCS_solver<T>::smooth_cut(real_type& cut_low, real_type& cut_high,
			      real_type& cut_width)
    {
	cut_low = m_cut_low;
	cut_high = m_cut_high;
	cut_width = m_cut_width;
	return m_do_smooth_cut;
    }

  template <typename T>
    T BCS_solver<T>::s_prec_default
    = std::numeric_limits<T>::epsilon() * 10.0;

  template struct BCS_solver<float>;
  template struct BCS_solver<double>;
//   template struct BCS_solver<long double>;

}} // more::phys

// Local Variable:
// coding: utf-8
// End:
