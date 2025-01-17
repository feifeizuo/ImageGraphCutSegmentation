/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "ITKHelpers.h"

// ITK
#include "itkComposeImageFilter.h"
#include "itkRGBToLuminanceImageFilter.h"
#include "itkVectorMagnitudeImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"


#include "Helpers/Helpers.h"

#include <cmath>

namespace ITKHelpers
{

std::vector<itk::Index<2> > GetDownsampledIndicesInRegion(const itk::ImageRegion<2>& region, const itk::SizeValueType stride)
{
  std::vector<itk::Index<2> > indices;

  for(itk::SizeValueType i = 0; i < region.GetSize()[0]; i += stride)
  {
    for(itk::SizeValueType j = 0; j < region.GetSize()[1]; j += stride)
    {
    itk::Index<2> currentIndex = {{static_cast<itk::Index<2>::IndexValueType>(i),
                                   static_cast<itk::Index<2>::IndexValueType>(j)}};
    indices.push_back(currentIndex);
    }
  }

  return indices;
}

std::vector<itk::Index<2> > GetIndicesInRegion(const itk::ImageRegion<2>& region)
{
  std::vector<itk::Index<2> > indices;

  typedef itk::Image<unsigned char, 2> DummyImageType;
  DummyImageType::Pointer image = DummyImageType::New();
  image->SetRegions(region);
  image->Allocate();

  itk::ImageRegionConstIteratorWithIndex<DummyImageType> regionIterator(image, region);
  while(!regionIterator.IsAtEnd())
  {
    indices.push_back(regionIterator.GetIndex());
    ++regionIterator;
  }
  return indices;
}

itk::ImageRegion<2> GetQuadrant(const itk::ImageRegion<2>& region, const unsigned int requestedQuadrant)
{
  // Note: the four quadrants might not cover the entire 'region'.

  // If the region is smaller than 2x2, it doesn't make sense to get a quadrant of it
  if((region.GetSize()[0] < 2) || (region.GetSize()[1] < 2))
  {
//    std::stringstream ss;
//    ss << "GetQuadrant(): 'region' is only " << region.GetSize();
//    throw std::runtime_error(ss.str());

    std::cerr << "Warning: GetQuadrant(): 'region' is only " << region.GetSize() << std::endl;
    return region;
  }

  itk::Offset<2>::OffsetValueType quadrantSideLength = region.GetSize()[0]/2;
  itk::Size<2> size = {{static_cast<itk::SizeValueType>(quadrantSideLength),
                        static_cast<itk::SizeValueType>(quadrantSideLength)}};
  itk::Index<2> corner;
  if(requestedQuadrant == 0)
  {
    corner = region.GetIndex();
  }
  else if(requestedQuadrant == 1)
  {
    itk::Offset<2> offset = {{quadrantSideLength, 0}};
    corner = region.GetIndex() + offset;
  }
  else if(requestedQuadrant == 2)
  {
    itk::Offset<2> offset = {{0, quadrantSideLength}};
    corner = region.GetIndex() + offset;
  }
  else if(requestedQuadrant == 3)
  {
    itk::Offset<2> offset = {{quadrantSideLength, quadrantSideLength}};
    corner = region.GetIndex() + offset;
  }
  else
  {
    std::stringstream ss;
    ss << "There are only 4 quadrants (0-3). Requested " << requestedQuadrant;
    throw std::runtime_error(ss.str());
  }

  itk::ImageRegion<2> quadrant(corner, size);
  return quadrant;
}

unsigned int GetNumberOfComponentsPerPixelInFile(const std::string& filename)
{
//  typedef itk::VectorImage<float, 2> TestImageType;
//  typedef  itk::ImageFileReader<TestImageType> ImageReaderType;
//  ImageReaderType::Pointer imageReader = ImageReaderType::New();
//  imageReader->SetFileName(filename);
//  imageReader->Update();

//  return imageReader->GetOutput()->GetNumberOfComponentsPerPixel();

  itk::ImageIOBase::Pointer imageIO =
        itk::ImageIOFactory::CreateImageIO(
            filename.c_str(), itk::ImageIOFactory::ReadMode);
  return imageIO->GetNumberOfComponents();
}

std::string GetIndexString(const itk::Index<2>& index)
{
  std::stringstream ss;
  ss << "(" << index[0] << ", " << index[1] << ")";
  return ss.str();
}

std::string GetSizeString(const itk::Size<2>& size)
{
  std::stringstream ss;
  ss << "(" << size[0] << ", " << size[1] << ")";
  return ss.str();
}

FloatVector2Type AverageVectors(const std::vector<FloatVector2Type>& vectors)
{
  FloatVector2Type totalVector;
  totalVector.Fill(0);

  if(vectors.size() == 0)
  {
    std::cerr << "Cannot average 0 vectors!" << std::endl;
    return totalVector;
  }

  for(auto vec : vectors)
  {
    totalVector[0] += vec[0];
    totalVector[1] += vec[1];
  }

  FloatVector2Type averageVector;
  averageVector[0] = totalVector[0] / static_cast<float>(vectors.size());
  averageVector[1] = totalVector[1] / static_cast<float>(vectors.size());

  return averageVector;
}

float AngleBetween(const FloatVector2Type& v1, const FloatVector2Type& v2)
{
  FloatVector2Type v1normalized = v1;
  v1normalized.Normalize();

  FloatVector2Type v2normalized = v2;
  v2normalized.Normalize();

  return acos(v1normalized*v2normalized);
}

itk::Index<2> GetNextPixelAlongVector(const itk::Index<2>& pixel, const FloatVector2Type& vector)
{
  itk::Index<2> nextPixel = pixel + GetOffsetAlongVector(vector);

  return nextPixel;
}

itk::Offset<2> GetOffsetAlongVector(const FloatVector2Type& vector)
{
  FloatVector2Type normalizedVector = vector;
  normalizedVector.Normalize();

  itk::Offset<2> offset;
  offset[0] = Helpers::RoundAwayFromZero(normalizedVector[0]);
  offset[1] = Helpers::RoundAwayFromZero(normalizedVector[1]);

  return offset;
}


itk::Size<2> SizeFromRadius(const unsigned int radius)
{
  itk::Size<2> size;
  size.Fill(Helpers::SideLengthFromRadius(radius));

  return size;
}

itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2>& pixel, const unsigned int radius)
{
  // This function returns a Region with the specified 'radius' centered at 'pixel'. By the definition of the radius of a square patch, the output region is (radius*2 + 1)x(radius*2 + 1).
  // Note: This region is not necessarily entirely inside the image!

  // The "index" is the lower left corner, so we need to subtract the radius from the center to obtain it
  itk::Index<2> lowerLeft;
  lowerLeft[0] = pixel[0] - radius;
  lowerLeft[1] = pixel[1] - radius;

  itk::ImageRegion<2> region;
  region.SetIndex(lowerLeft);
  itk::Size<2> size;
  size[0] = radius*2 + 1;
  size[1] = radius*2 + 1;
  region.SetSize(size);

  return region;
}


