/*
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
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

/**
 * \file
 * This is a tool to visualize the classification performance of
 * various classifiers on 2D data.
 * 
 * Usage: classifierViz [-c <classifier>] [-p <paramFile>] 
 *                      [-k <kernelType>] [-s <kernelParam>]
 *                      [-v <vizParam>]
 *                      [-w <width>] [-h <height>]
 *                      [-r <[x1 x2 y1 y2]>] <filename>
 *        classifierViz -d
 *        classifierViz --help
 *
 * --help print help
 *
 * -d
 * writes all possible parameters files with filenames "def<TYPE>.par". All
 * other parameters including the filename are ignored when this option is 
 * given.
 *
 * -c 
 * <classifier> is a lti::supervisedInstanceClassifier that takes sets
 *              of dvectors and integer ids as training data. Currently 
 *              the following classifiers are availabele in the LTI-Lib:
 *   MLP        Multi-Layer Perceptron.
 *   RBF        Radial Basis Function network
 *   SVM        Support Vector Machine
 *   KNN        k-Nearest-Neighbor Classifier
 * The default is RBF.
 *
 * -p
 * <paramFile> is a parameters file of the classifier type given 
 *             by <classifier>.
 *
 * -k
 * <kernelType> if <classifier> is SVM this parameter sets the kernel type. 
 *              It overrides the kernel given in a parameters file. 
 *              The following kernels are available:
 *   Linear     Linear kernel.
 *   Polynomial Polynomial kernel.
 *   Radial     Radial (RBF) kernel.
 *   Sigmoid    Sigmoid kernel.
 * The default is Linear.
 *
 * -s
 * <kernelParam> is a scalar parameter for the SVM kernel. If a Radial kernel
 *               is used it is the variance. For Polynomial its the degree.
 *               This value is interpreted only if the <kernelType> is set 
 *               manually. Defaults are 0.5 and 2 for the above kernels.
 * 
 * -v
 * <vizParam> A parameters file of the lti::classifier2DVisualization functor.
 *
 * -w
 * <width> Width of the image to be used for visualization. Overrides value
 *         in the vizParam file. Default 400.
 *
 * -h
 * <height> Height of the image to be used for visualization. Overrides value
 *         in the vizParam file. Default 400.
 *
 * -r
 * <[x1 y1 x2 y2]> Range the x and y values in the data file can take.
 *                 Overrides value in the vizParam file. Default [-1 -1 1 1].
 *
 * filename An ASCII file that contains a double data matrix with the label 
 *          "inputs" and ids with the label "ids". The example "setgen" creates
 *          such files. They are easily created with 
 *          lti::write(handler,"inputs",data);
 *          lti::write(handler,"ids",ids);
 *          where handler is a file handle, data and ids as expected.
 */

#include <ltiRGBPixel.h>
#include "ltiMLP.h"
#include "ltiSVM.h"
#include "ltiRbf.h"
#include "ltiKNNClassifier.h"
#include "ltiSupervisedInstanceClassifier.h"

#include "ltiKernelFunctor.h"
#include "ltiLinearKernel.h"
#include "ltiPolynomialKernel.h"
#include "ltiRadialKernel.h"
#include "ltiSigmoidKernel.h"

#include "ltiClassifier2DVisualizer.h"
#include "ltiImage.h"
#include "ltiViewer.h"

#include "ltiPoint.h"
#include "ltiGtkServer.h"
#include "ltiTimer.h"
#include "ltiLispStreamHandler.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

enum eClassifierType {
  MLP, RBF, SVM, KNN};

enum eKernelType {
  Linear, Polynomial, Radial, Sigmoid};

static lti::classifier2DVisualizer _my_viz;
static lti::viewer _my_viewer;

