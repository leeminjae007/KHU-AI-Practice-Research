/*=========================================================================
 *
 *  Copyright Insight Software Consortium
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
#ifndef __itkAverageOverDimensionImageFilter_h
#define __itkAverageOverDimensionImageFilter_h

#include "itkInPlaceImageFilter.h"
#include "itkSmartPointer.h"
#include "itkExtractImageFilterRegionCopier.h"

namespace itk
{
/** \class AverageOverDimensionImageFilter
 * \brief Decrease the image size by cropping the image to the selected
 * region bounds.
 *
 * AverageOverDimensionImageFilter changes the image boundary of an image by removing
 * pixels outside the target region.  The target region must be specified.
 *
 * AverageOverDimensionImageFilter also collapses dimensions so that the input image
 * may have more dimensions than the output image (i.e. 4-D input image
 * to a 3-D output image).  To specify what dimensions to collapse,
 * the ExtractionRegion must be specified.  For any dimension dim where
 * ExtractionRegion.Size[dim] = 0, that dimension is collapsed.  The
 * index to collapse on is specified by ExtractionRegion.Index[dim].
 * For example, we have a image 4D = a 4x4x4x4 image, and we want
 * to get a 3D image, 3D = a 4x4x4 image, specified as [x,y,z,2] from 4D
 * (i.e. the 3rd "time" slice from 4D).  The ExtractionRegion.Size =
 * [4,4,4,0] and ExtractionRegion.Index = [0,0,0,2].
 *
 * The number of dimension in ExtractionRegion.Size and Index must =
 * InputImageDimension.  The number of non-zero dimensions in
 * ExtractionRegion.Size must = OutputImageDimension.
 *
 * The output image produced by this filter will have the same origin as the
 * input image, while the ImageRegion of the output image will start at the
 * starting index value provided in the ExtractRegion parameter.  If you are
 * looking for a filter that will re-compute the origin of the output image,
 * and provide an output image region whose index is set to zeros, then you may
 * want to use the RegionOfInterestImageFilter.  The output spacing is is
 * simply the collapsed version of the input spacing.
 *
 * Determining the direction of the collapsed output image from an larger
 * dimensional input space is an ill defined problem in general.  It is
 * required that the application developer select the desired transformation
 * strategy for collapsing direction cosigns.  It is REQUIRED that a strategy
 * be explicitly requested (i.e. there is no working default).
 * Direction Collapsing Strategies:
 *    1)  DirectionCollapseToUnknown();
 *            This is the default and the filter can not run when this is set.
 *            The reason is to explicitly force the application developer to
 *            define their desired behavior.
 *    1)  DirectionCollapseToIdentity();
 *            Output has identity direction no matter what
 *    2)  DirectionCollapseToSubmatrix();
 *            Output direction is the sub-matrix if it is positive definite, else throw an exception.
 *
 * This filter is implemented as a multithreaded filter.  It provides a
 * ThreadedGenerateData() method for its implementation.
 *
 * \note This filter is derived from InPlaceImageFilter. When the
 * input to this filter matched the output requirested region, like
 * with streaming filter for input, then setting this filter to run
 * in-place will result in no copying of the bulk pixel data.
 *
 * \sa CropImageFilter
 * \ingroup GeometricTransform
 * \ingroup ITKCommon
 *
 * \wiki
 * \wikiexample{ImageProcessing/AverageOverDimensionImageFilter,Crop an image by specifying the region to keep}
 * \endwiki
 */

