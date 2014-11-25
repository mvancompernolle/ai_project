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

/*
 * Usage:
 * shapeRecognition [-h] [-e] [-t] [-s] [filename]
 *   -h        show help
 *   -e        extract descriptors
 *   -t        train classifiers
 *   -s        statistics with images in test directory
 *   filename  try to recognize all shapes in the given image
 */

#include <fstream>
#include <cmath>
#include <string>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <limits>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#include "ltiRGBPixel.h"
#include "ltiViewer.h"
#include "ltiGtkServer.h"
#include "ltiFastViewer.h"
#include "ltiGeometricFeatures.h"
#include "ltiFastRelabeling.h"
#include "ltiLispStreamHandler.h"
#include "ltiLncFeatureFile.h"
#include "ltiALLFunctor.h"
#include "ltiDraw.h"
#include "ltiRbf.h"
#include "ltiKMColorQuantization.h"
#include "ltiUsePalette.h"
#include "ltiClassificationStatistics.h"
#include "ltiTimer.h"
#include "ltiSTLIoInterface.h"

/**
 * Recognition
 *
 * This class encapsulates all steps required for recognition:
 * - Descriptor extraction
 * - Classifier training
 * - Recognition/Test
 */
class recognition {
public:
  /**
   * Constructor
   */
  recognition();

  /**
   * Destructor
   */
  ~recognition();

  /**
   * Read the parameters from the given file
   */
  bool load(const std::string& str);

  /**
   * Write the parameters in the given file
   */
  bool save(const std::string& str) const;

  /**
   * Extract descriptors
   */
  bool extract(const lti::areaPoints& mask,
               lti::dvector& vct) const;

  /**
   * Extract descriptors
   */
  bool extract(const std::string& subdir,
               lti::dmatrix& vcts,
               lti::ivector& ids);

  /**
   * Save descriptors
   */
  bool saveDescriptors(const lti::dmatrix& vcts,
                       const lti::ivector& ids) const;

  /**
   * Load descriptors
   */
  bool loadDescriptors(lti::dmatrix& vcts,
                       lti::ivector& ids);

  /**
   * Train
   */
  bool train(lti::dmatrix& vcts,
             lti::ivector& ids);

  /**
   * Test
   *
   * Try to detect regions in the given image and for each region try to
   * recognize what for an object it is.
   */
  bool test(const lti::image& input,
            lti::image& output,
            lti::image& labels);

  /**
   * Load the network and if found, test if the given vectors can be correctly
   * recognized as the objects given by the corresponding ids.
   *
   * Leave the result in the "data" subdirectory in a file called "rates.txt"
   */
  bool stats(const lti::dmatrix& vcts,
             const lti::ivector& ids);

  /**
   * Set Status String
   */
  void setStatusString(const std::string& str) const;
  
  /**
   * Append Status String
   */
  void appendStatusString(const std::string& str) const;
  
  /**
   * Get Status String
   */
  const char* getStatusString() const;
  
protected:
  /**
   * Absolute path
   */
  std::string path;
  
  /**
   * Train Directory Path
   *
   * Default value: "train/"
   */
  std::string trainPath;

  /**
   * Test Directory Path
   *
   * Default value: "test/"
   */
  std::string testPath;

  /**
   * Data Directory Path
   *
   * Default value: "data/"   
   */
  std::string dataPath;

  /**
   * Class-Id File
   *
   * This is the name of a file with the names and ids of the classes to
   * be identified.  The class name must be the same than the corresponding
   * directory in the train directory.
   * 
   * The format of the file: one class per line: first the class name and then
   * the id.
   *
   * Default value: "classId.txt"
   */
  std::string classIdFile;

  /**
   * Parameters for geometric features
   */
  lti::geometricFeatures::parameters geoParam;

  /**
   * Vector containing the indices of the entries that have to be considered
   * in the final feature vector
   * 
   * Default: [lti::geometricFeatures::compactness, 
   *           lti::geometricFeatures::eccentricity]
   */
  lti::ivector indices;

