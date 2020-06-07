/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ECamera.h: class representing a camera
/*=============================================================================*/
#ifndef ELITE_CAMERA
#define	ELITE_CAMERA

#include "EMath.h"

namespace Elite
{
	namespace Rasterizer
	{
		struct Camera
		{
			Camera(const FPoint3& position, float fovAngle, float nearPlane, float farPlane)
				:Position(position), FOVAngle(fovAngle), NearPlane(nearPlane), FarPlane(farPlane)
			{}
			FPoint3 Position = { 0.f, 0.f, 0.f };
			FVector3 m_U = { 1.f, 0.f, 0.f }; //ONB x-axis
			FVector3 m_V = { 0.f, 1.f, 0.f }; //ONB y-axis
			FVector3 m_W = { 0.f, 0.f, 1.f }; //ONB z-axis
			float FOVAngle = 60.f;
			float NearPlane = 1.f;
			float FarPlane = 1000.f;

			void MoveForward(float d)
			{
				Position += m_W * d;
			}

			void MoveRight(float d)
			{
				Position += m_U * d;
			}

			void MoveUp(float d)
			{
				Position += FVector3(0.f, 1.f, 0.f) * d;
			}

			void Pitch(float angle)
			{
				FMatrix3 rotation = MakeRotationX(angle * static_cast<float>(E_TO_RADIANS));
				m_W = Inverse(Transpose(rotation)) * m_W;
			}

			void Yaw(float angle)
			{
				FMatrix3 rotation = MakeRotationY(angle * static_cast<float>(E_TO_RADIANS));
				m_W = Inverse(Transpose(rotation)) * m_W;
			}

			FMatrix4 LookAt(const FVector3& up = FVector3(0.f, 1.f, 0.f))
			{
				//Compute the OrthoNormal Basis (U = local X, V = local Y, W = local Z)
				Normalize(m_W);
				m_U = Cross(up, m_W);
				Normalize(m_U);
				m_V = Cross(m_W, m_U);
				Normalize(m_V);

				FMatrix4 cameraToWorld = FMatrix4(
					FVector4(m_U), FVector4(m_V), FVector4(m_W), FVector4(FVector3(Position), 1.f));
				return cameraToWorld;
			}
		};
	}
}
#endif