void printHelp() {
  std::cerr
 <<"This is a tool to visualize the classification performance of"<<std::endl
 <<"various classifiers on 2D data."<<std::endl
 <<""<<std::endl
 <<"Usage: classifierViz [-c <classifier>] [-p <paramFile>] "<<std::endl
 <<"                     [-k <kernelType>] [-s <kernelParam>]"<<std::endl
 <<"                     [-v <vizParam>]"<<std::endl
 <<"                     [-w <width>] [-h <height>]"<<std::endl
 <<"                     [-r <[x1 x2 y1 y2]>] <filename>"<<std::endl
 <<"       classifierViz -d"<<std::endl
 <<"       classifierViz --help"<<std::endl
 <<std::endl
 <<"--help print help"<<std::endl
 <<std::endl
 <<"-d"<<std::endl
 <<"writes all possible parameters files with filenames \"def<TYPE>.par\". All"<<std::endl
 <<"other parameters including the filename are ignored when this option is "<<std::endl
 <<"given."<<std::endl
 <<std::endl
 <<"-c "<<std::endl
 <<"<classifier> is a lti::supervisedInstanceClassifier that takes sets"<<std::endl
 <<"             of dvectors and integer ids as training data. Currently "<<std::endl
 <<"             the following classifiers are availabele in the LTI-Lib:"<<std::endl
 <<"  MLP        Multi-Layer Perceptron."<<std::endl
 <<"  RBF        Radial Basis Function network"<<std::endl
 <<"  SVM        Support Vector Machine"<<std::endl
 <<"  KNN        k Nearest Neighbor Classifier"<<std::endl
 <<"The default is RBF."<<std::endl
 <<std::endl
 <<"-p"<<std::endl
 <<"<paramFile> is a parameters file of the classifier type given "<<std::endl
 <<"            by <classifier>."<<std::endl
 <<std::endl
 <<"-k"<<std::endl
 <<"<kernelType> if <classifier> is SVM this parameter sets the kernel type. "<<std::endl
 <<"             It overrides the kernel given in a parameters file. "<<std::endl
 <<"             The following kernels are available:"<<std::endl
 <<"  Linear     Linear kernel."<<std::endl
 <<"  Polynomial Polynomial kernel."<<std::endl
 <<"  Radial     Radial (RBF) kernel."<<std::endl
 <<"  Sigmoid    Sigmoid kernel."<<std::endl
 <<"The default is Linear."<<std::endl
 <<std::endl
 <<"-s"<<std::endl
 <<"<kernelParam> is a scalar parameter for the SVM kernel. If a Radial kernel"<<std::endl
 <<"              is used it is the variance. For Polynomial its the degree."<<std::endl
 <<"              This value is interpreted only if the <kernelType> is set "<<std::endl
 <<"              manually. Defaults are 0.5 and 2 for the above kernels."<<std::endl
 <<std::endl
 <<"-v"<<std::endl
 <<"<vizParam> A parameters file of the lti::classifier2DVisualization functor."<<std::endl
 <<std::endl
 <<"-w"<<std::endl
 <<"<width> Width of the image to be used for visualization. Overrides value"<<std::endl
 <<"        in the vizParam file. Default 400."<<std::endl
 <<std::endl
 <<"-h"<<std::endl
 <<"<height> Height of the image to be used for visualization. Overrides value"<<std::endl
 <<"        in the vizParam file. Default 400."<<std::endl
 <<std::endl
 <<"-r"<<std::endl
 <<"<[x1 y1 x2 y2]> Range the x and y values in the data file can take."<<std::endl
 <<"                Overrides value in the vizParam file. Default [-1 -1 1 1]."<<std::endl
 <<std::endl
 <<"filename An ASCII file that contains a double data matrix with the label "<<std::endl
 <<"         inputs and ids with the label ids. The example setgen creates"<<std::endl
 <<"         such files. They are easily created with "<<std::endl
 <<"         lti::write(handler,\"inputs\",data);"<<std::endl
 <<"         lti::write(handler,\"ids\",ids);"<<std::endl
 <<"         where handler is a file handle, data and ids as expected."<<std::endl;
  
}

void dumpParameters() {

  lti::rbf::parameters rp;
  lti::MLP::parameters mp;
  lti::svm::parameters sp;
  lti::kNNClassifier::parameters kp;

  lti::classifier2DVisualizer::parameters vp;

  std::ofstream of;
  lti::lispStreamHandler lsh;

  of.open("defaultRBF.par");
  if (of.good() && of.is_open()) {
    lsh.use(of);
    rp.write(lsh);
  } else {
    std::cerr << "Could not write default parmeters file\n";
  }
  of.close();

  of.open("defaultMLP.par");
  if (of.good() && of.is_open()) {
    lsh.use(of);
    mp.write(lsh);
  } else {
    std::cerr << "Could not write default parmeters file\n";
  }
  of.close();

  of.open("defaultSVM.par");
  if (of.good() && of.is_open()) {
    lsh.use(of);
    sp.write(lsh);
  } else {
    std::cerr << "Could not write default parmeters file\n";
  }
  of.close();

  of.open("defaultKNN.par");
  if (of.good() && of.is_open()) {
    lsh.use(of);
    kp.write(lsh);
  } else {
    std::cerr << "Could not write default parmeters file\n";
  }
  of.close();


  of.open("defaultVisualizer.par");
  if (of.good() && of.is_open()) {
    lsh.use(of);
    vp.write(lsh);
  } else {
    std::cerr << "Could not write default parmeters file\n";
  }
  of.close();

}