itk::Index<2> GetRegionCenter(const itk::ImageRegion<2>& region)
{
  // This assumes that the region is an odd size in both dimensions
  itk::Index<2> center;
  center[0] = region.GetIndex()[0] + region.GetSize()[0] / 2;
  center[1] = region.GetIndex()[1] + region.GetSize()[1] / 2;

  return center;
}

itk::Offset<2> OffsetFrom1DOffset(const itk::Offset<1>& offset1D, const unsigned int dimension)
{
  // Manually construct a 2D offset with 0 in all dimensions except the specified dimension
  itk::Offset<2> offset;
  offset.Fill(0);
  offset[dimension] = offset1D[0];

  return offset;
}

void OutputImageType(const itk::ImageBase<2>* const input)
{
  if(dynamic_cast<const FloatScalarImageType*>(input))
    {
    std::cout << "Image type FloatScalarImageType" << std::endl;
    }
  else if(dynamic_cast<const UnsignedCharScalarImageType*>(input))
    {
    std::cout << "Image type UnsignedCharScalarImageType" << std::endl;
    }
  else if(dynamic_cast<const FloatVectorImageType*>(input))
    {
    std::cout << "Image type FloatVectorImageType" << std::endl;
    }
  else
    {
    std::cout << "OutputImageType: Image is Invalid type!" << std::endl;
    }
}

