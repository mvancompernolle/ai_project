/*
 * Copyright (C) 1998, 1999, 2000, 2001
 * Lehrstuhl fuer Technische Informatik, RWTH-Aachen, Germany
 *
 * This file is part of the LTI-Computer Vision Library (LTI-Lib)
 *
 * The LTI-Lib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * The LTI-Lib is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the LTI-Lib; see the file LICENSE.  If
 * not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

// -------------------------------------------------------------------------
// Pareto Front-based Image-Based Segmentation Evaluation (Batch Mode)
//
// This example provides several functions:
//
// 1. Evaluates with the PESA algorithm the IBS segmentation.  This takes hours
//    or days or even weeks if you want to...
// 2. Takes the output file *.log.pf of 1. and generates a table with all
//    fitness function columns.  At this time there are seven columns:
//    1. Number of Regions
//    2. Reciprocal of Number of Regions (mean of normalized region size)
//    3. Pixel-wise potential accuracy
//    4. Region-wise potential accuracy (maybe buggy?)
//    5. Region-wise information content
//    6. Throughput
//    7. Region integrity
//    This has to compute the evaluation for all members of the front and can
//    take several minutes of hours, depending on the algorithm used.
// 3. Takes the output file *.log.pf and generates a huge tables with all
//    parameter groups specified.
//    This is just a parsing task, that should be ready in seconds.
//
// -------------------------------------------------------------------------

#include <ltiRGBPixel.h>
#include "ltiCWAGMSegmentation.h"
#include "ltiCWAGMSegmentationEvaluation.h"
#include "ltiProgressInfo.h"
#include "ltiImage.h"
#include "ltiDraw.h"
#include "ltiViewer.h"
#include "ltiLispStreamHandler.h"
#include "ltiALLFunctor.h"
#include "ltiException.h"
#include "ltiSerialStatsFunctor.h"
#include <iostream>
#include <string>
#include <fstream>

using std::cout;
using std::cerr;
using std::endl;

namespace lti {

  //
  // ^^^^^ Main documentation above!! ^^^^^
  // 

  /**
   * The real class
   */
  class tester {
  public:
    void operator()(int argc,char *argv[]);
  };

  /**
   * Pareto Front domination
   */
  bool dominate(const lti::dvector& a,
                const lti::dvector& b,
                const lti::ivector& idx) {
    bool theOne = false;
    
    // a little pointer arithmetic to accelerate this frequently called
    // function
    int i,j;
    for (i=0;i<idx.size();++i) {
      j=idx.at(i);
      if (a[j] < b[j]) {
        // if any element is smaller => definitively not greater!
        return false;
      } else if (a[j] > b[j]) {
        // only greater if at least one element has been strictly greater
        theOne = true;
      }
    }
    
    return theOne;
  }
  
  /**
   * Get non-dominated elements in the matrix
   * Return the size of the front (num of elements with true
   */
  int getNonDominated(const lti::dmatrix& mat,
                      const lti::ivector& idx,
                      std::vector<bool>& nonDom) {
    int i,j;
    bool dominated;
    int n = 0;
    nonDom.resize(mat.rows());
    for (i=0;i<mat.rows();++i) {
      dominated = false;
      for (j=0;!dominated && (j<mat.rows());++j) {
        dominated = dominate(mat.getRow(j),mat.getRow(i),idx);
      }
      nonDom[i] = !dominated;
      if (nonDom[i]) {
        n++;
      }
    }

    return n;
  }
  
  void plotIndividuals(const dmatrix& bbox,
                       const dmatrix& indivs,
                       image& canvas,
                       const rgbPixel& color = White,
                       const char symbol = 'o',
                       const bool line=false) {
    
    static const int border = 8;

    draw<rgbPixel> painter;
    painter.use(canvas);

    // set colors and symbols
    painter.setColor(color);
    std::string str = "o";
    str[0]=symbol;
    painter.setStyle(str.c_str());

    double mx,my,bx,by;
    int j;
    mx = (canvas.lastColumn()-2*border)/(bbox.at(1,0)-bbox.at(0,0));
    bx = border-bbox.at(0,0)*mx;

    my = (canvas.lastRow()-2*border)/(bbox.at(0,1)-bbox.at(1,1));
    by = border-bbox.at(1,1)*my;
    
    point p;
    const rgbPixel lineColor = rgbPixel(64,128,256);

    if ((indivs.rows() > 0) && line) {
      p.x = iround(mx*indivs.at(0,0) + bx);
      p.y = iround(my*indivs.at(0,1) + by);
      
      painter.setColor(lineColor);
      painter.set(p);
      painter.setColor(color);
    }


    for (j=0;j<indivs.rows();++j) {
      p.x = iround(mx*indivs.at(j,0) + bx);
      p.y = iround(my*indivs.at(j,1) + by);

      if (line) {
        painter.setColor(lineColor);
        painter.lineTo(p);
        painter.setColor(color);        
      }

      painter.marker(p);
    }
  };

  // read pareto front file 
  bool 
  readParetoFront(const std::string& file,
                  std::vector<dvector>& subFitness,
                  std::vector<cwagmSegmentation::parameters>& parSet) {
    std::ifstream in(file.c_str());
    if (in) {
      lispStreamHandler lsh(in);

      parSet.clear();
      parSet.reserve(250);

      subFitness.clear();
      subFitness.reserve(250);

      // read all parameterization into parSet
      while (lsh.readBegin()) {
        // two possibilities:
        // 1. BoundingBox symbol (the last one), or
        // 2. A list with the vector of interest and the parameterization
        if (lsh.tryBegin()) {
          // the vector
          subFitness.push_back(dvector());
          subFitness.back().read(lsh,false); // the open parenthesis was
          // already read
          lsh.readEnd();
          
          // the parameters we care on
          parSet.push_back(cwagmSegmentation::parameters());
          if (!parSet.back().read(lsh)) {
            std::cerr << "Error reading parameterization: \n" 
                      << lsh.getStatusString() << std::endl;
          }
          
          lsh.readEnd(); // of the vector-params block
        } else {
          // read the bounding box and end
          dmatrix bbox;
          lti::read(lsh,"BoundingBox",bbox,false);
          lsh.readEnd();
          break;
        }
      }
    } else {
      return false;
    }

    in.close();
   
    if (parSet.empty()) {
      std::cerr << "No parameterization found in " << file << std::endl;
      return false;
    }
    
    return true;
  }
  
  // create file with parameters table for the given parameters
  bool createParamTable(const std::string& paretoFile) {
    std::vector<cwagmSegmentation::parameters> parSet;
    std::vector<dvector> subFitness;
    
    std::cout << "Creating Parameters Table..."; 
    std::cout.flush();

    // try to open the paretoFile
    if (!readParetoFront(paretoFile.c_str(),subFitness,parSet)) {
      return false;
    }
    
    std::string tableFile = paretoFile + ".table.csv";
    std::ofstream o(tableFile.c_str());

    if (!o) {
      std::cerr << "File " << tableFile << " could not be created.\n";
      return false;
    }

    unsigned int i,j;
    // simple header, to know what is what!
    o << "f1";
    for (i=1;i<static_cast<unsigned int>(subFitness.front().size());++i) {
      o << "\tf" << i+1;
    }

    o << "\tmedian";
    o << "\tcolor";
    o << "\tcontrast";
    o << "\tkernel";
    o << "\tneighbor";
    o << "\tflood";
    o << "\tmergeMode";
    o << "\tmergeThres";
    o << "\tminRegions";
    
    o << std::endl;

    for (i=0;i<parSet.size();++i) {
      // fitness

      o << subFitness[i].at(0);
      for (j=1;j<static_cast<unsigned int>(subFitness[i].size());++j) {
        o << "\t" << subFitness[i].at(j);
      }

      const cwagmSegmentation::parameters& p = parSet[i]; 

      o << "\t" << p.medianParam.kernelSize;
      o << "\t" << p.colorSplitter;         
      o << "\t" << static_cast<int>(p.colorContrastParam.contrastFormat);
      o << "\t" << static_cast<int>(p.colorContrastParam.kernelType);
      o << "\t" << ((p.watershedParam.neighborhood8) ? 8 : 4);
      o << "\t" << p.minProbForWatershedThreshold;
      o << "\t" << static_cast<int>(p.harisRegionMergeParam.mergeMode);
      o << "\t" << p.harisRegionMergeParam.mergeThreshold;
      o << "\t" << p.harisRegionMergeParam.minRegionNumber;
      
      o << std::endl;
    }

    o.close();

    std::cout << "done." << std::endl;
    return true;
  }


  // -------------------------------------------------------------------------
  // tester::operator() : The main program
  // -------------------------------------------------------------------------

  void tester::operator()(int argc,char *argv[]) {

    bool showProgress = true;
    bool showViewer = false;
    bool force = false;
    bool resume = false;
    bool createConfig = true;
    int verboseLevel = 1;
    std::string str;
    int i=0;
    std::string configFile = "config_cwagmeval.dat";
    std::string paretoFile = "";
    std::string paretoTable = "";

    // simple command line parsing
    while (i<argc) {
      str = argv[i];
      if ((str == "-h") || (str == "--help")) {
        std::cout << "Evaluation of image-based segmentation" << std::endl;
        std::cout << "Usage: " << argv[0] 
                  << " [-f] [-h] [-q] [-v] [-c configFile]\n\n";
        std::cout << "  -h This help.\n";
        std::cout << "  -f Force creation, even if files already exist.\n";
        std::cout << "  -v At the end of one evaluation show the front.\n";
        std::cout << "  -r Reassume a broken process.\n";
        std::cout << "  -l [012] Verbose level.\n";
        std::cout << "  -c config_file Use given configuration file.\n";
        std::cout << "  -e paretoFrontFile evaluate parameterizations.\n";
        std::cout << "  -t paretoFrontFile generate a parameter table.\n";
        std::cout << "  -q Do not output the progress of the evaluation.\n";
        return;
      } else if (str == "-q") {
        showProgress = false;
      } else if (str == "-v") {
        showViewer = true;
      } else if (str == "-f") {
        force = true;
      } else if (str == "-r") {
        resume = true;
      } else if (str == "-l") {
        ++i;
        str = argv[i];
        if (!str.empty()) {
          verboseLevel = str[0]-'0';
          std::cout << "Verbose level: " << verboseLevel << std::endl;
        }
      } else if (str.substr(0,2) == "-l") {
        verboseLevel = str[2]-'0';
        std::cout << "Verbose level: " << verboseLevel << std::endl;        
      } else if (str == "-c") {
        ++i;
        if ((i<argc) && (argv[i][0]!='-')) {
          configFile = argv[i];
          createConfig = false;
        }
      } else if (str == "-e") {
        ++i;
        if ((i<argc) && (argv[i][0]!='-')) {
          paretoFile = argv[i];
        }        
      } else if (str == "-t") {
        ++i;
        if ((i<argc) && (argv[i][0]!='-')) {
          paretoTable = argv[i];
        }        
      } 
      
      ++i;
    }

    if (!paretoTable.empty()) {
      createParamTable(paretoTable);
      return;
    }

    // default parameters
    cwagmSegmentationEvaluation::parameters ibseParam;
    ibseParam.numOfIterations = 250;
    ibseParam.sortResult = true;
    ibseParam.logAllEvaluations = true;

    // the batch is simply controlled by the number of parameter objects
    // stored in the configuration file.
    std::vector<cwagmSegmentationEvaluation::parameters> parSet;

    // read parameters

    bool saveParam = true;
    std::ifstream in(configFile.c_str());
    lispStreamHandler lsh;

    if (in) {
      lsh.use(in);
      // try to read one
      saveParam = !ibseParam.read(lsh);

      if (!saveParam) {
        // seems to be allright
        parSet.push_back(ibseParam);

        while (ibseParam.read(lsh)) {
          parSet.push_back(ibseParam);
        }
      }

      in.close();
    }

    // if something went wrong, save parameters for the next time
    if (saveParam) {
      if (createConfig) {
        std::ofstream out(configFile.c_str());
        if (out) {
          lsh.use(out);
          ibseParam.write(lsh);
          out << std::endl;
          out.close();
        }
        
        // Abort
        std::cout << "Configuration could not be read.\n";
        std::cout << "A new configuration file " << configFile 
                  << " has been written.  Please edit\n";
        std::cout << "it and eventually replicate the parameter sets "
                  << "therein\n";
        std::cout << "and start this program again." << std::endl;
        return;
      } else {
        std::cout << "Configuration file " << configFile 
                  << " could not be read.\n"
                  << "Aborting." << std::endl
                  << lsh.getStatusString() << std::endl;
        return;
      }
    }

    // the segmentation evaluation functor
    cwagmSegmentationEvaluation ibse;
    if (showProgress) {
      streamProgressInfo progInfo;
      ibse.setProgressObject(progInfo);
      ibse.getProgressObject().setDetailLevel(verboseLevel);
    }
    viewer* view(0);

    if (showViewer) {
      view = new viewer("Individuals");
    }

    // functor to save images
    saveImage saver;

    std::vector<cwagmSegmentationEvaluation::parameters>::iterator it;

    // ----------------
    // evaluatePareto ?
    // ----------------
    if (!paretoFile.empty()) {
      // evaluate all fitness measures for all points in a pareto front file

      if (!ibse.setParameters(parSet[0])) { // used the first parameters in
        // the configuration for the evaluator
        std::cerr << "Error setting parameters: " 
                  << ibse.getStatusString() << std::endl;
        return;
      }
      
      std::vector<cwagmSegmentation::parameters> parSet;
      std::vector<dvector> subFitness;

      // try to open the paretoFile
      if (readParetoFront(paretoFile.c_str(),subFitness,parSet)) {

        // and evaluate everything!
        static const int totalFitness = 7;
        dmatrix fitness(parSet.size(),totalFitness);
        
        std::cout << "Evaluating " << std::endl;
        unsigned int i;
        for (i=0;i<parSet.size();++i) {
          std::cout << "\r" << i+1 << "/" << parSet.size();
          std::cout.flush();
          if (!ibse.evaluate(parSet[i],fitness.getRow(i))) {
            std::cerr << "Error in evaluation: " 
                      << ibse.getStatusString() << std:: endl;
          }
        }
        std::cout << std::endl;
        
        // -------------
        // data analysis
        // try to guess from which fitnesses this pareto front is made of
        serialStatsFunctor<double> stats;
        stats.considerRows(fitness);
        dvector fmean; // fitness mean
        dvector sfmean; // subFitness mean
        stats.getMean(fmean);
        stats.reset();
        for (i=0;i<subFitness.size();++i) {
          stats.consider(subFitness[i]);
        }
        stats.getMean(sfmean);

        dmatrix normedDiff(3,totalFitness,0.0);
        const int subfsize = subFitness[0].size();
        int j,k;
        // subFitness: std::vector<dvector> with 2 or 3 elements in .pf file
        // fitness   : dmatrix with complete fitnesses
        for (i=0;i<static_cast<unsigned int>(fitness.rows());++i) {
          for (j=0;j<subfsize;++j) {
            for (k=0;k<totalFitness;++k) {
              normedDiff.at(j,k) += abs((subFitness[i][j]/sfmean.at(j)) -
                                        (fitness.at(i,k)/fmean.at(k)));
            }
          }
        }
        
        // now try to get the indices of the smallest errors
        std::cout << "Pareto Front has " << subfsize << " dimensions."
                  << std::endl;
        
        ivector idx(subfsize,0);
        for (k=0;k<subfsize;++k) {
          idx.at(k)=normedDiff.getRow(k).getIndexOfMinimum();
          std::cout << "  Fitness " << k+1<< ": " << idx.at(k)+1 << std::endl;
        }
        
        // now, the fitnesses in idx must form a pareto front
        std::vector<bool> nonDom;
        int n;
        n = getNonDominated(fitness,idx,nonDom);

        dmatrix pfront(n,fitness.columns());
        for (i=0,n=0;i<nonDom.size();++i) {
          if (nonDom[i]) {
            pfront.getRow(n++).fill(fitness.getRow(i));
          }
        }
        pfront.swap(fitness);

        if (pfront.rows() > fitness.rows()) {
          std::cout << "  "
                    << pfront.rows()-fitness.rows() << " elements were removed"
                    << std::endl;
        }

        // write evaluations out in a CSV format (comma separated value)
        std::string outFile = paretoFile + ".csv";
        std::ofstream out(outFile.c_str());
        if (out) {
          // write a small header to know what is this all about
          out << "NR \tNRR \tPA \tOA \tRINF \tT \tRINT" << std::endl;

          int x,y;
          for (y=0;y<fitness.rows();++y) {
            out << fitness.at(y,0);
            for (x=1;x<fitness.columns();++x) {
              out << " \t" << fitness.at(y,x);
            }
            out << std::endl;
          }
          out.close();
          std::cout << "Output written in " << outFile << std::endl;
        }
      } else {
        std::cerr << "File '" << paretoFile << "' could not be opened."
                  << std::endl;
      }
      return;
    }

    //
    // Normal PESA-based evaluation mechanism
    //

    for (i=0,it=parSet.begin();it!=parSet.end();++it,++i) {
      std::cout << "-------------------------------------------------------\n";
      std::cout << " PARAMETER SET " << i << std::endl;
      std::cout << "-------------------------------------------------------\n";
      std::cout << std::endl;

      // set current parameters
      ibse.setParameters(*it);
      
      // evaluate the whole thing
      dmatrix front;
      std::vector<functor::parameters*> phenotypes;
      
      // try to find the pareto.log or equivalent
      std::ifstream inLog((*it).logFilename.c_str());
      
      if ((inLog) && (!force)) {
        inLog.close();
        if (resume) {
          std::cout << "Reassuming..." << std::endl;
          if (!ibse.resume(front,phenotypes)) {
            cout << "Error: " << ibse.getStatusString() << std::endl;
            ibse.trash(phenotypes);
            continue;
          };
        } else {
          std::cout << "Seems to be already computed. If not,please " 
                    << "remove the file:\n";
          std::cout << "  " << (*it).logFilename << std::endl;
          continue;
        }
      } else {
        // using log
        if (!ibse.apply(front,phenotypes)) {
          cout << "Error: " << ibse.getStatusString() << std::endl;
          ibse.trash(phenotypes);
          continue;
        };
      }

      // get the bounding box for the detected fitness space
      dmatrix bbox;
      ibse.getAnalyzedBox(bbox);

      // get all evaluated points.
      dmatrix dead;
      ibse.getDominatedIndividuals(dead);
    
      // draw the front
      image canvas(320,320,lti::Black);

      plotIndividuals(bbox,dead,canvas,rgbPixel(127,127,127),'+');
      plotIndividuals(bbox,front,canvas,lti::White,'o',true);
    
      if (showViewer) {
        view->show(canvas);
      }

      // save the image
      std::string file = (*it).logFilename;
      file += ".png";
      saver.save(file,canvas);

      // save a log file with the whole data and the bounding box at the end.
      file = (*it).logFilename;
      file += ".pf";
      std::ofstream out(file.c_str());
      lispStreamHandler lsh(out);
      
      for (int j=0;j<front.rows();++j) {
        lsh.writeBegin();
        front.getRow(j).write(lsh);
        phenotypes[j]->write(lsh);
        lsh.writeEnd();
        lsh.writeEOL();
      }

      lti::write(lsh,"BoundingBox",bbox);

      out << std::endl;
      out.close();

      // remove all allocated objects
      ibse.trash(phenotypes);
    }

    if (showViewer) {     
      getchar();
      view->hide();
      delete view;
    }

  }

};

int main(int argc,char *argv[]) {
  try {
    lti::tester testObj;
    testObj(argc,argv);
    std::cout << "Test at the end!" << std::endl;
  }
  catch (lti::exception& exp) {
    std::cout << "An LTI::EXCEPTION was thrown: ";
    std::cout << exp.what() << std::endl;
  }
  catch (std::exception& exp) {
    std::cout << "std::exception was thrown: ";
    std::cout << exp.what() << std::endl;
  }
  catch (...) {
    std::cout << "Unknown exception thrown!" << std::endl;
  }
  return 0;

} // main