  /**
   * Parameters for the RBF networks
   */
  lti::rbf::parameters rbfParam;
  
  /**
   * Palette used to represent the results
   *
   * The first entry is always the background
   */
  lti::palette palette;
  

  /**
   * @name Create and Manipulate Directories
   */
  //@{
  /**
   * Read the whole training image directory structure.  It also
   * reads and updates the class ids from/to the corresponding file.
   *
   * @param dirName name of the directory to be read
   * @param files lists of files grouped by the corresponding object class 
   *              ID.  Each element in the list will contain the complete 
   *              file name and the id assigned to the image.
   *        
   * @param totalFiles total number of files in the tree.
   */
  bool readDirectoryTree(const std::string& dirName,
                         std::map<int,
                         std::list<std::pair<std::string,int> > >& files,
                         int& totalFiles);
  
  /**
   * Ensure that the given sub-directory exists
   */
  bool ensureDir(const std::string& theDir) const;
  
  /**
   * Create given directory "as-is", i.e. no path is prepended.
   */
  bool createDir(const std::string& path) const;
  //@}
  
  /**
   * Read the lists of image files sorted per object names.
   *
   * This reflects the same file structure.
   */
  bool getFileList(const std::string& dirName,
                   std::map< std::string,
                   std::list<std::string> >& files);
  
  /**
   * Read the class-name to class-id table.
   */
  bool readClassIdFile(std::map<std::string,int>& nameToIds) const;
  
  /**
   * Write the class-name to class-id table
   */
  bool writeClassIdFile(const std::map<std::string,int>& nameToIds) const;
  
  /**
   * Actualize id-name maps
   */
  bool actualizeIdNamesMaps();

  /**
   * Name to ids
   *
   * Maps the name of a class (e.g. "circle") to an integer id.
   */
  std::map<std::string,int> nameToId;
  
  /**
   * Id to name
   *
   * Maps the class id to the class name.
   */
  std::map<int,std::string> idToName;

private:
  /**
   * The status string
   */
  mutable std::string statusString;

  /**
   * Geometric feature extractor
   */
  lti::geometricFeatures featExt;

};

recognition::recognition() {
  path      = "";
  trainPath = "train/";
  testPath  = "test/";
  dataPath  = "data/";
  classIdFile = "classId.txt";

  geoParam.calcFeatureGroup1 = false;
  geoParam.calcFeatureGroup2 = true;
  geoParam.calcFeatureGroup3 = false;
  geoParam.calcFeatureGroup4 = false;

  indices.resize(2,0,false,false);
  indices.at(0) = lti::geometricFeatures::compactness;
  indices.at(1) = lti::geometricFeatures::eccentricity;

  rbfParam.nbHiddenNeurons = 3;
  rbfParam.nbPresentations2 = 50;

  palette.resize(8,lti::Black);
  palette.at(1) = lti::rgbPixel(192,192,192);
  palette.at(2) = lti::Red;
  palette.at(3) = lti::Green;
  palette.at(4) = lti::Blue;
  palette.at(5) = lti::Cyan;
  palette.at(6) = lti::Yellow;
  palette.at(7) = lti::Magenta;
}

recognition::~recognition() {
}

/*
 * Set Status String
 */
void recognition::setStatusString(const std::string& str) const {
  statusString = str;
}

/*
 * Append Status String
 */
void recognition::appendStatusString(const std::string& str) const {
  statusString += str;
}

/*
 * Get Status String
 */
const char* recognition::getStatusString() const {
  return statusString.c_str();
}

// ---------------------------------------------------------------------------
// Parameters
// ---------------------------------------------------------------------------