// The return value MUST be a smart pointer
itk::ImageBase<2>::Pointer CreateImageWithSameType(const itk::ImageBase<2>* const input)
{
  itk::LightObject::Pointer objectCopyLight = input->CreateAnother();

  itk::ImageBase<2>::Pointer objectCopy = dynamic_cast<itk::ImageBase<2>*>(objectCopyLight.GetPointer());

  return objectCopy;
}

std::vector<itk::Index<2> > Get8Neighbors(const itk::Index<2>& pixel)
{
  std::vector<itk::Index<2> > neighborsInRegion;

  std::vector<itk::Offset<2> > neighborOffsets = Get8NeighborOffsets();
  for(auto offset : neighborOffsets)
  {
    itk::Index<2> index = pixel + offset;
    neighborsInRegion.push_back(index);
  }
  return neighborsInRegion;
}

std::vector<itk::Index<2> > Get8NeighborsInRegion(const itk::ImageRegion<2>& region, const itk::Index<2>& pixel)
{
  std::vector<itk::Index<2> > neighborsInRegion;

  std::vector<itk::Offset<2> > neighborOffsets = Get8NeighborOffsets();
  for(auto offset : neighborOffsets)
  {
    itk::Index<2> index = pixel + offset;
    if(region.IsInside(index))
    {
      neighborsInRegion.push_back(index);
    }
  }
  return neighborsInRegion;
}

std::vector<itk::Offset<2> > Get8NeighborOffsets()
{
  std::vector<itk::Offset<2> > offsets;

  for(int i = -1; i <= 1; ++i)
  {
    for(int j = -1; j <= 1; ++j)
    {
      if(i == 0 && j == 0)
      {
        continue;
      }
      itk::Offset<2> offset;
      offset[0] = i;
      offset[1] = j;
      offsets.push_back(offset);
    }
  }
  return offsets;
}

// itk::VariableLengthVector<float> Average(const std::vector<itk::VariableLengthVector<float> >& v)
// {
//   // std::cout << "ITKHelpers::Average" << std::endl;
//   if(v.size() == 0)
//   {
//     throw std::runtime_error("Cannot average vector with size 0!");
//   }
//   itk::VariableLengthVector<float> vectorSum;
//   vectorSum.SetSize(v[0].GetSize());
//   vectorSum.Fill(0);
//
//   for(unsigned int i = 0; i < v.size(); ++i)
//     {
//     //std::cout << "Average: Adding value " << v[i] << std::endl;
//     vectorSum += v[i];
//     //std::cout << "Average: Current vectorSum " << vectorSum << std::endl;
//     }
//
//   itk::VariableLengthVector<float> averageVector;
//   averageVector.SetSize(v[0].GetSize());
//   averageVector = vectorSum / static_cast<float>(v.size());
//
//   return averageVector;
// }

std::vector<itk::Index<2> > OffsetsToIndices(const std::vector<itk::Offset<2> >& offsets, const itk::Index<2>& index)
{
  std::vector<itk::Index<2> > indices;
  for(auto offset : offsets)
  {
    indices.push_back(index + offset);
  }
  return indices;
}

std::vector<itk::Index<2> > OffsetsToIndices(const std::vector<itk::Offset<2> >& offsets)
{
  std::vector<itk::Index<2> > indices;
  for(auto offset : offsets)
  {
    indices.push_back(CreateIndex(offset));
  }
  return indices;
}

std::vector<itk::Offset<2> > IndicesToOffsets(const std::vector<itk::Index<2> >& indices, const itk::Index<2>& referenceIndex)
{
  std::vector<itk::Offset<2> > offsets;
  for(auto index : indices)
  {
    offsets.push_back(index - referenceIndex);
  }
  return offsets;
}

