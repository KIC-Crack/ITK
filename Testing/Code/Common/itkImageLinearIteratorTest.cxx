/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImageLinearIteratorTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <iostream>

#include "itkImage.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkImageLinearConstIteratorWithIndex.h"




int main()
{
  std::cout << "Creating an image of indices" << std::endl;

  const unsigned int ImageDimension = 3;

  typedef itk::Index< ImageDimension >             PixelType;

  typedef itk::Image< PixelType, ImageDimension >  ImageType;

  ImageType::Pointer myImage = ImageType::New();
  
  ImageType::SizeType size;

  size[0] = 100;
  size[1] = 100;
  size[2] = 100;

  ImageType::IndexType start;
  start = ImageType::IndexType::ZeroIndex;

  ImageType::RegionType region;
  region.SetIndex( start );
  region.SetSize( size );

  myImage->SetLargestPossibleRegion( region );
  myImage->SetBufferedRegion( region );
  myImage->SetRequestedRegion( region );
  myImage->Allocate();

  typedef itk::ImageLinearIteratorWithIndex< ImageType > IteratorType;

  typedef itk::ImageLinearConstIteratorWithIndex< ImageType > ConstIteratorType;

  IteratorType it( myImage, region );


  it.GoToBegin();
  it.SetDirection( 0 ); // 0=x, 1=y, 2=z

  ImageType::IndexType index;
  
  while( !it.IsAtEnd() )
  {
    while( !it.IsAtEndOfLine() )
    {
      index = it.GetIndex();
      it.Set( index );
      ++it;
    }
    it.NextLine();
  }

  
  // Verification 
  IteratorType ot( myImage, region );

  ot.GoToBegin();
  ot.SetDirection( 0 ); // 0=x, 1=y, 2=z
 
  std::cout << "Verifying iterator... ";

  while( !ot.IsAtEnd() )
  {
    while( !ot.IsAtEndOfLine() )
    {
      index = ot.GetIndex();
      if( ot.Get() != index )
      {
        std::cerr << "Values don't correspond to what was stored "
          << std::endl;
        std::cerr << "Test failed at index ";
        std::cerr << index << std::endl;
        return EXIT_FAILURE;
      }
      ++ot;
    }
    ot.NextLine();
  }
  std::cout << "   Done ! " << std::endl;

  
  // Verification 
  ConstIteratorType cot( myImage, region );

  cot.GoToBegin();
  cot.SetDirection( 0 ); // 0=x, 1=y, 2=z
 
  std::cout << "Verifying const iterator... ";

  while( !cot.IsAtEnd() )
  {
    while( !cot.IsAtEndOfLine() )
    {
      index = cot.GetIndex();
      if( cot.Get() != index )
      {
        std::cerr << "Values don't correspond to what was stored "
          << std::endl;
        std::cerr << "Test failed at index ";
        std::cerr << index << " value is " << cot.Get() <<  std::endl;
        return EXIT_FAILURE;
      }
      ++cot;
    }
    cot.NextLine();
  }
  std::cout << "   Done ! " << std::endl;



  // Verification 
  std::cout << "Verifying iterator in reverse direction... ";

  IteratorType ior( myImage, region );

  ior.GoToEnd();
  ior.SetDirection( 0 ); // 0=x, 1=y, 2=z
 

  while( !ior.IsAtBegin() )
  {
    while( !ior.IsAtBeginOfLine() )
    {
      index = ior.GetIndex();
      if( ior.Get() != index )
      {
        std::cerr << "Values don't correspond to what was stored "
          << std::endl;
        std::cerr << "Test failed at index ";
        std::cerr << index << " value is " << ior.Get() <<  std::endl;
        return EXIT_FAILURE;
      }
      --ior;
    }
    ior.PreviousLine();
  }
  std::cout << "   Done ! " << std::endl;



  // Verification 
  std::cout << "Verifying const iterator in reverse direction... ";

  ConstIteratorType cor( myImage, region );

  cor.GoToEnd();
  cor.SetDirection( 0 ); // 0=x, 1=y, 2=z
 

  while( !cor.IsAtBegin() )
  {
    while( !cor.IsAtBeginOfLine() )
    {
      index = cor.GetIndex();
      if( cor.Get() != index )
      {
        std::cerr << "Values don't correspond to what was stored "
          << std::endl;
        std::cerr << "Test failed at index ";
        std::cerr << index << " value is " << cor.Get() <<  std::endl;
        return EXIT_FAILURE;
      }
      --cor;
    }
    cor.PreviousLine();
  }
  std::cout << "   Done ! " << std::endl;




  std::cout << "Test passed" << std::endl;




  return EXIT_SUCCESS;

}



