

#include "GlobalShader.h"
#include "RenderGraphFwd.h"
#include "ShaderCompilerCore.h"
#include "ShaderParameterStruct.h"
#include "SceneTextureParameters.h"

class FArmASRConvertVelocity : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FArmASRConvertVelocity);
	SHADER_USE_PARAMETER_STRUCT(FArmASRConvertVelocity, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		RDG_TEXTURE_ACCESS(DepthTexture, ERHIAccess::SRVGraphics)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, InputDepth)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, InputVelocity)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return FArmASRGlobalShader::ShouldCompilePermutation(Parameters);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
	}
};