std::vector<itk::Index<2> > GetBoundaryPixels(const itk::ImageRegion<2>& region, const unsigned int thickness)
{
  std::vector<itk::Index<2> > boundaryPixels;

  typedef itk::Image<float,2> DummyImageType;
  DummyImageType::Pointer dummyImage = DummyImageType::New();
  dummyImage->SetRegions(region);

  itk::ImageRegionIteratorWithIndex<DummyImageType> imageIterator(dummyImage, region);

  while(!imageIterator.IsAtEnd())
  {
    if( (std::abs(imageIterator.GetIndex()[0] - region.GetIndex()[0]) < static_cast<itk::Index<2>::IndexValueType>(thickness)) ||
		(std::abs(imageIterator.GetIndex()[0] - (region.GetIndex()[0] + static_cast<itk::Index<2>::IndexValueType>(region.GetSize()[0]) - 1)) < static_cast<itk::Index<2>::IndexValueType>(thickness)) ||
        (std::abs(imageIterator.GetIndex()[1] - region.GetIndex()[1]) < static_cast<itk::Index<2>::IndexValueType>(thickness)) ||
        (std::abs(imageIterator.GetIndex()[1] - (region.GetIndex()[1] + static_cast<itk::Index<2>::IndexValueType>(region.GetSize()[1]) - 1)) < static_cast<itk::Index<2>::IndexValueType>(thickness)))
    {
      boundaryPixels.push_back(imageIterator.GetIndex());
    }
    ++imageIterator;
  }

  return boundaryPixels;
}

std::vector<itk::Index<2> > GetBoundaryPixels(const itk::ImageRegion<2>& region)
{
  std::vector<itk::Index<2> > boundaryPixels;

  for(unsigned int i = region.GetIndex()[0]; i < region.GetIndex()[0] + region.GetSize()[0]; ++i)
  {
    itk::Index<2> index;
    index[0] = i;
    index[1] = region.GetIndex()[1];
    boundaryPixels.push_back(index);

    index[0] = i;
    index[1] = region.GetIndex()[1] + region.GetSize()[1] - 1;
    boundaryPixels.push_back(index);
  }

  for(unsigned int j = region.GetIndex()[1]; j < region.GetIndex()[1] + region.GetSize()[1]; ++j)
  {
    itk::Index<2> index;
    index[0] = region.GetIndex()[0];
    index[1] = j;
    boundaryPixels.push_back(index);

    index[0] = region.GetIndex()[0] + region.GetSize()[0] - 1;
    index[1] = j;
    boundaryPixels.push_back(index);
  }

  return boundaryPixels;
}

itk::ImageRegion<2> CornerRegion(const itk::Size<2>& size)
{
  itk::Index<2> corner = {{0,0}};
  itk::ImageRegion<2> region(corner, size);
  return region;
}

void Write2DVectorImage(const FloatVector2ImageType* const image, const std::string& filename)
{
  Write2DVectorRegion(image, image->GetLargestPossibleRegion(), filename);
}

void Write2DVectorRegion(const FloatVector2ImageType* const image, const itk::ImageRegion<2>& region, const std::string& filename)
{
  // This is a separate function than WriteRegion because Paraview requires vectors to be 3D to glyph them.

  typedef itk::RegionOfInterestImageFilter<FloatVector2ImageType, FloatVector2ImageType> RegionOfInterestImageFilterType;

  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(region);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  itk::Point<float, 2> origin;
  origin.Fill(0);
  regionOfInterestImageFilter->GetOutput()->SetOrigin(origin);

  FloatVector3ImageType::Pointer vectors3D = FloatVector3ImageType::New();
  vectors3D->SetRegions(regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());
  vectors3D->Allocate();

  itk::ImageRegionConstIterator<FloatVector2ImageType> iterator(regionOfInterestImageFilter->GetOutput(),
                                                                regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  while(!iterator.IsAtEnd())
  {
    FloatVector2Type vec2d = iterator.Get();
    FloatVector3Type vec3d;
    vec3d[0] = vec2d[0];
    vec3d[1] = vec2d[1];
    vec3d[2] = 0;

    vectors3D->SetPixel(iterator.GetIndex(), vec3d);
    ++iterator;
  }

  //std::cout << "regionOfInterestImageFilter " << regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion() << std::endl;

  itk::ImageFileWriter<FloatVector3ImageType>::Pointer writer = itk::ImageFileWriter<FloatVector3ImageType>::New();
  writer->SetFileName(filename);
  writer->SetInput(vectors3D);
  writer->Update();
}

std::vector<itk::Index<2> > DilatePixelList(const std::vector<itk::Index<2> >& pixelList,
                                            const itk::ImageRegion<2>& region, const unsigned int radius)
{
  //std::cout << "DilatePixelList: input has " << pixelList.size() << " pixels." << std::endl;
  // Construct an image of the pixels in the list
  typedef itk::Image<unsigned char, 2> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(0);

  typedef std::vector<itk::Index<2> > PixelVectorType;

  for(auto iter = pixelList.begin(); iter != pixelList.end(); ++iter)
  {
    // Note, this must be 255, not just any non-zero number, for BinaryDilateImageFilter to work properly.
    image->SetPixel(*iter, 255);
  }

  //WriteImage(image.GetPointer(), "beforeDilation.png");

  // Dilate the image
  typedef itk::BinaryBallStructuringElement<ImageType::PixelType,2> StructuringElementType;
  StructuringElementType structuringElement;
  structuringElement.SetRadius(radius);
  structuringElement.CreateStructuringElement();

  typedef itk::BinaryDilateImageFilter<ImageType, ImageType, StructuringElementType> BinaryDilateImageFilterType;

  BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
  dilateFilter->SetInput(image);
  dilateFilter->SetKernel(structuringElement);
  dilateFilter->Update();

  //WriteImage(dilateFilter->GetOutput(), "afterDilation.png");

  PixelVectorType dilatedPixelList;

  itk::ImageRegionConstIteratorWithIndex<ImageType> imageIterator(dilateFilter->GetOutput(),
                                                         dilateFilter->GetOutput()->GetLargestPossibleRegion());
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get())
      {
      dilatedPixelList.push_back(imageIterator.GetIndex());
      }
    ++imageIterator;
    }

  //std::cout << "DilatePixelList: output has " << dilatedPixelList.size() << " pixels." << std::endl;
  return dilatedPixelList;
}


