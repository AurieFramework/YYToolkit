#include "PI.hpp"
using namespace Aurie;

namespace YYTK
{
	AurieStatus YYTKPrivateInterfaceImpl::Create()
	{
		return AURIE_SUCCESS;
	}

	void YYTKPrivateInterfaceImpl::Destroy()
	{

	}

	void YYTKPrivateInterfaceImpl::QueryVersion(
		OUT short& Major,
		OUT short& Minor,
		OUT short& Patch
	)
	{
		Major = YYTK_MAJOR;
		Minor = YYTK_MINOR;
		Patch = YYTK_PATCH;
	}
}
