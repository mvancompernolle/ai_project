#include "MathHelper.h"

double RoundNearest(double value)
{
  double Result = floor(value);

  if ((value - Result) < 0.5)
  {
    return Result;
  }

  Result = ceil(value);
  return Result;
}