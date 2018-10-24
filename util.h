#pragma once
#include "SDK.hpp"
#include "global.h"

#undef max
#define M_PI 3.14159265358979323846f

namespace Util
{
    bool DataCompare(PBYTE pData, PBYTE bSig, char* szMask)
    {
        for (; *szMask; ++szMask, ++pData, ++bSig)
        {
            if (*szMask == 'x' && *pData != *bSig)
                return false;
        }
        return (*szMask) == 0;
    }

    PBYTE FindPattern(PBYTE dwAddress, DWORD dwSize, PBYTE pbSig, char* szMask, long offset)
    {
        size_t length = strlen(szMask);
        for (size_t i = NULL; i < dwSize - length; i++)
        {
            if (DataCompare(dwAddress + i, pbSig, szMask))
                return dwAddress + i + offset;
        }
        return nullptr;
    }

    namespace Vector
    {
        SDK::FVector Add(SDK::FVector point1, SDK::FVector point2)
        {
			SDK::FVector vector{ 0, 0, 0 };
            vector.X = point1.X + point2.X;
            vector.Y = point1.Y + point2.Y;
            vector.Z = point1.Z + point2.Z;
            return vector;
        }

		SDK::FVector Subtract(SDK::FVector point1, SDK::FVector point2)
        {
			SDK::FVector vector{ 0, 0, 0 };
            vector.X = point1.X - point2.X;
            vector.Y = point1.Y - point2.Y;
            vector.Z = point1.Z - point2.Z;
            return vector;
        }

		SDK::FVector Square(SDK::FVector vector)
        {
            return SDK::FVector{ vector.X * vector.X, vector.Y * vector.Y, vector.Z * vector.Z };
        }

		SDK::FVector Divide(SDK::FVector point1, float num)
		{
			SDK::FVector vector{ 0, 0, 0 };
			vector.X = point1.X / num;
			vector.Y = point1.Y / num;
			vector.Z = point1.Z / num;
			return vector;
		}
    }

    namespace Vector2D
    {
		SDK::FVector2D Subtract(SDK::FVector2D point1, SDK::FVector2D point2)
        {
			SDK::FVector2D vector{ 0, 0 };
            vector.X = point1.X - point2.X;
            vector.Y = point1.Y - point2.Y;
            return vector;
        }

		SDK::FVector2D Add(SDK::FVector2D point1, SDK::FVector2D point2)
		{
			SDK::FVector2D vector{ 0, 0 };
			vector.X = point1.X + point2.X;
			vector.Y = point1.Y + point2.Y;
			return vector;
		}

		SDK::FVector2D Divide(SDK::FVector2D point1, float num)
		{
			SDK::FVector2D vector{ 0, 0 };
			vector.X = point1.X / num;
			vector.Y = point1.Y / num;
			return vector;
		}
    }

    namespace Engine
    {
        DWORD_PTR boneAddress;
		SDK::FMatrix* GetBoneMatrix(SDK::USkeletalMeshComponent* mesh, SDK::FMatrix* result, int boneid)
        {
            return reinterpret_cast<SDK::FMatrix*(__fastcall*)(SDK::USkeletalMeshComponent*, SDK::FMatrix*, int)>(boneAddress)(mesh, result, boneid);
        }

        void GetBoneLocation(SDK::USkeletalMeshComponent* mesh, SDK::FVector* result, int boneid)
        {
			SDK::FMatrix vMatrix;
			SDK::FMatrix *vTempMatrix = GetBoneMatrix(mesh, &vMatrix, boneid);
            *result = vMatrix.WPlane;
        }
    }

    bool IsLocalPlayer(SDK::AActor* player)
    {
        if (Global::m_LocalPlayer->PlayerController->AcknowledgedPawn == nullptr)
            return true;
        return (static_cast<SDK::APawn*>(player) == Global::m_LocalPlayer->PlayerController->AcknowledgedPawn);
    }

    std::wstring DistanceToString(float distance)
    {
        float meters = distance * 0.01f;
        std::wstringstream ss;

        if (meters < 1000.f)
        {
            ss << std::fixed << std::setprecision(0) << meters << "m";
        }
		else
		{
			ss.precision(3);
			ss << std::fixed << std::setprecision(0) << (meters / 1000.f) << "km";
		}
        return ss.str();
    }

    float GetDistance(SDK::FVector x, SDK::FVector y)
    {
        auto z = Vector::Subtract(x, y);
        return sqrt(z.X * z.X + z.Y * z.Y + z.Z * z.Z);
    }

	float GetDistance2D(SDK::FVector2D point1, SDK::FVector2D point2)
	{
		SDK::FVector2D heading = Vector2D::Subtract(point2, point1);
		float distanceSquared;
		float distance;

		distanceSquared = heading.X * heading.X + heading.Y * heading.Y;
		distance = sqrt(distanceSquared);

		return distance;
	}
}