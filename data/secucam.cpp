/*
 * A little and dumb security camera application with the LTI-Lib.
 * Revision list:
 * $Id: secucam.cpp,v 1.4 2005/10/07 17:02:52 ltilib Exp $
 */

#include <ltiToUCam.h>

#ifdef _USE_PHILIPS_TOUCAM

#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <iostream>

#include <ltiRGBPixel.h>
#include <ltiJPEGFunctor.h>
#include <ltiViewer.h>
#include <ltiDownsampling.h>
#include <ltiTemporalTemplate.h>
#include <ltiLispStreamHandler.h>
#include <ltiDraw.h>

// this is for killing
#if __linux__
#include <sys/types.h>
#include <signal.h>
#endif

using std::endl;
using std::cout;
using std::cerr;

const std::string paramfile="tocam.dat";

void parseArgs(int argc, char*argv[], int& delay, bool& view, float& thresh,
               bool& killmode, bool& color, bool& timestamp) {
  delay=3;
  view=false;
  thresh=200.0f;
  killmode=false;
  color=false;
  timestamp=true;
  for (int i=1; i<argc; i++) {
    if (*argv[i] == '-' || *argv[i] == '+') {
      //bool val=(*argv[i] == '+');
      switch (argv[i][1]) {
        // the minimum delay between taking two images
        case 's': delay=atoi(argv[++i]); break;
        // the threshold for considering two images different
        case 't': thresh=atof(argv[++i]); break;
        // enable the viewer
        case 'v': view=true; break;
        // enable color mode
        case 'c': color=true; break;
#if __linux__
        // kill the runnign secucam process
        case 'k': killmode=true; break;
#endif
        // do not print time stamps
        case 'p': timestamp=false; break;
        default: break;
      }
    } else {
      //
    }
  }
}


void readParameters(lti::toUCam::parameters& p, const std::string& file) {
  // read camera parameter file
  std::ifstream gpf(file.c_str());
  if (!gpf.is_open() || !gpf.good()) {
    // not found -> make one!
    std::ofstream gpf2(file.c_str());
    lti::lispStreamHandler lsp(gpf2);
    p.write(lsp);
    gpf2.close();
    std::cerr << "Parameter file " << file << " not found, created it!\n";
   } else {
    lti::lispStreamHandler lsp(gpf);
    if (!p.read(lsp)) {
      std::cerr << "Parameter read failed\n";
      exit(1);
    }
    gpf.close();
  }
  
}


void processKey(int key, lti::toUCam::parameters p, lti::toUCam& grabber, 
                bool& view, bool& quit, lti::viewer& v) {
  static int oldKey=-1;
  if (key == oldKey) {
    return;
  }
  switch (key) {
    case 'r': // re-read parameter file
      {
        readParameters(p,paramfile);
        grabber.setParameters(p);
        break;
      }
    case 'h': // hide viewer
      {
        view=false;
        v.hide();
        break;
      }
    case 'q': // quit program
      {
        quit=true;
        break;
      }
    default:
      // ignore
      break;
    }
  oldKey=key;
}

#if __linux__
int accessLock(bool write=false) {
  std::string home;
  int pid=getpid();
  const char* tmp=getenv("HOME");
  if (tmp == NULL || strlen(tmp) == 0) {
    home="/var/tmp/.secucam.lock";
  } else {
    home=tmp;
    home+="/.secucam.lock";
  }
  if (write) {
    std::ifstream in(home.c_str());
    if (in.is_open()) {
      // there is an old lock file left, refuse to start
      std::cerr << "Fatal error: There already is a lock file" << std::endl;
      pid=-1;
    } else {
      std::ofstream out(home.c_str());
      if (out.is_open() && out.good()) {
        out << pid << std::endl;
      } else {
        std::cerr << "Fatal error: cannot write lock file " << home << std::endl;
        pid=-1;
      }    
    }
  } else {
    std::ifstream in(home.c_str());
    if (in.is_open() && in.good()) {
      in >> pid;
    } else {
      std::cerr << "Fatal error: cannot read lock file " << home << std::endl;
      std::cerr << "Maybe there is no secucam process running." << std::endl;
      pid=-1;
    }
    in.close();
    unlink(home.c_str());
  }
  return pid;
}
#endif