void IndicesToBinaryImage(const std::vector<itk::Index<2> >& indices, UnsignedCharScalarImageType* const image)
{
  // The Regions of the 'image' must be set before calling this function
  //std::cout << "Setting " << indices.size() << " points to non-zero." << std::endl;

  image->Allocate();
  image->FillBuffer(0);

  // Set the pixels of indices in list to 255
  for(auto index : indices)
  {
    image->SetPixel(index, 255);
  }
}

std::vector<itk::Index<2> > Get4NeighborIndicesInsideRegion(const itk::Index<2>& pixel,
                                                            const itk::ImageRegion<2>& region)
{
  std::vector<itk::Index<2> > indices;

  itk::Offset<2> offset;
  offset[0] = -1;
  offset[1] = 0;

  if(region.IsInside(pixel + offset))
  {
    indices.push_back(pixel + offset);
  }

  offset[0] = 1;
  offset[1] = 0;

  if(region.IsInside(pixel + offset))
  {
    indices.push_back(pixel + offset);
  }

  offset[0] = 0;
  offset[1] = -1;

  if(region.IsInside(pixel + offset))
  {
    indices.push_back(pixel + offset);
  }

  offset[0] = 0;
  offset[1] = 1;

  if(region.IsInside(pixel + offset))
  {
    indices.push_back(pixel + offset);
  }

  return indices;
}

itk::ImageRegion<2> GetInternalRegion(const itk::ImageRegion<2>& wholeRegion,
                                      const unsigned int patchRadius)
{
  unsigned int width = wholeRegion.GetSize()[0];
  unsigned int height = wholeRegion.GetSize()[1];

  itk::Index<2> regionCorner = {{static_cast<itk::Index<2>::IndexValueType>(patchRadius),
                                 static_cast<itk::Index<2>::IndexValueType>(patchRadius)}};
  itk::Size<2> regionSize = {{width - 2*patchRadius, height - 2*patchRadius}};
  itk::ImageRegion<2> region(regionCorner, regionSize);

  return region;
}

std::vector<itk::ImageRegion<2> > GetPatchesCenteredAtIndices(const std::vector<itk::Index<2> >& indices,
                                                              const unsigned int patchRadius)
{
  std::vector<itk::ImageRegion<2> > regions(indices.size());
  for(size_t i = 0; i < indices.size(); ++i)
  {
    regions[i] = GetRegionInRadiusAroundPixel(indices[i], patchRadius);
  }
  return regions;
}

std::vector<itk::ImageRegion<2> > GetValidPatchesCenteredAtIndices(const std::vector<itk::Index<2> >& indices,
                                                                   const itk::ImageRegion<2>& imageRegion,
                                                                   const unsigned int patchRadius)
{
  std::vector<itk::ImageRegion<2> > regions;
  for(auto index : indices)
  {
    itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(index, patchRadius);

    if(imageRegion.IsInside(region))
    {
      regions.push_back(region);
    }
  }
  return regions;
}