bool recognition::load(const std::string& str) {
  lti::lispStreamHandler lsh;
  std::ifstream in(str.c_str());

  if (in) {
    lsh.use(in);
    
    bool b = true;
    
    b = b && lti::read(lsh,"path",path);
    
    if (!path.empty()) {
      if (path[path.length()-1] != '/') {
        path += '/';
      }
    }
    
    b = b && lti::read(lsh,"trainPath",trainPath);
    b = b && lti::read(lsh,"testPath",testPath);
    b = b && lti::read(lsh,"dataPath",dataPath);
    b = b && lti::read(lsh,"classIdFile",classIdFile);
    b = b && lti::read(lsh,"geoParam",geoParam);
    b = b && lti::read(lsh,"indices",indices);
    b = b && lti::read(lsh,"rbfParam",rbfParam);
    b = b && lti::read(lsh,"palette",palette);
    
    // initialize feature extractor
    featExt.setParameters(geoParam);
    
    return b;
  } 

  return false;
}

bool recognition::save(const std::string& str) const {
  lti::lispStreamHandler lsh;
  std::ofstream out(str.c_str());
  lsh.use(out);

  lti::write(lsh,"path",path);
  lti::write(lsh,"trainPath",trainPath);
  lti::write(lsh,"testPath",testPath);
  lti::write(lsh,"dataPath",dataPath);
  lti::write(lsh,"classIdFile",classIdFile);

  out << std::endl;
  lsh.writeComment("Geometric Descriptor");

  lti::write(lsh,"geoParam",geoParam);

  lti::write(lsh,"indices",indices);

  out << std::endl;
  lsh.writeComment("RBF Network");

  lti::write(lsh,"rbfParam",rbfParam);
  
  out << std::endl;
  lsh.writeComment("Result Palette");
  lti::write(lsh,"palette",palette);

  out << std::endl;
  out.close();

  return true;
}

// ---------------------------------------------------------------------------
// Directory administration
// ---------------------------------------------------------------------------

/*
 * Read directory tree
 */
bool 
recognition::readDirectoryTree(const std::string& str,
                               std::map<int,
                               std::list<std::pair<std::string,int> > >& tree,
                               int& totalFiles) {

  bool nameIdMapChanged = false;
  tree.clear();
  totalFiles = 0;
  
  // read the maps between ids and names
  if (!actualizeIdNamesMaps()) {
    setStatusString(std::string("Inconsistencies in ") + classIdFile + ":\n" +
                    getStatusString());
    return false;
  }
  
  int imgId = 1;
  int lastId = nameToId.size()+1;
  int j;
  
  // read the object names and their corresponding file lists
  std::map< std::string,std::list<std::string> > files;
  
  if (!getFileList(str,files)) {
    setStatusString(std::string("Error reading ") + str + ": " +
                    getStatusString());
    return false;
  }
  
  // convert the objects to their ids and assign the files to them
  std::map< std::string,std::list<std::string> >::const_iterator it;
  std::map< std::string,int >::iterator fit;
  std::list<std::string>::const_iterator cit;
  
  for (it=files.begin();it!=files.end();++it) {
    fit = nameToId.find((*it).first);
    // Name already used?
    if (fit == nameToId.end()) {
      // no, we can insert it
      nameToId[(*it).first]=lastId;
      idToName[lastId]=(*it).first;
      lastId++;
      nameIdMapChanged = true;
      fit = nameToId.find((*it).first);
    }
    
    // transform the filenames list into one containing also the image id.
    std::list< std::pair<std::string,int> >& lst = tree[(*fit).second];
    j=0;
    for (cit=(*it).second.begin();cit!=(*it).second.end();++cit) {
      lst.push_back(std::make_pair(*cit,imgId++)); 
      j++;
    }
    totalFiles += (*it).second.size();
  }
  
  files.clear(); // clean up a few thousands of files
  
  if (nameIdMapChanged) {
    writeClassIdFile(nameToId);
  }
  
  return true;
}

bool recognition::ensureDir(const std::string& theDir) const {
    DIR *dir;

    
    std::string path(theDir);
    if (path[path.length()-1] != '/') {
      path += '/';
    }

    // now try to open the subdirectory
    dir = opendir(path.c_str());
    if (dir == NULL) {
      // directory do not exit.  Create it.
      if (!createDir(path.c_str())) {
        setStatusString("Could not create directory " + path);
        return false;
      }
    }
    closedir(dir);
    
    return true;
  }

