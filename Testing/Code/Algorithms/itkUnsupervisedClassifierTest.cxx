/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkUnsupervisedClassifierTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// Insight classes
#include "itkImage.h"
#include "itkVector.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_math.h"
#include "itkImageRegionIterator.h"
#include "itkKmeansUnsupervisedClassifier.h"

//File name definitions 
#define   IMGWIDTH            16
#define   IMGHEIGHT           1
#define   NFRAMES             1
#define   NUMBANDS            2
#define   NDIMENSION          3


#define   CDBKWIDTH           4
#define   CDBKHEIGHT          1
#define   NFRAMES             1
#define   NCODEWORDS          CDBKWIDTH * CDBKHEIGHT * NFRAMES
#define   NUMBANDS            2
#define   NDIMENSION          3
#define   STARTFRAME          0
#define   NUM_BYTES_PER_PIXEL 1
#define   ONEBAND             1


int main()
{

  //------------------------------------------------------
  //Create a simple test vector with 16 entries and 2 bands
  //------------------------------------------------------
  typedef itk::Image<itk::Vector<double,NUMBANDS>,NDIMENSION> VecImageType; 

  VecImageType::Pointer vecImage = VecImageType::New();

  VecImageType::SizeType vecImgSize = {{ IMGWIDTH , IMGHEIGHT, NFRAMES }};

  VecImageType::IndexType index = VecImageType::IndexType::ZeroIndex;
  VecImageType::RegionType region;

  region.SetSize( vecImgSize );
  region.SetIndex( index );

  vecImage->SetLargestPossibleRegion( region );
  vecImage->SetBufferedRegion( region );
  vecImage->Allocate();

  // setup the iterators
  typedef VecImageType::PixelType VecPixelType;

  enum { VecImageDimension = VecImageType::ImageDimension };
  typedef
    itk::ImageRegionIterator< VecImageType > VecIterator;

  VecIterator outIt( vecImage, vecImage->GetBufferedRegion() );

  //--------------------------------------------------------------------------
  //Manually create and store each vector
  //--------------------------------------------------------------------------

  //Vector no. 1
  VecPixelType vec;
  vec = 21,9; outIt.Set( vec ); ++outIt;
  //Vector no. 2
  vec = 10,20; outIt.Set( vec ); ++outIt;
  //Vector no. 3
  vec = 8,21; outIt.Set( vec ); ++outIt;
  //Vector no. 4
  vec = 10,23; outIt.Set( vec ); ++outIt;
  //Vector no. 5
  vec = 12,21; outIt.Set( vec ); ++outIt;
  //Vector no. 6
  vec = 11,12; outIt.Set( vec ); ++outIt;
  //Vector no. 7
  vec = 15,22; outIt.Set( vec ); ++outIt;
  //Vector no. 8
  vec = 9,10; outIt.Set( vec ); ++outIt;
  //Vector no. 9
  vec = 19,10; outIt.Set( vec ); ++outIt;
  //Vector no. 10
  vec = 19,10; outIt.Set( vec ); ++outIt;
  //Vector no. 11
  vec = 21,21; outIt.Set( vec ); ++outIt;
  //Vector no. 12
  vec = 11,20; outIt.Set( vec ); ++outIt;
  //Vector no. 13
  vec = 8,18; outIt.Set( vec ); ++outIt;
  //Vector no. 14
  vec = 18,10; outIt.Set( vec ); ++outIt;
  //Vector no. 15
  vec = 22,10; outIt.Set( vec ); ++outIt;
  //Vector no. 16
  vec = 24,23; outIt.Set( vec ); ++outIt;

  //---------------------------------------------------------------
  //Input the codebook
  //---------------------------------------------------------------
  //------------------------------------------------------------------
  //Read the codebook into an vnl_matrix
  //------------------------------------------------------------------

  vnl_matrix<double> inCDBK(NCODEWORDS, NUMBANDS);
  //There are 4 entries to the code book
  int r,c;
  r=0; c=0; inCDBK.put(r,c,10);
  r=0; c=1; inCDBK.put(r,c,10);
  r=1; c=0; inCDBK.put(r,c,10);
  r=1; c=1; inCDBK.put(r,c,20);
  r=2; c=0; inCDBK.put(r,c,20);
  r=2; c=1; inCDBK.put(r,c,10);
  r=3; c=0; inCDBK.put(r,c,20);
  r=3; c=1; inCDBK.put(r,c,20);

  //----------------------------------------------------------------------
  // Test code for the Unsupervised classifier algorithm
  //----------------------------------------------------------------------

  //---------------------------------------------------------------------
  // Multiband data is now available in the right format
  //---------------------------------------------------------------------
  // Begin the application of the Kmeans algorithm
  //---------------------------------------------------------------------
  //Set the Kmeans classifier
  //---------------------------------------------------------------------
  typedef itk::KmeansUnsupervisedClassifier<VecImageType,VecImageType> 
    KmeansUnsupervisedClassifierType;

  KmeansUnsupervisedClassifierType::Pointer 
    applyKmeansClusterer = KmeansUnsupervisedClassifierType::New();


  //----------------------------------------------------------------------
  //Set the parameters of the clusterer
  //----------------------------------------------------------------------
  applyKmeansClusterer->SetInputImage(vecImage);
  applyKmeansClusterer->SetNumberOfClasses(NCODEWORDS);
  applyKmeansClusterer->SetThreshold(0.01);
  applyKmeansClusterer->Cluster();
  applyKmeansClusterer->Print(std::cout);
  //applyKMeansClusterer->PrintKmeansAlgorithmResults();

  //Validation with no codebook/initial Kmeans estimate
  vnl_matrix<double> kmeansResult = applyKmeansClusterer->GetKmeansResults();
  vnl_matrix<double> errorMat = (kmeansResult - inCDBK);
  double error =0;
  double meanCDBKvalue = 0;

  for(int r = 0; r < NCODEWORDS; r++)
    for(int c = 0; c < NUMBANDS; c++)
    {
      error += vnl_math_abs((kmeansResult[r][c]-inCDBK[r][c])/inCDBK[r][c]);
      meanCDBKvalue += inCDBK[r][c];
    }

  error /= NCODEWORDS*NUMBANDS;
  meanCDBKvalue /= NCODEWORDS*NUMBANDS; 

  if( error < 0.1 * meanCDBKvalue)
    std::cout << "Kmeans algorithm passed (without initial input)"<<std::endl;
  else
    std::cout << "Kmeans algorithm failed (without initial input)"<<std::endl;

  applyKmeansClusterer->SetCodebook(inCDBK);
  applyKmeansClusterer->Cluster();
  applyKmeansClusterer->Print(std::cout);
  //applyKMeansClusterer->PrintKmeansAlgorithmResults();

  //Validation with initial Kmeans estimate provided as input by the user
  kmeansResult = applyKmeansClusterer->GetKmeansResults();
  errorMat = (kmeansResult - inCDBK);
  error =0;
  meanCDBKvalue = 0;

  for(int r = 0; r < NCODEWORDS; r++)
    for(int c = 0; c < NUMBANDS; c++)
    {
      error += vnl_math_abs((kmeansResult[r][c]-inCDBK[r][c])/inCDBK[r][c]);
      meanCDBKvalue += inCDBK[r][c];
    }

  error /= NCODEWORDS*NUMBANDS;
  meanCDBKvalue /= NCODEWORDS*NUMBANDS; 

  if( error < 0.1 * meanCDBKvalue)
    std::cout << "Kmeans algorithm passed (with initial input)"<<std::endl;
  else
    std::cout << "Kmeans algorithm failed (with initial input)"<<std::endl;

  return 0;


}