std::vector<itk::ImageRegion<2> > GetAllPatches(const itk::ImageRegion<2>& fullImageRegion,
                                                const unsigned int patchRadius)
{
  typedef itk::Image<float,2> DummyImageType;
  DummyImageType::Pointer dummyImage = DummyImageType::New();
  dummyImage->SetRegions(fullImageRegion);
  //dummyImage->Allocate(); // Do we actually need this to iterate over the image?

  itk::Size<2> patchSize;
  patchSize.Fill(patchRadius * 2 + 1);

  itk::ImageRegion<2> fullRegion = dummyImage->GetLargestPossibleRegion();

  itk::ImageRegionIteratorWithIndex<DummyImageType> imageIterator(dummyImage, fullRegion);

  std::vector<itk::ImageRegion<2> > regions;

  while(!imageIterator.IsAtEnd())
  {
    itk::ImageRegion<2> possibleRegion;
    possibleRegion.SetIndex(imageIterator.GetIndex());
    possibleRegion.SetSize(patchSize);

    if(fullRegion.IsInside(possibleRegion))
    {
      regions.push_back(possibleRegion);
    }
    ++imageIterator;
  }

  return regions;
}

std::vector<itk::ImageRegion<2> > GetAllPatchesContainingPixel(const itk::Index<2>& pixel,
                                                               const unsigned int patchRadius,
                                                               const itk::ImageRegion<2>& imageRegion)
{
  typedef itk::Image<float,2> DummyImageType;
  DummyImageType::Pointer dummyImage = DummyImageType::New();
  dummyImage->SetRegions(imageRegion);
  //dummyImage->Allocate(); // Do we actually need this to iterate over the image?

  // This region includes all patch centers
  itk::Index<2> possibleRegionCorner = {{pixel[0] - static_cast<itk::Index<2>::IndexValueType>(patchRadius),
                                         pixel[1] - static_cast<itk::Index<2>::IndexValueType>(patchRadius)}};
  itk::Size<2> possibleRegionSize = {{patchRadius*2 + 1, patchRadius*2 + 1}};
  itk::ImageRegion<2> possibleRegion(possibleRegionCorner, possibleRegionSize);

  // Don't include patch centers that fall outside the image
  possibleRegion.Crop(imageRegion);

  itk::ImageRegionIteratorWithIndex<DummyImageType> imageIterator(dummyImage, possibleRegion);

  std::vector<itk::ImageRegion<2> > regions;

  // Each pixel in this loop is a potential patch center
  while(!imageIterator.IsAtEnd())
  {
    itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), patchRadius);
    if(imageRegion.IsInside(region))
    {
      regions.push_back(region);
    }
    ++imageIterator;
  }

  return regions;
}

itk::Size<2> MakeSizeEven(const itk::Size<2>& inputSize)
{
  itk::Size<2> outputSize = inputSize;

  if(Helpers::IsOdd(inputSize[0]))
  {
    outputSize[0]--;
  }

  if(Helpers::IsOdd(inputSize[1]))
  {
    outputSize[1]--;
  }

  return outputSize;
}

float IndexDistance(const itk::Index<2>& p0, const itk::Index<2>& p1)
{
  itk::Point<float,2> point0;
  point0[0] = p0[0];
  point0[1] = p0[1];

  itk::Point<float,2> point1;
  point1[0] = p1[0];
  point1[1] = p1[1];

  return point1.EuclideanDistanceTo(point0);
}

unsigned int ClosestIndexId(const std::vector<itk::Index<2> >& pixels, const itk::Index<2>& queryPixel)
{
  float minDistance = std::numeric_limits<float>::max();
  unsigned int closestId = 0;

  for(unsigned int i = 0; i < pixels.size(); ++i)
  {
    float distance = IndexDistance(queryPixel, pixels[i]);
    if(distance < minDistance)
    {
      closestId = i;
      minDistance = distance;
    }
  }

  return closestId;
}