bool recognition::createDir(const std::string& path) const {
  return (mkdir(path.c_str(),0770) == 0) ;
}


/*
 * get file list of the images for the given directory
 */
bool recognition::getFileList(const std::string& dirName,
                              std::map< std::string,
                                        std::list<std::string> >& files) {
    
  DIR *dir,*sdir;
  dirent* entry;
  std::string ndir,file;
  
  typedef std::map< std::string,std::list<std::string> > ftree_type;
  std::pair<ftree_type::iterator,bool> resIns;
  
  // check if the path directory exists
  dir = opendir(dirName.c_str());
  if (dir==NULL) {
    setStatusString("Could not open directory " + dirName);
    return false;
  }
  
  files.clear();
  
  // while still objects in directory
  while ((entry=readdir(dir)) != 0) {
    
    file = entry->d_name;
    
    if ((file != ".") && (file != "..")) {
      
      ndir = dirName + file + "/";
      
      // get the image names
      sdir = opendir(ndir.c_str());
      if (sdir == NULL) {
        continue;
      }
      
      // create a list for the given object.
      resIns = files.insert(std::make_pair(file,std::list<std::string>()));
      
      if (!resIns.second) {
        // object existed already!
        // this shouldn't happen but it did! 
        // so let's ignore it
        continue;
      }
      
      while ((entry=readdir(sdir)) != 0) {
        file = entry->d_name;
        if ((file != ".") && 
            (file != "..") && 
            (file.find(".png") != std::string::npos)) {
          file = ndir + file;
          (*(resIns.first)).second.push_back(file);
        }
      }
      closedir(sdir);        
    }
  }
  
  closedir(dir);
  
  // The map is automatically sorted by the object name. 
  // Sort now each object list
  ftree_type::iterator it;
  for (it=files.begin();it!=files.end();++it) {
    (*it).second.sort();
  }
  
  return true;
}

bool recognition::readClassIdFile(std::map<std::string,int>& nameToIds) const {
    
  std::string file = path + classIdFile;  
  std::ifstream in(file.c_str());

  nameToIds.clear();
  
  if (in) {
    lti::lispStreamHandler lsh(in);
    std::string name;
    int id;
    bool ok = true;
    int size = 0;
    do {
      ok = lsh.read(name);
      ok = ok && lsh.readDataSeparator();
      ok = ok && lsh.read(id);
      if (ok) {
        nameToIds.insert(std::make_pair(name,id));
        ++size;
      }
      
    } while(ok);
    
    in.close();
    
    return (size > 0);
  }
  
  setStatusString(std::string("Class-Id file ") + path + classIdFile +
                  " could not be opened.");
  
  return false;
  
}

bool recognition::writeClassIdFile(
                            const std::map<std::string,int>& nameToIds) const {

  const std::string file = path + classIdFile;
  
  std::ofstream out(file.c_str());
  
  if (out) {
    lti::lispStreamHandler lsh(out);
    std::map<std::string,int>::const_iterator it;
    for (it=nameToIds.begin();it!=nameToIds.end();++it) {
      lsh.write((*it).first);
      lsh.writeDataSeparator();
      lsh.write((*it).second);
      out << std::endl;
    }
    return true;
  }
  
  setStatusString(std::string("Class-Id file ") + classIdFile +
                  " could not be written.");
  
  return false;
}

bool recognition::actualizeIdNamesMaps() {
  nameToId.clear();
  idToName.clear();
  
  bool ok = true;
  
  if (readClassIdFile(nameToId)) {
    idToName.clear();
    std::map<std::string,int>::const_iterator it;
    std::pair<std::map<int,std::string>::iterator,bool> insRes;
    for (it=nameToId.begin();it!=nameToId.end();++it) {
      insRes=idToName.insert(std::make_pair((*it).second,(*it).first));
      if (!insRes.second) {
        if (ok) {
          setStatusString("Duplicated labels for objects ");
          ok = false;
        }
        appendStatusString((*it).first + " ");
      }
    }
  }
  
  return ok;
}

