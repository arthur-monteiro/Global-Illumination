#pragma once

#include "RenderPass.h"

class IrradianceProbes
{
public:
	IrradianceProbes() = default;
	~IrradianceProbes() = default;

private:
	RenderPass m_renderPass;
};