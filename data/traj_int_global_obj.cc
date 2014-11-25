#include "traj_int_global_obj.h"

traj_int_global_obj::traj_int_global_obj(char *Nfile, char *Sfile) {

  N_hemi=new traj_int_obj(Nfile);
  S_hemi=new traj_int_obj(Sfile);

}

traj_int_global_obj::