// ---------------------------------------------------------------------------
// The real stuff
// ---------------------------------------------------------------------------

bool recognition::extract(const lti::areaPoints& aPts,
                          lti::dvector& vct) const {
  int i;

  if (!aPts.empty()) {

    lti::dvector tmp;

    // extract some geometric information
    featExt.apply(aPts,tmp);

    if (vct.size() != indices.size()) {
      vct.resize(indices.size());
    }
    
    // use only the desired "features"
    for (i=0;i<indices.size();++i) {
      vct.at(i)=tmp.at(indices.at(i));
    }

    return true;
  }

  return false;
}

bool recognition::extract(const std::string& subdir,
                          lti::dmatrix& vcts,
                          lti::ivector& ids) {
  
  lti::loadImage loader;
  lti::usePalette paletter;

  lti::palette bw(2,lti::Black);
  bw.at(1)=lti::White;
  lti::image img;
  lti::channel8 mask;
  lti::dvector vct;
  std::vector<lti::areaPoints> aPts;
  lti::imatrix imask;
  lti::ivector regsizes;

  lti::fastRelabeling relabeler;

  lti::fastRelabeling::parameters frPar;
  frPar.assumeLabeledMask = false;
  frPar.sortSize = true;
  relabeler.setParameters(frPar);

  std::map<int,std::list<std::pair<std::string,int> > > files;
  std::map<int,std::list<std::pair<std::string,int> > >::const_iterator it;
  std::list<std::pair<std::string,int> >::const_iterator fit;
  int totalFiles;
  int i,oid;
  i=0;

  lti::fastViewer view;

  // for all images in the train directory
  if (readDirectoryTree(path + subdir,files,totalFiles)) {
    vcts.resize(totalFiles,indices.size(),0.0,false,true);
    ids.resize(vcts.rows(),0);
    
    for (it=files.begin();it!=files.end();++it) {
      oid = (*it).first;
      const std::list<std::pair<std::string,int> >& theList = (*it).second;
      for (fit=theList.begin();fit!=theList.end();++fit) {
        if (loader.load((*fit).first,img)) {
          std::cout << i+1 << "/" << totalFiles << " File: " 
                    << (*fit).first << std::endl;
          view.show(img);

          ids.at(i)=oid;
          // it is assumed that the train images contain only two colors: 
          // black and white
          // Get then a binary mask:
          paletter.apply(img,bw,mask);

          // get the second object of the list (the first is always background)
          relabeler.apply(mask,imask,regsizes,aPts);
          
          extract(aPts[1],vcts.getRow(i));
          
          ++i;
        } else {
          setStatusString(loader.getStatusString());
        }
      }
    }
  } else {
    setStatusString("Could not read directory "+subdir);
    return false;
  }

  return true;
}

bool recognition::saveDescriptors(const lti::dmatrix& vcts,
                                  const lti::ivector& ids) const {
  ensureDir(path + "data");
  lti::saveLnc::parameters lncPar;
  lncPar.filename = path + "data/descriptors.lnc";
  lncPar.useCnc = true;
  lncPar.cncFilename = path + classIdFile;
  lncPar.idFirst = false;

  lti::saveLnc lnc;
  lnc.setParameters(lncPar);
  lti::lncFeatureFile::patternId id;
  std::map<int,std::string>::const_iterator it;

  int i;
  for (i=0;i<vcts.rows();++i) {
    id.objectId=ids.at(i);
    
    if ((it=idToName.find(id.objectId)) != idToName.end()) {
      id.name = (*it).second;
    } else {
      id.name = "unknown";
    }

    lnc.apply(vcts.getRow(i),id);
  }

  return true;
}