bool getImage(lti::frameGrabber& grabber, lti::image& result, int retries,
              const int delay) {
  while (retries-- > 0) {
    if (grabber.apply(result)) {
      return true;
    }
    if (retries > 0) {
      sleep(delay);
    }
  }
  return false;
}


int main(int argc,char *argv[]) {

  int delay;
  bool view,killmode,colormode,showtime;
  float thresh;

  parseArgs(argc,argv,delay,view,thresh,killmode,colormode,showtime);

#if __linux__
  if (killmode) {
    // in kill mode, we look for a currently running secucam task
    // and try to kill it
    // WARNING: This is completely operating system-dependent

    int pid=accessLock();
    if (pid < 0) {
      return 1;
    } else {
      std::cerr << "Killing secucam process " << pid << std::endl;
      kill(pid,SIGTERM);
      return 0;
    }
  } else {
    int pid=accessLock(true);
    if (pid < 0) {
      return 1;
    }
  }
#endif

  lti::toUCam grabber;
  lti::toUCam::parameters p;

  readParameters(p,paramfile);

  lti::image tmp;
  lti::channel8 gray;

  // compute absolute threshold value
  // so the used can specify the value independent of the image
  // size
  if (thresh > std::numeric_limits<float>::epsilon()) {
    thresh=p.size.x*p.size.y/thresh;
  } else {
    thresh=0;
  }

  grabber.setParameters(p);
	//lti::lispStreamHandler mod(std::cerr);
	//grabber.getParameters().write(mod);
  lti::saveJPEG saver;

  char buf[32];
  char buf2[32];

  lti::draw<lti::rgbPixel> drawer;
  lti::temporalTemplate mhi;
  lti::downsampling sampler;
  lti::channel result;
  bool init=true;
  lti::image tmp2;

  struct tm timestamp,timestamp2;
  int frame=0;
  time_t ttime;
  lti::viewer viewer("LTIlib security camera demo");

  bool quit=false;

  // main loop
  while (getImage(grabber,tmp,3,1) && !quit) {
    time(&ttime);
    struct tm* t=localtime_r(&ttime,&timestamp);
    strcpy(buf2,asctime(t));
    buf2[strlen(buf2)-1]='\0';

    sampler.apply(tmp,tmp2);
    mhi.apply(tmp2,result);

    float s=result.sumOfElements();
    if (s > thresh || init) {

      if (showtime) {
        // draw time stamp into image
        drawer.use(tmp);
        drawer.setColor(lti::Black);
        drawer.box(0,0,192,8,true);
        drawer.setColor(lti::White);
        drawer.text(buf2);
      }

      init=false;
      // compare the time stamps
      if (memcmp(&timestamp,&timestamp2,sizeof(timestamp)) == 0) {
        // equal -> use frame number for image file name
        sprintf(buf,"/tmp/tou/%04d%02d%02d-%02d%02d%02d_%02d.jpg",
              t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,++frame);
      } else {
        // different -> no need for frame no.
        sprintf(buf,"/tmp/tou/%04d%02d%02d-%02d%02d%02d.jpg",
              t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
        memcpy(&timestamp2,&timestamp,sizeof(timestamp));
        frame=0;
      }
      // save image
      if (colormode) {
        saver.save(buf,tmp);
      } else {
        gray.castFrom(tmp);
        saver.save(buf,gray);
      }
    } else {
      // no movement->make image black so viewer does not show anything
      tmp.fill(lti::Black);
    }
    if (delay > 0) {
      sleep(delay);
    }
    if (view) {
      viewer.show(tmp);
      processKey(viewer.lastKey(),p,grabber,view,quit,viewer);
    }
  }
  if (!quit) {
    // grabbing failed
    std::cerr << "Grabbing failed: " << grabber.getStatusString() << "\n";
    exit(1);
  }
  ::exit(0);
}

#else

#include <iostream>

using std::endl;
using std::cout;
using std::cerr;

// no toUCam camera found
int main(int argc,char *argv[]) {
  std::cout<<"Sorry, but no toUCam support found with your LTI-Lib"<<endl;
  std::cout<<"You may want to recompile it enabling the camera support in "
           << "ltilib/src/io/ltiHardware.h" << endl;
  return 1;
}
#endif