template <typename TInputImage, typename TOutputImage>
class AverageOverDimensionImageFilter final : public InPlaceImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef AverageOverDimensionImageFilter               Self;
  typedef InPlaceImageFilter<TInputImage, TOutputImage> Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkOverrideGetNameOfClassMacro(AverageOverDimensionImageFilter);

  /** Image type information. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Typedef to describe the output and input image region types. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;
  typedef typename TInputImage::RegionType  InputImageRegionType;

  /** Typedef to describe the type of pixel. */
  typedef typename TOutputImage::PixelType OutputImagePixelType;
  typedef typename TInputImage::PixelType  InputImagePixelType;

  /** Typedef to describe the output and input image index and size types. */
  typedef typename TOutputImage::IndexType OutputImageIndexType;
  typedef typename TInputImage::IndexType  InputImageIndexType;
  typedef typename TOutputImage::SizeType  OutputImageSizeType;
  typedef typename TInputImage::SizeType   InputImageSizeType;

  typedef enum DirectionCollapseStrategyEnum
  {
    DIRECTIONCOLLAPSETOUNKOWN = 0,
    DIRECTIONCOLLAPSETOIDENTITY = 1,
    DIRECTIONCOLLAPSETOSUBMATRIX = 2,
    DIRECTIONCOLLAPSETOGUESS = 3
  } DIRECTIONCOLLAPSESTRATEGY;


  /**
   * Set the strategy to be used to collapse pysical space dimensions.
   *
   * itk::itkAverageOverDimensionImageFilter::DIRECTIONCOLLAPSETOIDENTITY
   * Set the strategy so that all collapsed images have an identity direction.
   * Use this strategy when you know that retention of the physical space
   * orientation of the collapsed image is not important.
   *
   * itk::itkAverageOverDimensionImageFilter::DIRECTIONCOLLAPSETOGUESS
   * Set the strategy so that all collapsed images where
   * output direction is the sub-matrix it it is positive definite, else
   * return identity. This is backwards compatible with ITKv3, but
   * is highly discouraged because the results are difficult to
   * anticipate under differing data scenerios.
   *
   * itk::itkAverageOverDimensionImageFilter::DIRECTIONCOLLAPSETOSUBMATRIX
   * Set the strategy so that all collapsed images where
   * output direction is the sub-matrix it it is positive definite,
   * else throw an exception.  Use this strategy when it is known
   * that properly identified physical space sub-volumes can be
   * reliably extracted from a higher dimensional space.  For
   * example when the application programmer knows that a 4D image
   * is 3D+time, and that the 3D sub-space is properly defined.
   */
  void
  SetDirectionCollapseToStrategy(const DIRECTIONCOLLAPSESTRATEGY choosenStrategy)
  {
    switch (choosenStrategy)
    {
      case DIRECTIONCOLLAPSETOGUESS:
      case DIRECTIONCOLLAPSETOIDENTITY:
      case DIRECTIONCOLLAPSETOSUBMATRIX:
        break;
      case DIRECTIONCOLLAPSETOUNKOWN:
      default:
        itkExceptionMacro(<< "Invalid Strategy Chosen for itk::AverageOverDimensionImageFilter");
    }

    this->m_DirectionCollapseStrategy = choosenStrategy;
    this->Modified();
  }

  /** NOTE:  The SetDirectionCollapseToUknown is explicitly not defined.
   * It is a state that a filter can be in only when it is first instantiate
   * prior to being initialized.
   */

  /**
   * Get the currently set strategy for collapsing directions of physical space.
   */
  DIRECTIONCOLLAPSESTRATEGY
  GetDirectionCollapseToStrategy() const
  {
    return this->m_DirectionCollapseStrategy;
  }

  /** \sa SetDirectionCollapseToStrategy */
  void
  SetDirectionCollapseToGuess()
  {
    this->SetDirectionCollapseToStrategy(DIRECTIONCOLLAPSETOGUESS);
  }

  /** \sa SetDirectionCollapseToStrategy */
  void
  SetDirectionCollapseToIdentity()
  {
    this->SetDirectionCollapseToStrategy(DIRECTIONCOLLAPSETOIDENTITY);
  }

  /** \sa SetDirectionCollapseToStrategy */
  void
  SetDirectionCollapseToSubmatrix()
  {
    this->SetDirectionCollapseToStrategy(DIRECTIONCOLLAPSETOSUBMATRIX);
  }


  /** ImageDimension enumeration */
  static constexpr unsigned int InputImageDimension = TInputImage::ImageDimension;
  static constexpr unsigned int OutputImageDimension = TOutputImage::ImageDimension;

  typedef ImageToImageFilterDetail::ExtractImageFilterRegionCopier<Self::InputImageDimension,
                                                                   Self::OutputImageDimension>
    ExtractImageFilterRegionCopierType;

  /** Set/Get the output image region.
   *  If any of the ExtractionRegion.Size = 0 for any particular dimension dim,
   *  we have to collapse dimension dim.  This means the output image will have
   *  'c' dimensions less than the input image, where c = number of
   *  ExtractionRegion.Size = 0. */
  void
  SetAveragingDimension(unsigned int averagingDimension);
  itkGetMacro(AveragingDimension, unsigned int);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputCovertibleToOutputCheck, (Concept::Convertible<InputImagePixelType, OutputImagePixelType>));
  /** End concept checking */
#endif

protected:
  AverageOverDimensionImageFilter();
  ~AverageOverDimensionImageFilter() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** AverageOverDimensionImageFilter can produce an image which is a different
   * resolution than its input image.  As such, AverageOverDimensionImageFilter
   * needs to provide an implementation for
   * GenerateOutputInformation() in order to inform the pipeline
   * execution model.  The original documentation of this method is
   * below.
   *
   * \sa ProcessObject::GenerateOutputInformaton()  */
  void
  GenerateOutputInformation() override;

  /** This function calls the actual region copier to do the mapping from
   * output image space to input image space.  It uses a
   * Function object used for dispatching to various routines to
   * copy an output region (start index and size) to an input region.
   * For most filters, this is a trivial copy because most filters
   * require the input dimension to match the output dimension.
   * However, some filters like itk::AverageOverDimensionImageFilter can
   * support output images of a lower dimension that the input.
   *
   * \sa ImageToImageFilter::CallCopyRegion() */
  void
  CallCopyOutputRegionToInputRegion(InputImageRegionType &        destRegion,
                                    const OutputImageRegionType & srcRegion) override;

  /** AverageOverDimensionImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData()  */
  void
  ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId) override;

  /** Overridden to check if there is no work to be done, before
   * calling superclass' implementation.
   */
  void
  GenerateData() override;

  unsigned int m_AveragingDimension;

  OutputImageRegionType m_OutputImageRegion;

private:
  AverageOverDimensionImageFilter(const Self &) = delete;
  void
  operator=(const Self &) = delete;

  DIRECTIONCOLLAPSESTRATEGY m_DirectionCollapseStrategy;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkAverageOverDimensionImageFilter.hxx"
#endif

#endif