bool recognition::loadDescriptors(lti::dmatrix& vcts,
                                  lti::ivector& ids) {
  ensureDir(path + "data");
  lti::loadLnc::parameters lncPar;
  lncPar.filename = path + "data/descriptors.lnc";
  lncPar.useCnc = true;
  lncPar.cncFilename = path + classIdFile;
  lncPar.idFirst = false;

  lti::loadLnc lnc;
  lnc.setParameters(lncPar);

  nameToId.clear();
  idToName.clear();
  
  // read all features
  if (lnc.load(lncPar.filename,vcts,ids,idToName,true)) {
  
    // update the name->ids map
    std::map<int,std::string>::const_iterator it;
    for (it=idToName.begin();it!=idToName.end();++it) {
      nameToId[(*it).second]=(*it).first;
    }

    return true;
  }

  setStatusString("Could ot read "+lncPar.filename);

  return false;
}

bool recognition::train(lti::dmatrix& vcts,
                        lti::ivector& ids) {

  // the neural network (this could also be a SVM, MLP or KNN)
  lti::rbf ann(rbfParam);

  // we want to see something while training
  lti::streamProgressInfo spg;
  ann.setProgressObject(spg);

  // try to train
  bool rc = true;
  rc = ann.train(vcts,ids);


  // train successful, save the results:
  std::string str = path + "data/data.rbf";
  std::ofstream out(str.c_str());
  
  lti::lispStreamHandler lsh(out);
  
  ann.write(lsh);
  
  out << std::endl;
  out.close();
  
  if (rc) {
    return true;
  } else {

    setStatusString("Problems training or saving neural network");
    appendStatusString(ann.getStatusString());
    return false;
  }
}

bool recognition::stats(const lti::dmatrix& vcts,
                        const lti::ivector& ids) {

  // try to read the neural network
  std::string str = path + "data/data.rbf";
  lti::rbf ann(rbfParam);
  actualizeIdNamesMaps();

  // initialize the statistics functor
  lti::classificationStatistics::parameters csPar;
  csPar.useNames = true;
  csPar.namesFile = path + classIdFile;
  csPar.namesInFileFirst = true;
  csPar.suppressZeros = true;
  csPar.writeConfusionMatrix = true;
  csPar.path = path;
  csPar.confusionMatrix = "data/rates.txt";
  csPar.useLogFile = false;  
  lti::classificationStatistics classStats(csPar);
  lti::rbf::outputVector result;

  std::ifstream in(str.c_str());
  if (in) {
    lti::lispStreamHandler lsh(in);
    // read the neural network
    if (ann.read(lsh)) {
      // and if it is all right
      int i;
      // test all the patterns given.
      for (i=0;i<vcts.rows();++i) {
        std::cout << i+1 << "/" << vcts.rows() << std::endl;
        ann.classify(vcts.getRow(i),result);
        classStats.consider(result,ids.at(i));
      }

      classStats.flush();
      return true;
    }
  }

  setStatusString("Problems reading data.rbf file");
  return false;  
}

