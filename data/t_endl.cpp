// ==============================================================
//
//  Copyright (C) 2002-2004 Alex Vinokur.
//
//  ------------------------------------------------------------
//  This file is part of C/C++ Program Perfometer.
//
//  C/C++ Program Perfometer is free software; 
//  you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License,
//  or (at your option) any later version.
//
//  C/C++ Program Perfometer is distributed in the hope 
//  that it will be useful, but WITHOUT ANY WARRANTY; 
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with C/C++ Program Perfometer; 
//  if not, write to the Free Software Foundation, Inc., 
//  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ------------------------------------------------------------
// 
//  http://up.to/alexv
//
// ==============================================================


// ##############################################################
//
//  SOFTWARE : C/C++ Program Perfometer
//  FILE     : t_endl.cpp
//
//  DESCRIPTION :
//         Implementation of measured/compared functions
//
// ##############################################################


// ===============
#include "tests.h"
// ===============



// #########################################
// #
// #  Functions To Be Measured (Compared)
// #  Prototypes are in file tests.h
// #
// #########################################



// =============================
void endl_action (void)
{
const string h1text__endl                   ("endl");
const string h1text__backclash_n_string     ("\"\\n\"");
const string h1text__backclash_n_char       ("\'\\n\'");

const string h2text__stdout                 ("stdout       ");
const string h2text__stderr                 ("stderr       ");
const string h2text__cout                   ("cout         ");
const string h2text__cerr                   ("cerr         ");
const string h2text__clog                   ("clog         ");
const string h2text__oss                    ("ostringstream");
const string h2text__fout                   ("cout-to-file ");
const string h2text__ferr                   ("cerr-to-file ");
const string h2text__flog                   ("clog-to-file ");


  // ------------------------------------------
  {
    TURN_ON_DEFAULT_TIMER (h2text__stdout + " " + h1text__backclash_n_string, "No") 
    {
      printf ("\n");
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (h2text__stdout + " " + h1text__backclash_n_char, "No") 
    {
      printf ("%c", '\n');
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (h2text__cout + " " + h1text__endl, "No") 
    {
      cout << endl;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (h2text__cout + " " + h1text__backclash_n_string, "No") 
    {
      cout << "\n";
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (h2text__cout + " " + h1text__backclash_n_char, "No") 
    {
      cout << '\n';
    }
  }


  // ------------------------------------------
  {
    TURN_ON_DEFAULT_TIMER (h2text__stderr + " " + h1text__backclash_n_string, "No") 
    {
      fprintf (stderr, "\n");
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (h2text__stderr + " " + h1text__backclash_n_char, "No") 
    {
      fprintf (stderr, "%c", '\n');
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (h2text__cerr + " " + h1text__endl, "No") 
    {
      cerr << endl;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (h2text__cerr + " " + h1text__backclash_n_string, "No") 
    {
      cerr << "\n";
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (h2text__cerr + " " + h1text__backclash_n_char, "No") 
    {
      cerr << '\n';
    }
  }



  // ------------------------------------------
  {
    TURN_ON_DEFAULT_TIMER (h2text__clog + " " + h1text__endl, "No") 
    {
      clog << endl;
    }
  }

  {
    TURN_ON_DEFAULT_TIMER (h2text__clog + " " + h1text__backclash_n_string, "No") 
    {
      clog << "\n";
    }
  }


  {
    TURN_ON_DEFAULT_TIMER (h2text__clog + " " + h1text__backclash_n_char, "No") 
    {
      clog << '\n';
    }
  }



  // ------------------------------------------
  {
    ostringstream oss;
    TURN_ON_DEFAULT_TIMER (h2text__oss + " " + h1text__endl, "No") 
    {
      oss << endl;
    }
  }

  {
    ostringstream oss;
    TURN_ON_DEFAULT_TIMER (h2text__oss + " " + h1text__backclash_n_string, "No") 
    {
      oss << "\n";
    }
  }


  {
    ostringstream oss;
    TURN_ON_DEFAULT_TIMER (h2text__oss + " " + h1text__backclash_n_char, "No") 
    {
      oss << '\n';
    }
  }


#define FOUT_FILE_NAME	"fout.txt"
#define FERR_FILE_NAME	"ferr.txt"
#define FLOG_FILE_NAME	"flog.txt"

  // ------------------------------------------
  {
    streambuf*  save_sbuf_cout;
    ofstream    fout;

    fout.open (FOUT_FILE_NAME);
    assert (fout.is_open());
    save_sbuf_cout = cout.rdbuf();
    assert (save_sbuf_cout);
    cout.rdbuf(fout.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__fout + " " + h1text__endl, "No") 
    {
      fout << endl;
    }

    cout.rdbuf(save_sbuf_cout);
    assert (fout.is_open());
    fout.close();
    assert (!fout || (fout && !fout.is_open()));
    remove (FOUT_FILE_NAME);
  }

  {
    streambuf*  save_sbuf_cout;
    ofstream    fout;

    fout.open (FOUT_FILE_NAME);
    assert (fout.is_open());
    save_sbuf_cout = cout.rdbuf();
    assert (save_sbuf_cout);
    cout.rdbuf(fout.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__fout + " " + h1text__backclash_n_string, "No") 
    {
      fout << "\n";
    }

    cout.rdbuf(save_sbuf_cout);
    assert (fout.is_open());
    fout.close();
    assert (!fout || (fout && !fout.is_open()));
    remove (FOUT_FILE_NAME);
  }


  {
    streambuf*  save_sbuf_cout;
    ofstream    fout;

    fout.open (FOUT_FILE_NAME);
    assert (fout.is_open());
    save_sbuf_cout = cout.rdbuf();
    assert (save_sbuf_cout);
    cout.rdbuf(fout.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__fout + " " + h1text__backclash_n_char, "No") 
    {
      fout << '\n';
    }

    cout.rdbuf(save_sbuf_cout);
    assert (fout.is_open());
    fout.close();
    assert (!fout || (fout && !fout.is_open()));
    remove (FOUT_FILE_NAME);

  }



  // ------------------------------------------
  {
    streambuf*  save_sbuf_cerr;
    ofstream    ferr;

    ferr.open (FERR_FILE_NAME);
    assert (ferr.is_open());
    save_sbuf_cerr = cerr.rdbuf();
    assert (save_sbuf_cerr);
    cerr.rdbuf(ferr.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__ferr + " " + h1text__endl, "No") 
    {
      ferr << endl;
    }

    cerr.rdbuf(save_sbuf_cerr);
    assert (ferr.is_open());
    ferr.close();
    assert (!ferr || (ferr && !ferr.is_open()));
    remove (FERR_FILE_NAME);

  }

  {
    streambuf*  save_sbuf_cerr;
    ofstream    ferr;

    ferr.open (FERR_FILE_NAME);
    assert (ferr.is_open());
    save_sbuf_cerr = cerr.rdbuf();
    assert (save_sbuf_cerr);
    cerr.rdbuf(ferr.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__ferr + " " + h1text__backclash_n_string, "No") 
    {
      ferr << "\n";
    }

    cerr.rdbuf(save_sbuf_cerr);
    assert (ferr.is_open());
    ferr.close();
    assert (!ferr || (ferr && !ferr.is_open()));
    remove (FERR_FILE_NAME);
  }


  {
    streambuf*  save_sbuf_cerr;
    ofstream    ferr;

    ferr.open (FERR_FILE_NAME);
    assert (ferr.is_open());
    save_sbuf_cerr = cerr.rdbuf();
    assert (save_sbuf_cerr);
    cerr.rdbuf(ferr.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__ferr + " " + h1text__backclash_n_char, "No") 
    {
      ferr << '\n';
    }

    cerr.rdbuf(save_sbuf_cerr);
    assert (ferr.is_open());
    ferr.close();
    assert (!ferr || (ferr && !ferr.is_open()));
    remove (FERR_FILE_NAME);

  }



  // ------------------------------------------
  {
    streambuf*  save_sbuf_clog;
    ofstream    flog;

    flog.open (FLOG_FILE_NAME);
    assert (flog.is_open());
    save_sbuf_clog = clog.rdbuf();
    assert (save_sbuf_clog);
    clog.rdbuf(flog.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__flog + " " + h1text__endl, "No") 
    {
      flog << endl;
    }

    clog.rdbuf(save_sbuf_clog);
    assert (flog.is_open());
    flog.close();
    assert (!flog || (flog && !flog.is_open()));
    remove (FLOG_FILE_NAME);
  }

  {
    streambuf*  save_sbuf_clog;
    ofstream    flog;

    flog.open (FLOG_FILE_NAME);
    assert (flog.is_open());
    save_sbuf_clog = clog.rdbuf();
    assert (save_sbuf_clog);
    clog.rdbuf(flog.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__flog + " " + h1text__backclash_n_string, "No") 
    {
      flog << "\n";
    }

    clog.rdbuf(save_sbuf_clog);
    assert (flog.is_open());
    flog.close();
    assert (!flog || (flog && !flog.is_open()));
    remove (FLOG_FILE_NAME);
  }


  {
    streambuf*  save_sbuf_clog;
    ofstream    flog;

    flog.open (FLOG_FILE_NAME);
    assert (flog.is_open());
    save_sbuf_clog = clog.rdbuf();
    assert (save_sbuf_clog);
    clog.rdbuf(flog.rdbuf());

    TURN_ON_DEFAULT_TIMER (h2text__flog + " " + h1text__backclash_n_char, "No") 
    {
      flog << '\n';
    }

    clog.rdbuf(save_sbuf_clog);
    assert (flog.is_open());
    flog.close();
    assert (!flog || (flog && !flog.is_open()));
    remove (FLOG_FILE_NAME);
  }



} // endl_action


///////////////
// End-Of-File
///////////////

