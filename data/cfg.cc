#include <more/sys/cfg.h>
#include <more/bits/conf.h>
#include <more/bits/automakedefs.h>
#include <more/io/filesys.h>

namespace more {
namespace sys {

  namespace {
    char const* cfgarr[cfg_end];
  }

  char const*
  getcfg(cfg_t i)
  {
      return cfgarr[i];
  }

  void
  setcfg(cfg_t i, char const* s)
  {
      cfgarr[i] = s;
  }

  struct init_cfg_type { init_cfg_type(); } init_cfg;
  init_cfg_type::init_cfg_type()
  {
      // Installation directories
      setcfg(cfg_PREFIX, MORE_CONF_PREFIX);
      setcfg(cfg_EXEC_PREFIX, MORE_CONF_EXEC_PREFIX);

      setcfg(cfg_BINDIR, MORE_CONF_BINDIR);
      setcfg(cfg_SBINDIR, MORE_CONF_SBINDIR);
      setcfg(cfg_LIBEXECDIR, MORE_CONF_LIBEXECDIR);

      setcfg(cfg_DATADIR, MORE_CONF_DATADIR);
      setcfg(cfg_SYSCONFDIR, MORE_CONF_SYSCONFDIR);
      setcfg(cfg_SHAREDSTATEDIR, MORE_CONF_SHAREDSTATEDIR);
      setcfg(cfg_LOCALSTATEDIR, MORE_CONF_LOCALSTATEDIR);
      setcfg(cfg_LIBDIR, MORE_CONF_LIBDIR);
      setcfg(cfg_INCLUDEDIR, MORE_CONF_INCLUDEDIR);

      // Programs
      if (more::io::file_status(LIBTOOL_INST).exists())
	  setcfg(cfg_LIBTOOL, LIBTOOL_INST);
      else
	  setcfg(cfg_LIBTOOL, LIBTOOL_BLD);
      setcfg(cfg_CC, CC);
      setcfg(cfg_CPP, CPP);
      setcfg(cfg_CXX, CXX);
      setcfg(cfg_CXXCPP, CXXCPP);
      setcfg(cfg_CCLD, CCLD);
      setcfg(cfg_CXXLD, CXXLD);
      setcfg(cfg_INSTALL, INSTALL);
      setcfg(cfg_INSTALL_DATA, INSTALL_DATA);
      setcfg(cfg_INSTALL_PROGRAM, INSTALL_PROGRAM);
  }
}}
