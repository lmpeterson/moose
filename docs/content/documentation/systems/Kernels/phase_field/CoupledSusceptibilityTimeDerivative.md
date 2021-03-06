# CoupledSusceptibilityTimeDerivative
!description /Kernels/CoupledSusceptibilityTimeDerivative

Implements

$$
F(u,v,a,b,\dots)\cdot\frac{\partial v}{\partial t},
$$

where $F$ (`f_name`) is a [FunctionMaterial](/FunctionMaterials.md) providing derivatives
(for example defined using the [DerivativeParsedMaterial](/DerivativeParsedMaterial.md)),
$u$ is the variable the kernel is acting on, $v$ (`v`) is the coupled variable the time
derivative is taken of, and $a, b, \dots$ (`args`) are further arguments of the susceptibility
function $F$ which should contribute to off-diagonal Jacobian entries.

See also [CoupledTimeDerivative](/CoupledTimeDerivative.md).s

!parameters /Kernels/CoupledSusceptibilityTimeDerivative

!inputfiles /Kernels/CoupledSusceptibilityTimeDerivative

!childobjects /Kernels/CoupledSusceptibilityTimeDerivative