itk::ImageIOBase::IOComponentType GetPixelTypeFromFile(const std::string& filename)
{
  itk::ImageIOBase::Pointer imageIO =
  itk::ImageIOFactory::CreateImageIO(
      filename.c_str(), itk::ImageIOFactory::ReadMode);
  imageIO->ReadImageInformation();

  typedef itk::ImageIOBase::IOComponentType ScalarPixelType;

  const ScalarPixelType pixelType = imageIO->GetComponentType();

//   std::cout << "Pixel Type is " << imageIO->GetComponentTypeAsString(pixelType)
//             << std::endl;

  return pixelType;
}

bool IsNeighbor(const itk::Index<2>& index1, const itk::Index<2>& index2)
{
  std::vector<itk::Index<2> > neighbors = Get8Neighbors(index1);

  for(auto neighbor : neighbors)
  {
    if(neighbor == index2)
    {
      return true;
    }
  }
  return false;
}

std::vector<itk::ImageRegion<2> > Get8NeighborRegionsInRegion(const itk::ImageRegion<2>& searchRegion, const itk::Index<2>& pixel,
                                                              const itk::Size<2>& queryRegionSize)
{
  std::vector<itk::ImageRegion<2> > validNeighborRegions;

  std::vector<itk::Index<2> > neighborPixels = Get8NeighborsInRegion(searchRegion, pixel);

  for(auto neighborhoodPixel : neighborPixels)
  {
    itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(neighborhoodPixel, queryRegionSize[0]/2);
    if(searchRegion.IsInside(region))
    {
      validNeighborRegions.push_back(region);
    }
  }

  return validNeighborRegions;
}

itk::ImageRegion<2> DilateRegion(const itk::ImageRegion<2>& region, const unsigned int radius)
{
  itk::ImageRegion<2> dilatedRegion;

  itk::Index<2> dilatedRegionCorner = region.GetIndex();
  dilatedRegionCorner[0] -= (radius + 1);
  dilatedRegionCorner[1] -= (radius + 1);
  dilatedRegion.SetIndex(dilatedRegionCorner);

  // 2*radius is the number of pixels that a patch can be shifted and still
  // touch the original region. The second *2 is because this is possible on both sides.
  itk::Size<2> dilatedRegionSize = region.GetSize();
  dilatedRegionSize[0] += (radius*2 * 2);
  dilatedRegionSize[1] += (radius*2 * 2);
  dilatedRegion.SetSize(dilatedRegionSize);

  return dilatedRegion;
}

itk::ImageRegion<2> ErodeRegion(const itk::ImageRegion<2>& region, const unsigned int radius)
{
  itk::ImageRegion<2> erodedRegion;

  itk::Index<2> erodedRegionCorner = region.GetIndex();
  erodedRegionCorner[0] += (radius + 1);
  erodedRegionCorner[1] += (radius + 1);
  erodedRegion.SetIndex(erodedRegionCorner);

  // 2*radius is the number of pixels that a patch can be shifted and still
  // touch the original region. The second *2 is because this is possible on both sides.
  itk::Size<2> erodedRegionSize = region.GetSize();
  erodedRegionSize[0] -= (radius*2 * 2);
  erodedRegionSize[1] -= (radius*2 * 2);
  erodedRegion.SetSize(erodedRegionSize);

  return erodedRegion;
}

void HighlightAndWriteRegions(const itk::Size<2>& imageSize, const std::vector<itk::ImageRegion<2> >& regions, const std::string& filename)
{
  UnsignedCharScalarImageType::Pointer image = UnsignedCharScalarImageType::New();
  itk::Index<2> corner = {{0,0}};
  
  itk::ImageRegion<2> region(corner, imageSize);
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(0);

  for(auto region : regions)
  {
    SetRegionToConstant(image.GetPointer(), region, 255);
  }

  WriteImage(image.GetPointer(), filename);
}

itk::Offset<2> IndexToOffset(const itk::Index<2>& index)
{
  itk::Offset<2> offset = {{index[0], index[1]}};
  return offset;
}

