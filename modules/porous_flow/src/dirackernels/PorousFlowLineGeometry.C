/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowLineGeometry.h"
#include "libmesh/utility.h"

template <>
InputParameters
validParams<PorousFlowLineGeometry>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<std::string>(
      "point_file",
      "The file containing the coordinates of the points and their weightings that approximate the "
      "line sink.  The physical meaning of the weightings depend on the scenario, eg, they may be "
      "borehole radii.  Each line in the file must contain a space-separated weight and "
      "coordinate, viz r x y z.  For boreholes, the last point in the file is defined as the "
      "borehole bottom, where the borehole pressure is bottom_pressure.  If your file contains "
      "just one point, you must also specify the line_length and line_direction parameters.  Note "
      "that you will get segementation faults if your points do not lie within your mesh!");
  params.addRangeCheckedParam<Real>(
      "line_length",
      0.0,
      "line_length>=0",
      "Line length.  Note this is only used if there is only one point in the point_file.");
  params.addParam<RealVectorValue>(
      "line_direction",
      RealVectorValue(0.0, 0.0, 1.0),
      "Line direction.  Note this is only used if there is only one point in the point_file.");
  params.addClassDescription("Approximates a polyline sink in the mesh using a number of Dirac "
                             "point sinks with given weightings that are read from a file");
  return params;
}

PorousFlowLineGeometry::PorousFlowLineGeometry(const InputParameters & parameters)
  : DiracKernel(parameters),
    _line_length(getParam<Real>("line_length")),
    _line_direction(getParam<RealVectorValue>("line_direction")),
    _point_file(getParam<std::string>("point_file"))
{
  statefulPropertiesAllowed(true);

  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("PorousFlowLineGeometry: Error opening file " + _point_file);

  // construct the arrays of weight, x, y and z
  std::vector<Real> scratch;
  while (parseNextLineReals(file, scratch))
  {
    if (scratch.size() >= 2)
    {
      _rs.push_back(scratch[0]);
      _xs.push_back(scratch[1]);
      if (scratch.size() >= 3)
        _ys.push_back(scratch[2]);
      else
        _ys.push_back(0.0);
      if (scratch.size() >= 4)
        _zs.push_back(scratch[3]);
      else
        _zs.push_back(0.0);
    }
  }

  file.close();

  const int num_pts = _zs.size();
  _bottom_point(0) = _xs[num_pts - 1];
  _bottom_point(1) = _ys[num_pts - 1];
  _bottom_point(2) = _zs[num_pts - 1];

  // construct the line-segment lengths between each point
  _half_seg_len.resize(std::max(num_pts - 1, 1));
  for (unsigned int i = 0; i + 1 < _xs.size(); ++i)
  {
    _half_seg_len[i] = 0.5 * std::sqrt(Utility::pow<2>(_xs[i + 1] - _xs[i]) +
                                       Utility::pow<2>(_ys[i + 1] - _ys[i]) +
                                       Utility::pow<2>(_zs[i + 1] - _zs[i]));
    if (_half_seg_len[i] == 0)
      mooseError("PorousFlowLineGeometry: zero-segment length detected at (x,y,z) = ",
                 _xs[i],
                 " ",
                 _ys[i],
                 " ",
                 _zs[i],
                 "\n");
  }
  if (num_pts == 1)
    _half_seg_len[0] = _line_length;
}

bool
PorousFlowLineGeometry::parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec)
// reads a space-separated line of floats from ifs and puts in myvec
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs, line))
  {
    gotline = true;

    // Harvest floats separated by whitespace
    std::istringstream iss(line);
    Real f;
    while (iss >> f)
    {
      myvec.push_back(f);
    }
  }
  return gotline;
}

void
PorousFlowLineGeometry::addPoints()
{
  // Add point using the unique ID "i", let the DiracKernel take
  // care of the caching.  This should be fast after the first call,
  // as long as the points don't move around.
  for (unsigned int i = 0; i < _zs.size(); i++)
    addPoint(Point(_xs[i], _ys[i], _zs[i]), i);
}
