//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OpalinusPermeabilityTensor.h"
#include "Function.h"
#include "CartesianLocalCoordinateSystem.h"

registerMooseObject(MOOSEAPPNAME, OpalinusPermeabilityTensor);

InputParameters
OpalinusPermeabilityTensor::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription(
      "This Material calculates the permeability tensor assuming it is constant");

  params.addRequiredParam<UserObjectName>(
      "local_coordinate_system", "The UserObject that defines the local coordinate system. ");

  params.addRequiredParam<Real>(
      "permeability1",
      "Intrinsic permeability in direction of the 'e1' axis of the coordinate system given by "
      "'local_coordinate_system' in unit LE² (e.g. m²). "
      "If the plane e1-e2 of the local coordinate system is representing the bedding, "
      "than this permeability controls flow parallel to this bedding "
      "(in this case for Opalinus, this corresponds to permeability of the P-samples)");
  params.addRequiredParam<Real>(
      "permeability2",
      "Intrinsic permeability in direction of the 'e2' axis of the coordinate system given by "
      "'local_coordinate_system' in unit LE² (e.g. m²). "
      "If the plane e1-e2 of the local coordinate system is representing the bedding, "
      "than this permeability controls flow parallel to this bedding "
      "(in this case for Opalinus, this corresponds to permeability of the P-samples)");
  params.addRequiredParam<Real>(
      "permeability3",
      "Intrinsic permeability in direction of the 'e3' axis of the coordinate system given by "
      "'local_coordinate_system' in unit LE² (e.g. m²). "
      "If the plane e1-e2 of the local coordinate system is representing the bedding, "
      "than this permeability controls flow normal to this bedding "
      "(in this case for Opalinus, this corresponds to permeability of the S-samples)");

  params.addParam<MooseFunctorName>(
      "prefactor_functor",
      "Optional functor to use as a scalar prefactor on the permeability tensor.");

  params.addParam<MaterialPropertyName>(
      "prefactor_mat_prop",
      "Optional material property to use as a scalar prefactor on the permeability tensor.");

  return params;
}

OpalinusPermeabilityTensor::OpalinusPermeabilityTensor(const InputParameters & parameters)
  : Material(parameters),
    _localCoordinateSystem(
        getUserObject<CartesianLocalCoordinateSystem>("local_coordinate_system")),
    _permeability1(parameters.get<Real>("permeability1")),
    _permeability2(parameters.get<Real>("permeability2")),
    _permeability3(parameters.get<Real>("permeability3")),
    _prefactor_functor(isParamValid("prefactor_functor")
                            ? &getFunctor<Real>("prefactor_functor")
                            : nullptr),
    _prefactor_matprop(isParamValid("prefactor_mat_prop")
                            ? &getMaterialProperty<Real>("prefactor_mat_prop")
                            : nullptr),
    _permeability_qp(declareProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _dpermeability_qp_dvar(
        declareProperty<std::vector<RealTensorValue>>("dPorousFlow_permeability_qp_dvar")),
    _dpermeability_qp_dgradvar(declareProperty<std::vector<std::vector<RealTensorValue>>>(
        "dPorousFlow_permeability_qp_dgradvar"))
{
  // create permeability tensor in local coordinates (unrotated)
  // and then rotate it to global coordinates
  _input_permeability =
      RankTwoTensor(_permeability1, 0, 0, 0, _permeability2, 0, 0, 0, _permeability3);
  _localCoordinateSystem.rotateLocalToGlobal(&_input_permeability);
}

void
OpalinusPermeabilityTensor::computeQpProperties()
{
  if (_prefactor_functor || _prefactor_matprop)
  {
    Real f;

    if (_prefactor_functor)
    {
      const Moose::ElemQpArg elem_arg{_current_elem, _qp, _qrule, _q_point[_qp]};
      const auto state = determineState();
      f = (*_prefactor_functor)(elem_arg, state);
    }
    else
      f = 1.0;

    if (_prefactor_matprop && f != 0.0)
    {
      f *= (*_prefactor_matprop)[_qp];
    }

    _permeability_qp[_qp] = _input_permeability * f;
  }
  else
    _permeability_qp[_qp] = _input_permeability;
}