std::vector<itk::ImageRegion<2> > DivideRegion(const itk::ImageRegion<2>& region, const unsigned int divisionsPerDimension)
{
  assert(divisionsPerDimension > 0);

  unsigned int numberOfPixelsX = region.GetSize()[0];
  while(numberOfPixelsX % divisionsPerDimension != 0)
  {
    numberOfPixelsX -= 1;
  }

  unsigned int numberOfPixelsY = region.GetSize()[1];
  while(numberOfPixelsY % divisionsPerDimension != 0)
  {
    numberOfPixelsY -= 1;
  }

  std::vector<itk::ImageRegion<2> > subregions;

  std::vector<itk::Index<2>::IndexValueType> startingPointsX;
  unsigned int startingPoint = 0;
  while(startingPoint < numberOfPixelsX)
  {
    startingPointsX.push_back(startingPoint);
    startingPoint += numberOfPixelsX/divisionsPerDimension;
  }

  assert(startingPointsX.size() == divisionsPerDimension);

  std::vector<itk::Index<2>::IndexValueType> startingPointsY;
  startingPoint = 0;
  while(startingPoint < numberOfPixelsY)
  {
    startingPointsY.push_back(startingPoint);
    startingPoint += numberOfPixelsY/divisionsPerDimension;
  }

  assert(startingPointsY.size() == divisionsPerDimension);

  itk::Size<2> subregionSize = {{numberOfPixelsX/divisionsPerDimension, numberOfPixelsY/divisionsPerDimension}};
  for(unsigned int i = 0; i < startingPointsX.size(); ++i)
  {
    for(unsigned int j = 0; j < startingPointsY.size(); ++j)
    {
      itk::Index<2> corner = {{startingPointsX[i], startingPointsY[j]}};
      itk::ImageRegion<2> subregion(corner, subregionSize);
      subregions.push_back(subregion);
    }
  }

  return subregions;
}

itk::ImageRegion<2> CropRegionAtPosition(itk::ImageRegion<2> regionToCrop, const itk::ImageRegion<2>& fullRegion, itk::ImageRegion<2> cropPosition)
{
  if(regionToCrop.GetSize() != cropPosition.GetSize())
  {
    throw std::runtime_error("CropRegionAtPosition only makes sense if the regions are the same size!");
  }

  itk::Offset<2> offset = cropPosition.GetIndex() - regionToCrop.GetIndex();

  regionToCrop.SetIndex(regionToCrop.GetIndex() + offset);
  regionToCrop.Crop(fullRegion);
  regionToCrop.SetIndex(regionToCrop.GetIndex() - offset);

  return regionToCrop;
}

void WriteBoolImage(const itk::Image<bool, 2>* const image, const std::string& fileName)
{
  typedef itk::Image<unsigned char, 2> UnsignedCharImageType;
  UnsignedCharImageType::Pointer unsignedCharImage = UnsignedCharImageType::New();
  unsignedCharImage->SetRegions(image->GetLargestPossibleRegion());
  unsignedCharImage->Allocate();

  itk::ImageRegionIteratorWithIndex<UnsignedCharImageType>
      unsignedCharImageIterator(unsignedCharImage, unsignedCharImage->GetLargestPossibleRegion());
  while(!unsignedCharImageIterator.IsAtEnd())
    {
    if(image->GetPixel(unsignedCharImageIterator.GetIndex()))
    {
      unsignedCharImageIterator.Set(255);
    }
    else
    {
      unsignedCharImageIterator.Set(0);
    }
    ++unsignedCharImageIterator;
    }

  WriteImage(unsignedCharImage.GetPointer(), fileName);
}

void WriteIndexImage(const itk::Image<itk::Index<2>, 2>* const image, const std::string& fileName)
{
  typedef itk::Image<itk::CovariantVector<int, 2> > VectorImageType;
  VectorImageType::Pointer vectorImage = VectorImageType::New();
  vectorImage->SetRegions(image->GetLargestPossibleRegion());
  vectorImage->Allocate();

  itk::ImageRegionIteratorWithIndex<VectorImageType>
      vectorImageIterator(vectorImage, vectorImage->GetLargestPossibleRegion());
  while(!vectorImageIterator.IsAtEnd())
  {
    VectorImageType::PixelType vectorPixel;
    vectorPixel[0] = image->GetPixel(vectorImageIterator.GetIndex())[0];
    vectorPixel[1] = image->GetPixel(vectorImageIterator.GetIndex())[1];

    vectorImageIterator.Set(vectorPixel);

    ++vectorImageIterator;
  }

  WriteImage(vectorImage.GetPointer(), fileName);
}

} // end namespace
