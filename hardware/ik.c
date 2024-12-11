/* File: inversekinematics.c
 * This program computes the required angles of the two legs to reach end effector using inverse
 * kinematics.
 * Contribution: Calvin Xu mathlib implementation used for math functions
 */

#include "math_float.h"
#include "ik.h"

int calculate_angles(float x, float y, float *theta1, float *theta2) {
  float r = sqrt(x * x + y * y);

  float cos_theta2 = (r * r - L1 * L1 - L2 * L2) / (2 * L1 * L2);
  float sin_theta2 = sqrt(1 - cos_theta2 * cos_theta2);

  *theta2 = atan2(sin_theta2, cos_theta2) * (180 / PI);

  float phi = atan2(y, x);
  float beta = atan2(L2 * sin_theta2, L1 + L2 * cos_theta2);
  *theta1 = (phi - beta) * (180 / PI);

  return 0;
}
