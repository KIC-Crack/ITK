/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkRecursiveGaussianImageFiltersTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/




#include <itkImage.h>
#include <itkFirstDerivativeRecursiveGaussianImageFilter.h>
#include <itkSecondDerivativeRecursiveGaussianImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>


int main() 
{

  // Define the dimension of the images
  const unsigned int myDimension = 3;

  // Declare gradient type
  typedef itk::CovariantVector<float, myDimension> myGradientType;

  // Declare the types of the images
  typedef itk::Image<float, myDimension>           myImageType;

  // Declare the type of the index to access images
  typedef itk::Index<myDimension>             myIndexType;

  // Declare the type of the size 
  typedef itk::Size<myDimension>              mySizeType;

  // Declare the type of the Region
  typedef itk::ImageRegion<myDimension>        myRegionType;

  // Create the image
  myImageType::Pointer inputImage  = myImageType::New();

  
  // Define their size, and start index
  mySizeType size;
  size[0] = 100;
  size[1] = 100;
  size[2] = 100;

  myIndexType start;
  start = myIndexType::ZeroIndex;

  myRegionType region;
  region.SetIndex( start );
  region.SetSize( size );

  // Initialize Image A
  inputImage->SetLargestPossibleRegion( region );
  inputImage->SetBufferedRegion( region );
  inputImage->SetRequestedRegion( region );
  inputImage->Allocate();

  // Declare Iterator types apropriated for each image 
  typedef itk::ImageRegionIteratorWithIndex<myImageType>  myIteratorType;


  // Create one iterator for the Input Image A (this is a light object)
  myIteratorType it( inputImage, inputImage->GetRequestedRegion() );

  // Initialize the content of Image A
  std::cout << "Input Image initialization " << std::endl;
  while( !it.IsAtEnd() ) 
  {
    it.Set( 0.0 );
    ++it;
  }

  size[0] = 60;
  size[1] = 60;
  size[2] = 60;

  start[0] = 20;
  start[1] = 20;
  start[2] = 20;

  // Create one iterator for an internal region
  region.SetSize( size );
  region.SetIndex( start );
  myIteratorType itb( inputImage, region );

  // Initialize the content the internal region
  while( !itb.IsAtEnd() ) 
  {
    itb.Set( 100.0 );
    ++itb;
  }




  // Declare the type for the  Smoothing  filter
  typedef itk::RecursiveGaussianImageFilter<
                                      myImageType,
                                      myImageType,
                                      float       >  mySmoothingFilterType;
            

  // Create a  Filter                                
  mySmoothingFilterType::Pointer filter = mySmoothingFilterType::New();


  // Connect the input images
  filter->SetInput( inputImage ); 
  filter->SetDirection( 2 );  // apply along Z

  
  // Execute the filter
  std::cout << "Executing Smoothing filter...";
  filter->Update();
  std::cout << " Done !" << std::endl;




  // Declare the type for the  First Derivative
  typedef itk::FirstDerivativeRecursiveGaussianImageFilter<
                                            myImageType,
                                            myImageType,
                                            float       >  myFirstDerivativeFilterType;
            

  // Create a  Filter                                
  myFirstDerivativeFilterType::Pointer filter1 = myFirstDerivativeFilterType::New();


  // Connect the input images
  filter1->SetInput( inputImage ); 
  filter1->SetDirection( 2 );  // apply along Z

  
  // Execute the filter1
  std::cout << "Executing First Derivative filter...";
  filter1->Update();
  std::cout << " Done !" << std::endl;



  // Declare the type for the  Second Derivative
  typedef itk::SecondDerivativeRecursiveGaussianImageFilter<
                                            myImageType,
                                            myImageType,
                                            float       >  mySecondDerivativeFilterType;
            

  // Create a  Filter                                
  mySecondDerivativeFilterType::Pointer filter2 = mySecondDerivativeFilterType::New();


  // Connect the input images
  filter2->SetInput( inputImage ); 
  filter2->SetDirection( 2 );  // apply along Z

  
  // Execute the filter2
  std::cout << "Executing Second Derivative filter...";
  filter2->Update();
  std::cout << " Done !" << std::endl;



  
  // All objects should be automatically destroyed at this point
  return 0;

}