bool recognition::test(const lti::image& input,
                       lti::image& output,
                       lti::image& labels) {
  
  // try to read the neural network
  std::string str = path + "data/data.rbf";
  lti::rbf ann(rbfParam);
  actualizeIdNamesMaps();
  std::string::size_type maxNameLength = 0;

  // we need a map from the object id to the index in the color palette
  std::map<int,int> idToColorIdx,idCounter;
  std::map<int,std::string>::const_iterator cit;
  int idx=1;
  for (cit=idToName.begin();cit!=idToName.end();++cit) {
    idToColorIdx[(*cit).first]=idx;
    maxNameLength=lti::max(maxNameLength,(*cit).second.size());
    idx = 1+(idx%palette.lastIdx());
  }

  // the classification output
  lti::rbf::outputVector result;

  std::ifstream in(str.c_str());
  if (in) {
    lti::lispStreamHandler lsh(in);
    // read the neural network
    if (ann.read(lsh)) {
      // and if it is all right
      lti::channel8 chnl;
      lti::palette pal;
      lti::imatrix imask;
      lti::ivector numPixels;
      lti::dvector descriptor;
      std::vector<lti::areaPoints> aPts;
      lti::draw<lti::rgbPixel> painter;
      unsigned int i;

      // the result "canvas"
      output.resize(input.size(),palette.at(0));
      painter.use(output);
      
      // detect homogeneous regions in the image
      lti::kMColorQuantization km;
      km.apply(input,chnl,pal);
      
      // now detect the regions in the image
      lti::fastRelabeling::parameters frPar;
      frPar.assumeLabeledMask = true;
      frPar.sortSize = true;

      lti::fastRelabeling relabeler(frPar);
      relabeler.apply(chnl,imask,numPixels,aPts);

      // for all objects with a color different than the background (in the
      // palette)
      for (i=0;i<aPts.size();++i) {
        if (input.at(aPts[i].front()) == palette.at(0)) {
          continue;
        }

        extract(aPts[i],descriptor);
        ann.classify(descriptor,result);
        idx=result.maxId();

        // paint the winner object in the canvas
        painter.setColor(palette.at(idToColorIdx[idx]));
        painter.set(aPts.at(i));

        idCounter[idx]++;
      }

      // now create the labels image and some simple statistics
      labels.resize(8+8*idToColorIdx.size(),8+8*maxNameLength,palette.at(0));
      idx = 1;
      i=0;
      painter.use(labels);
      std::map<int,int>::const_iterator ci2;
      for (cit=idToName.begin(),ci2=idCounter.begin();
           cit!=idToName.end();
           ++cit,++ci2,++i) {
        painter.setColor(palette.at(idx));
        painter.text((*cit).second,lti::ipoint(4,4+i*8));
        idx = 1+(idx%palette.lastIdx());
        std::cout << (*ci2).second << " " << (*cit).second << std::endl;
      }

      return true;
    }
  }

  return false;
}

// ---------------------------------------------------------------------------
// USAGE
// ---------------------------------------------------------------------------

void usage() {

  std::cout 
    << "Usage:\n"
    << "shapeRecognition [-h] [-e] [-t] [-s] [filename]\n"
    << "-h        show help\n"
    << "-e        extract descriptors\n"
    << "-t        train classifiers\n"
    << "-s        statistics with images in test directory\n"
    << "filename  try to recognize all shapes in the given image\n"
    << std::endl;
}

// ---------------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {

  // read the configuration file
  const static std::string config = "config.dat";
  recognition recognizer;
  if (!recognizer.load(config)) {
    // problems with configuration, re-writting it.
    recognizer.save(config);
  }
  
  lti::dmatrix vcts;
  lti::ivector ids;
  int argn = 1;
  std::string str;
  while (argn < argc) {
    str = argv[argn];
    if (str == "-h" || str == "--help") {
      usage();
      return EXIT_SUCCESS;      
    } else if (str == "-e") {
      // extract some descriptors used in the training phase
      if (!recognizer.extract("train/",vcts,ids) ||
          !recognizer.saveDescriptors(vcts,ids)) {
        std::cerr << recognizer.getStatusString() << std::endl;
      }
    } else if (str == "-t") {
      // train the network using the descriptors extracted with '-e'
      if (!recognizer.loadDescriptors(vcts,ids) ||
          !recognizer.train(vcts,ids)) {
        std::cerr << recognizer.getStatusString() << std::endl;
      }
    } else if (str == "-s") {
      // generate some statistics using the network trained with '-t'
      if (!recognizer.extract("test/",vcts,ids) ||
          !recognizer.stats(vcts,ids)) {
        std::cerr << recognizer.getStatusString() << std::endl;
      }
    } else {
      lti::loadImage loader;
      lti::image img;
      lti::image out;
      lti::image labels;

      lti::viewer vorig("Original");
      lti::viewer vresu("Result");
      lti::viewer vlabs("Labels");

      if (loader.load(str,img)) {
        if (recognizer.test(img,out,labels)) {
          vorig.show(img);
          lti::passiveWait(100000);
          vresu.show(out);
          lti::passiveWait(100000);
          vlabs.show(labels);

          getchar();

          vorig.hide();
          vresu.hide();
          vlabs.hide();

          lti::passiveWait(100000);

        }
      }
    }
    argn++;
  }

  lti::gtkServer::shutdown();

  return EXIT_SUCCESS;
}
