//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeRotatedElasticityTensorBase.h"
#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "RankFourTensor.h"
#include "SymmetricRankFourTensor.h"
#include "GuaranteeProvider.h"

/**
 * OpalinusElasticityTensor defines an elasticity tensor material object with a given base name.
 */

class OpalinusElasticityTensor: public Material
{
public:
  static InputParameters validParams();

  OpalinusElasticityTensor(const InputParameters & parameters);

protected:
   void computeQpProperties() override;

  /// Individual material information
  const std::string _base_name;
  std::string _elasticity_tensor_name;
  MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const Function * const _prefactor_function;
  RankFourTensor _Cijkl;
  RealVectorValue _geological_angles;
  RankTwoTensor _geological_rotation;
  const Real _Ep;
  const Real _Es;
  const Real _nu_p;
  const Real _nu_s;
  const Real _G_s;
  MaterialProperty<std::vector<Real>> & _first_local_axis;
  MaterialProperty<std::vector<Real>> & _second_local_axis;
  MaterialProperty<std::vector<Real>> & _normal_local_axis;
 
};