bool initClassifier(lti::supervisedInstanceClassifier*& classifier, 
                    const eClassifierType& cType,
                    const std::string& cParam,
                    const eKernelType& kType,
                    const std::string& kScalar) {

  lti::supervisedInstanceClassifier::parameters* param;
  classifier=0;

  switch (cType) {
    case RBF:
      classifier=new lti::rbf();
      param=new lti::rbf::parameters();
      break;
    case SVM:
      classifier=new lti::svm();
      param=new lti::svm::parameters();
      break;
    case MLP:
      classifier=new lti::MLP();
      param=new lti::MLP::parameters();
      break;
    case KNN:
      classifier=new lti::kNNClassifier();
      param=new lti::kNNClassifier::parameters();
      break;
    default:
      std::cerr << "Unknown classifier type\n";
      return false;
  }

  if (classifier==0) {
    std::cerr << "Could not instantiate classifier\n";
    return false;
  }
    
  if (cParam.length() != 0) {
    std::ifstream ifs(cParam.c_str());
    if (ifs.good() && ifs.is_open()) {
      lti::lispStreamHandler lsh;
      lsh.use(ifs);
      param->read(lsh);
      classifier->setParameters(*param);
      ifs.close();
      delete param;
      param=0;
    } else {
      std::string msg = "Could not open classifier parameters file" + cParam;
      std::cerr << msg << std::endl;
    }
  }

  if (cType==SVM) {
    lti::kernelFunctor<double>* kernel=0;
    lti::svm* svm=dynamic_cast<lti::svm*>(classifier);
    switch(kType) {
      case Linear:
        kernel=new lti::linearKernel();
        break;
      case Polynomial: {
        int d=2;
        if (kScalar.length()!=0) {
          d=atoi(kScalar.c_str());
        }
        kernel=new lti::polynomialKernel(d);
        break;
      }
      case Radial: {
        double s=0.5;
        if (kScalar.length()!=0) {
          s=atof(kScalar.c_str());
        }
        kernel=new lti::radialKernel(s);
        break;
      }
      case Sigmoid:
        kernel=new lti::sigmoidKernel();
        break;
      default:
        std::cerr << "Unknown kernel type\n";
        exit(1);
    }
    svm->setKernel(*kernel);
    //classifier=svm;
    delete kernel;
    kernel=0;
  }
  
  return true;

}

bool getData(const std::string& filename,
             lti::dmatrix& data, lti::ivector& ids) {

  std::ifstream ifs(filename.c_str());
  if (ifs.good() && ifs.is_open()) {
    lti::lispStreamHandler lsh;
    lsh.use(ifs);
    if(!lti::read(lsh,"inputs",data)) {
      std::string msg="Error reading data from "+filename;
      std::cerr << msg << std::endl;
      return false;
    }
    if(!lti::read(lsh,"ids",ids)) {
      std::string msg="Error reading ids from "+filename;
      std::cerr << msg << std::endl;
      return false;
    }
    ifs.close();
  } else {
    std::string msg="Error opening file "+filename;
    std::cerr << msg << std::endl;
    return false;
  }

  return true;
}

bool initVisualizerParam(lti::classifier2DVisualizer::parameters& vizParam,
                         const std::string& vParam, 
                         const int& width, const int& height,
                         const lti::dpoint& ll, const lti::dpoint& ur) {

  if (vParam.length()!=0) {
    std::ifstream ifs(vParam.c_str());
    if (ifs.good() && ifs.is_open()) {
      lti::lispStreamHandler lsh;
      lsh.use(ifs);
      if(!vizParam.read(lsh)) {
        std::string msg="Could not read visualizer parameters from file"
          +vParam;
        std::cerr << msg << std::endl;
        return false;
      }
      ifs.close();
    } else {
      std::string msg="Could not open file"+vParam;
      std::cerr << msg << std::endl;
      return false;
    }
  }
  
  vizParam.imgSize=lti::point(width, height);
  vizParam.upperRight=ur;
  vizParam.lowerLeft=ll;
  
  return true;
}

