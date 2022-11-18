#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

//structs
struct EFX_Header {
	char magic[4];
	uint32_t ukn;
	uint32_t effectCount;
	uint32_t nameBufferSize;
	uint32_t linkedEfxCount;
	uint32_t msk4Count;
	uint32_t modifierCount;
	uint32_t cndBlockCount;
	uint32_t cndBuffSize;
};

//enums
enum SegmentType : uint32_t {
	ItemType_Unknown = 0x00,
	ItemType_Spawn = 0x01,
	ItemType_SpawnExpression = 0x02,
	ItemType_Transform2D = 0x03,
	ItemType_Transform2DModifier = 0x04,
	ItemType_Transform2DClip = 0x05,
	ItemType_Transform2DExpression = 0x06,
	ItemType_Transform3D = 0x07,
	ItemType_Transform3DModifier = 0x08,
	ItemType_Transform3DClip = 0x09,
	ItemType_Transform3DExpression = 0x0A,
	ItemType_ParentOptions = 0x0B,
	ItemType_FixRandomGenerator = 0x0C,
	ItemType_TypeBillboard2D = 0x0D,
	ItemType_TypeBillboard2DExpression = 0x0E,
	ItemType_TypeBillboard3D = 0x0F,
	ItemType_TypeBillboard3DExpression = 0x10,
	ItemType_TypeMesh = 0x11,
	ItemType_TypeMeshClip = 0x12,
	ItemType_TypeMeshExpression = 0x13,
	ItemType_TypeRibbonFollow = 0x14,
	ItemType_TypeRibbonLength = 0x15,
	ItemType_TypeRibbonChain = 0x16,
	ItemType_TypeRibbonFixEnd = 0x17,
	ItemType_TypeRibbonLightweight = 0x18,
	ItemType_TypeRibbonFollowExpression = 0x19,
	ItemType_TypeRibbonLengthExpression = 0x1A,
	ItemType_TypeRibbonChainExpression = 0x1B,
	ItemType_TypeRibbonFixEndExpression = 0x1C,
	ItemType_TypePolygon = 0x1D,
	ItemType_TypePolygonClip = 0x1E,
	ItemType_TypePolygonExpression = 0x1F,
	ItemType_TypeRibbonTrail = 0x20,
	ItemType_TypePolygonTrail = 0x21,
	ItemType_TypeNoDraw = 0x22,
	ItemType_TypeNoDrawExpression = 0x23,
	ItemType_Velocity2D = 0x24,
	ItemType_Velocity2DExpression = 0x25,
	ItemType_Velocity3D = 0x26,
	ItemType_Velocity3DExpression = 0x27,
	ItemType_RotateAnim = 0x28,
	ItemType_RotateAnimExpression = 0x29,
	ItemType_ScaleAnim = 0x2A,
	ItemType_ScaleAnimExpression = 0x2B,
	ItemType_Life = 0x2C,
	ItemType_LifeExpression = 0x2D,
	ItemType_UVSequence = 0x2E,
	ItemType_UVSequenceExpression = 0x2F,
	ItemType_UVScroll = 0x30,
	ItemType_TextureUnit = 0x31,
	ItemType_EmitterShape2D = 0x32,
	ItemType_EmitterShape2DExpression = 0x33,
	ItemType_EmitterShape3D = 0x34,
	ItemType_EmitterShape3DExpression = 0x35,
	ItemType_AlphaCorrection = 0x36,
	ItemType_TypeStrainRibbon = 0x37,
	ItemType_TypeStrainRibbonExpression = 0x38,
	ItemType_TypeLightning3D = 0x39,
	ItemType_ShaderSettings = 0x3A,
	ItemType_ShaderSettingsExpression = 0x3B,
	ItemType_Distortion = 0x3C,
	ItemType_RenderTarget = 0x3D,
	ItemType_PtLife = 0x3E,
	ItemType_PtBehavior = 0x3F,
	ItemType_PtBehaviorClip = 0x40,
	ItemType_PlayEfx = 0x41,
	ItemType_FadeByAngle = 0x42,
	ItemType_FadeByAngleExpression = 0x43,
	ItemType_FadeByEmitterAngle = 0x44,
	ItemType_FadeByDepth = 0x45,
	ItemType_FadeByDepthExpression = 0x46,
	ItemType_FadeByOcclusion = 0x47,
	ItemType_FadeByOcclusionExpression = 0x48,
	ItemType_FakeDoF = 0x49,
	ItemType_LuminanceBleed = 0x4A,
	ItemType_TypeNodeBillboard = 0x4B,
	ItemType_TypeNodeBillboardExpression = 0x4C,
	ItemType_UnitCulling = 0x4D,
	ItemType_FluidEmitter2D = 0x4E,
	ItemType_FluidSimulator2D = 0x4F,
	ItemType_PlayEmitter = 0x50,
	ItemType_PtTransform3D = 0x51,
	ItemType_PtTransform3DClip = 0x52,
	ItemType_PtTransform2D = 0x53,
	ItemType_PtTransform2DClip = 0x54,
	ItemType_PtVelocity3D = 0x55,
	ItemType_PtVelocity3DClip = 0x56,
	ItemType_PtVelocity2D = 0x57,
	ItemType_PtVelocity2DClip = 0x58,
	ItemType_PtColliderAction = 0x59,
	ItemType_PtCollision = 0x5A,
	ItemType_PtColor = 0x5B,
	ItemType_PtColorClip = 0x5C,
	ItemType_PtUvSequence = 0x5D,
	ItemType_PtUvSequenceClip = 0x5E,
	ItemType_MeshEmitter = 0x5F,
	ItemType_MeshEmitterClip = 0x60,
	ItemType_MeshEmitterExpression = 0x61,
	ItemType_VectorFieldParameter = 0x62,
	ItemType_VectorFieldParameterClip = 0x63,
	ItemType_VectorFieldParameterExpression = 0x64,
	ItemType_DepthOperator = 0x65,
	ItemType_ShapeOperator = 0x66,
	ItemType_ShapeOperatorExpression = 0x67,
	ItemType_WindInfluence3D = 0x68,
	ItemType_TypeGpuBillboard = 0x69,
	ItemType_TypeGpuBillboardExpression = 0x6A,
	ItemType_TypeGpuRibbonFollow = 0x6B,
	ItemType_EmitterPriority = 0x6C,
	ItemType_DrawOverlay = 0x6D,
	ItemType_VectorField = 0x6E,
	ItemType_VolumeField = 0x6F,
	ItemType_AngularVelocity3D = 0x70,
	ItemType_PtAngularVelocity3D = 0x71,
	ItemType_PtAngularVelocity3DExpression = 0x72,
	ItemType_AngularVelocity2D = 0x73,
	ItemType_PtAngularVelocity2D = 0x74,
	ItemType_PtAngularVelocity2DExpression = 0x75,
	ItemType_IgnorePlayerColor = 0x76,
	ItemType_ProceduralDistortion = 0x77,
	ItemType_ProceduralDistortionClip = 0x78,
	ItemType_ItemNum = 0x79,
};

struct Segment {
	uint32_t id;
	std::string segName;
	uint32_t size;
};

struct Effect {
	uint32_t id;
	std::string name;
	std::unordered_map<uint32_t, Segment> segments;
};



