//  Copyright (C) 2001--2009  Petter Urkedal
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


#include <more/phys/ens.h>
#include <more/io/cmdline.h>

#if 1
#  define D_STMT(stmt) (stmt)
#else
#  define D_STMT(stmt) ((void)0)
#endif



namespace more {
namespace phys {
namespace ens {

  confiv_t
  four_point_pairing(nucleus const& nucl, more::math::pm_half mt)
  {
      // XXX n_part(1-mt) must be even.
      nucleus iso[3];
      rec_qvalue const* qv[3];
      for (int i = -1; i <= 1; ++i) {
	  iso[i+1] = neighbor(nucl, i*(math::half-mt), i*(math::half+mt));
	  if (!(qv[i+1] = iso[i+1].qvalue())) {
	      D_STMT(std::cerr << "No Q-Value for " << iso[i+1] << std::endl);
	      return confiv_t();
	  }
      }
      if (more::io::cmdline::verbose()) {
	  std::clog << "Calculating Δ⌄"
		    << (mt == math::half? 'p' : 'n') << " from\n";
	  for (int i = 0; i < 3; ++i)
	      std::clog << "\tS(" << iso[i] << ", " << mt << ") = "
			<< qv[i]->S(mt)/SI::MeV << " MeV\n";
      }
      double coeff[] = { .25, -.5, .25 };
      phys::confidence_interval<double>
	  Delta(0.0, 0.0, 0.0, phys::origin_theory);
      for (int i = 0; i < 3; ++i)
	  Delta += coeff[i]*qv[i]->S(mt);
      if (iso[0].n_part(mt) % 2)
	  return -Delta;
      else
	  return Delta;
  }


  dataset const*
  adopted_levels_dataset(nucleus const& nucl)
  {
      // Find the ADOPTED LEVELS dataset
      ens::nucleus::dataset_iterator it_ds
	  = nucl.dataset_begin();
      while (it_ds != nucl.dataset_end()) {
	  std::string idn = it_ds->ident()->dataset_id_string();
	  if (idn.size() >= 14 &&
	      idn.substr(0, 14) == "ADOPTED LEVELS")
	      break;
	  ++it_ds;
      }
      if (it_ds != nucl.dataset_end())
	  return &*it_ds;
      else
	  return 0;
  }

  rec_level const*
  ground_state_level(nucleus const& nucl)
  {
      // Find the ground state level
      if (dataset const* ds = adopted_levels_dataset(nucl)) {
	  for (ens::dataset::level_iterator
		   it_lev = ds->level_begin();
	       it_lev != ds->level_end(); ++it_lev) {
	      if (it_lev->E_minus_E_ref().is_known() &&
		  it_lev->i_E_ref() == ens::energy_index_none &&
		  it_lev->E_minus_E_ref().cent() == 0)
		  return &*it_lev;
	  }
      }
      return 0;
  }


  confiv_t
  half_life(nucleus const& nucl)
  {
      if (rec_level const* lev = ground_state_level(nucl))
	  return lev->T_half();
      return confiv_t();
  }

}}} // more::phys::ens

// Local Variables:
// coding: utf-8
// End:
