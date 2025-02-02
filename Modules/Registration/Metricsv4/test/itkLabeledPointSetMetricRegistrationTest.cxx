/*=========================================================================
 *
 *  Copyright NumFOCUS
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

#include "itkEuclideanDistancePointSetToPointSetMetricv4.h"
#include "itkExpectationBasedPointSetToPointSetMetricv4.h"
#include "itkJensenHavrdaCharvatTsallisPointSetToPointSetMetricv4.h"
#include "itkLabeledPointSetToPointSetMetricv4.h"
#include "itkGradientDescentOptimizerv4.h"
#include "itkTransform.h"
#include "itkAffineTransform.h"
#include "itkRegistrationParameterScalesFromPhysicalShift.h"
#include "itkCommand.h"

#include <fstream>

template <typename TFilter>
class itkLabeledPointSetMetricRegistrationTestCommandIterationUpdate : public itk::Command
{
public:
  using Self = itkLabeledPointSetMetricRegistrationTestCommandIterationUpdate;

  using Superclass = itk::Command;
  using Pointer = itk::SmartPointer<Self>;
  itkNewMacro(Self);

protected:
  itkLabeledPointSetMetricRegistrationTestCommandIterationUpdate() = default;

public:
  void
  Execute(itk::Object * caller, const itk::EventObject & event) override
  {
    Execute((const itk::Object *)caller, event);
  }

  void
  Execute(const itk::Object * object, const itk::EventObject & event) override
  {
    if (typeid(event) != typeid(itk::IterationEvent))
    {
      return;
    }
    const auto * optimizer = static_cast<const TFilter *>(object);

    std::cout << "It: " << optimizer->GetCurrentIteration() << " metric value: " << optimizer->GetCurrentMetricValue()
              << std::endl;
  }
};

template <class PointSetMetricType>
int
itkLabeledPointSetMetricRegistrationTestPerMetric(unsigned int numberOfIterations, PointSetMetricType * pointSetMetric)
{
  using PointSetType = typename PointSetMetricType::FixedPointSetType;
  using PointType = typename PointSetType::PointType;

  auto fixedPoints = PointSetType::New();
  fixedPoints->Initialize();

  auto movingPoints = PointSetType::New();
  movingPoints->Initialize();

  // two circles with a small offset
  PointType offset;
  for (unsigned int d = 0; d < PointSetType::PointDimension; ++d)
  {
    offset[d] = 2.0;
  }
  unsigned long count = 0;
  for (float theta = 0; theta < 2.0 * itk::Math::pi; theta += 0.1)
  {
    auto label = static_cast<unsigned int>(1.5 + count / 35);

    PointType fixedPoint;
    float     radius = 100.0;
    fixedPoint[0] = radius * std::cos(theta);
    fixedPoint[1] = radius * std::sin(theta);
    if (PointSetType::PointDimension > 2)
    {
      fixedPoint[2] = radius * std::sin(theta);
    }
    fixedPoints->SetPoint(count, fixedPoint);
    fixedPoints->SetPointData(count, label);

    PointType movingPoint;
    movingPoint[0] = fixedPoint[0] + offset[0];
    movingPoint[1] = fixedPoint[1] + offset[1];
    if (PointSetType::PointDimension > 2)
    {
      movingPoint[2] = fixedPoint[2] + offset[2];
    }
    movingPoints->SetPoint(count, movingPoint);
    movingPoints->SetPointData(count, label);

    count++;
  }

  using AffineTransformType = itk::AffineTransform<double, PointSetType::PointDimension>;
  auto transform = AffineTransformType::New();
  transform->SetIdentity();

  using LabeledPointSetMetricType = itk::LabeledPointSetToPointSetMetricv4<PointSetType>;
  auto metric = LabeledPointSetMetricType::New();
  metric->SetFixedPointSet(fixedPoints);
  metric->SetMovingPointSet(movingPoints);
  metric->SetMovingTransform(transform);
  metric->SetPointSetMetric(pointSetMetric);
  metric->Initialize();

  // scales estimator
  using RegistrationParameterScalesFromShiftType =
    itk::RegistrationParameterScalesFromPhysicalShift<LabeledPointSetMetricType>;
  typename RegistrationParameterScalesFromShiftType::Pointer shiftScaleEstimator =
    RegistrationParameterScalesFromShiftType::New();
  shiftScaleEstimator->SetMetric(metric);
  // needed with pointset metrics
  shiftScaleEstimator->SetVirtualDomainPointSet(metric->GetVirtualTransformedPointSet());

  // optimizer
  using OptimizerType = itk::GradientDescentOptimizerv4;
  auto optimizer = OptimizerType::New();
  optimizer->SetMetric(metric);
  optimizer->SetNumberOfIterations(numberOfIterations);
  optimizer->SetScalesEstimator(shiftScaleEstimator);
  optimizer->SetMaximumStepSizeInPhysicalUnits(3.0);

  using CommandType = itkLabeledPointSetMetricRegistrationTestCommandIterationUpdate<OptimizerType>;
  auto observer = CommandType::New();
  optimizer->AddObserver(itk::IterationEvent(), observer);

  optimizer->SetMinimumConvergenceValue(0.0);
  optimizer->SetConvergenceWindowSize(10);
  optimizer->StartOptimization();

  std::cout << "numberOfIterations: " << numberOfIterations << std::endl;
  std::cout << "Moving-source final value: " << optimizer->GetCurrentMetricValue() << std::endl;
  std::cout << "Moving-source final position: " << optimizer->GetCurrentPosition() << std::endl;
  std::cout << "Optimizer scales: " << optimizer->GetScales() << std::endl;
  std::cout << "Optimizer learning rate: " << optimizer->GetLearningRate() << std::endl;

  // applying the resultant transform to moving points and verify result
  std::cout << "Fixed\tMoving\tMovingTransformed\tFixedTransformed\tDiff" << std::endl;
  bool                                                      passed = true;
  typename PointType::ValueType                             tolerance = 1e-2;
  typename AffineTransformType::InverseTransformBasePointer movingInverse =
    metric->GetMovingTransform()->GetInverseTransform();
  typename AffineTransformType::InverseTransformBasePointer fixedInverse =
    metric->GetFixedTransform()->GetInverseTransform();
  for (unsigned int n = 0; n < metric->GetNumberOfComponents(); ++n)
  {
    // compare the points in virtual domain
    PointType transformedMovingPoint = movingInverse->TransformPoint(movingPoints->GetPoint(n));
    PointType transformedFixedPoint = fixedInverse->TransformPoint(fixedPoints->GetPoint(n));
    PointType difference;
    difference[0] = transformedMovingPoint[0] - transformedFixedPoint[0];
    difference[1] = transformedMovingPoint[1] - transformedFixedPoint[1];
    std::cout << fixedPoints->GetPoint(n) << "\t" << movingPoints->GetPoint(n) << "\t" << transformedMovingPoint << "\t"
              << transformedFixedPoint << "\t" << difference << std::endl;
    if (itk::Math::abs(difference[0]) > tolerance || itk::Math::abs(difference[1]) > tolerance)
    {
      passed = false;
    }
  }
  if (!passed)
  {
    std::cerr << "Results do not match truth within tolerance." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int
itkLabeledPointSetMetricRegistrationTest(int argc, char * argv[])
{
  unsigned int numberOfIterations = 10;
  if (argc > 1)
  {
    numberOfIterations = std::stoi(argv[1]);
  }

  constexpr unsigned int Dimension = 2;
  using PointSetType = itk::PointSet<unsigned int, Dimension>;

  int allSuccess = EXIT_SUCCESS;

  {
    using PointSetMetricType = itk::EuclideanDistancePointSetToPointSetMetricv4<PointSetType>;
    auto metric = PointSetMetricType::New();
    int  success =
      itkLabeledPointSetMetricRegistrationTestPerMetric<PointSetMetricType>(numberOfIterations, metric.GetPointer());

    allSuccess *= success;
  }

  {
    using PointSetMetricType = itk::ExpectationBasedPointSetToPointSetMetricv4<PointSetType>;
    auto metric = PointSetMetricType::New();
    metric->SetPointSetSigma(2.0);
    metric->SetEvaluationKNeighborhood(3);
    int success =
      itkLabeledPointSetMetricRegistrationTestPerMetric<PointSetMetricType>(numberOfIterations, metric.GetPointer());

    allSuccess *= success;
  }

  {
    using PointSetMetricType = itk::JensenHavrdaCharvatTsallisPointSetToPointSetMetricv4<PointSetType>;
    auto metric = PointSetMetricType::New();
    metric->SetPointSetSigma(1.0);
    metric->SetKernelSigma(10.0);
    metric->SetUseAnisotropicCovariances(false);
    metric->SetCovarianceKNeighborhood(5);
    metric->SetEvaluationKNeighborhood(10);
    metric->SetAlpha(1.1);
    int success =
      itkLabeledPointSetMetricRegistrationTestPerMetric<PointSetMetricType>(numberOfIterations, metric.GetPointer());

    allSuccess *= success;
  }

  return allSuccess;
}
