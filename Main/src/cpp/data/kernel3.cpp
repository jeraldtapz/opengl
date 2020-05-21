#include "data/kernel3.h"

kernel3::kernel3(const float kernel[9], const float offset)
{
	for (unsigned int i = 0; i < 9; i++)
	{
		this->kernel[i] = kernel[i];
		this->offset[i] *= offset;
	}
}
