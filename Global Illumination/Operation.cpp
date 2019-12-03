#include "Operation.h"

Operation::~Operation()
{
}

void Operation::addCopyImage(Image* srcImage, Image* dstImage)
{
	m_copyImageOperations.push_back({ srcImage, dstImage });
}