int main(int argc, char* argv[]) {

// parses the command line arguments
  std::string filename="";
  eClassifierType cType=RBF;
  std::string cParam="";
  eKernelType kType=Linear;
  std::string kScalar="";
  std::string vParam="";
  int width=400;
  int height=400;
  lti::dpoint ur(-1,-1);
  lti::dpoint ll(1,1);

  bool ok;

  for (int i=1; i<argc; i++) {
    if (*argv[i] == '-') {
      switch (argv[i][1]) {
        // dump parametes?
        case 'd':
          dumpParameters();
          exit(0);
          break;
        // print help
        case '-':
          if (strcasecmp(&argv[i][2],"help")==0) {
            printHelp();
            exit(0);
          } else {
            std::cerr << "Unknown option "<<argv[i]<<std::endl;
          }
          break;
        // classifier
        case 'c':
          if (i < argc-1) {
            ++i;
            if (strcasecmp(argv[i],"rbf") == 0) {
              cType=RBF;
            } else if (strcasecmp(argv[i],"mlp") == 0) {
              cType=MLP;
            } else if (strcasecmp(argv[i],"svm") == 0) {
              cType=SVM;
            } else if (strcasecmp(argv[i],"knn") == 0) {
              cType=KNN;
            } else {
              std::cerr << "Warning: Classifier " << argv[i] 
                        << " is not supported, using RBF instead\n";
            }
          } else {
            std::cerr << "Missing argument to -c\n";
          }
          break;
        // classifier parameters
        case 'p':
          if (i < argc-1) {
            cParam=argv[++i];
          } else {
            std::cerr << "Missing argument to -p\n";
          }
          break;
        // kernel
        case 'k':
          if (i < argc-1) {
            ++i;
            if (strcasecmp(argv[i],"linear") == 0) {
              kType=Linear;
            } else if (strcasecmp(argv[i],"polynomial") == 0) {
              kType=Polynomial;
            } else if (strcasecmp(argv[i],"radial") == 0) {
              kType=Radial;
            } else if (strcasecmp(argv[i],"sigmoid") == 0) {
              kType=Sigmoid;
            } else {
              std::cerr << "Warning: Kernel " << argv[i] 
                        << " is not supported using Linear\n";
            }
          } else {
            std::cerr << "Missing argument to -k\n";
          }
          break;
        // kernel parameter
        case 's':
          if (i < argc-1) {
            kScalar=argv[++i];
          } else {
            std::cerr << "Missing argument to -s\n";
          }
          break;
        // visualizer parameters
        case 'v':
          if (i < argc-1) {
            vParam=argv[++i];
          } else {
            std::cerr << "Missing argument to -v\n";
          }
          break;
        // image width
        case 'w':
          if (i < argc-1) {
            width=atoi(argv[++i]);
          } else {
            std::cerr << "Missing argument to -w\n";
          }
          break;
        // image height
        case 'h':
          if (i < argc-1) {
            height=atoi(argv[++i]);
          } else {
            std::cerr << "Missing argument to -h\n";
          }
          break;
        // range
        case 'r':
          ok = i < argc-1;
          if (ok) {
            ll.x=atof(&argv[++i][1]);
          }
          ok = ok && i < argc-1;
          if (ok) {
            ll.y=atof(argv[++i]);
          }
          ok = ok && i < argc-1;
          if (ok) {
            ur.x=atof(argv[++i]);
          }
          ok = ok && i < argc-1;
          if (ok) {
            ++i;
            int itmp;
            itmp=strlen(argv[i]);
            char stmp[itmp-1];
            strncpy(stmp,argv[i],itmp-2);
            ur.y=atof(stmp);
          }
          if (!ok) {
            std::cerr << "Missing argument to -r or argument incomplete\n";
          }
          break;
        // unknown option
        default:
          std::cerr << "Warning: Unknown option -" << argv[i][1] << std::endl;
          break;
      }
    } else {
      filename=argv[i];
      if (i<argc-1) {
        std::cerr << "filename must be last argument, ignoring the rest.\n";
        break;
      }
    }
  }
  if (filename.length() == 0) {
    std::cerr << "You need to give a file name.\n";
    printHelp();
    exit(1);
  }

  lti::supervisedInstanceClassifier* classifier=0;

  if (!initClassifier(classifier, cType, cParam, kType, kScalar)) {
    std::cerr << "Could not initialize classifier\n";
    exit(1);
  }

  if (classifier==0) {
    std::cerr << "Classifier not initialized\n";
    exit(1);
  }

  lti::dmatrix data;
  lti::ivector ids;

  if (!getData(filename,data,ids)) {
    std::string msg="Could not read data from file " + filename;
    std::cerr << msg << std::endl;
    exit(1);
  }

  if (!classifier->train(data,ids)) {
    std::cerr << "Could not train classifier on given data\n";
    std::cerr << classifier->getStatusString() << std::endl;
    exit(1);
  }

  lti::classifier2DVisualizer::parameters vizParam;
  if (!initVisualizerParam(vizParam, vParam, width, height, ll, ur)) {
    std::cerr << "Could not initialize visualizer\n";
    exit(1);
  }

  _my_viz.setParameters(vizParam);

  lti::image img(width,height);
  
  if (!_my_viz.apply(*classifier,data,img,ids)) {
    std::cerr << "Error visualizing the classification result\n";
  }

  _my_viewer.show(img);
  _my_viewer.waitButtonPressed();
  _my_viewer.hide();
  lti::passiveWait(10000);

  delete classifier;
  classifier=0;

  lti::gtkServer::shutdown();

}

