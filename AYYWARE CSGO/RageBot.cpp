#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include <iostream>
#include "UTIL Functions.h"

#define TICK_INTERVAL			( Interfaces::Globals->interval_per_tick )
#define TIME_TO_TICKS( dt )	( ( int )( 0.5f + ( float )( dt ) / Interfaces::Globals->interval_per_tick ) )
#define TICKS_TO_TIME( t ) ( Interfaces::Globals->interval_per_tick *( t ) )

void CRageBot::Init()
{

	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}

void AngleVectors3(const Vector &angles, Vector *forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward->x = cp*cy;
	forward->y = cp*sy;
	forward->z = -sp;
}

void FakeWalk(CUserCmd * pCmd, bool & bSendPacket)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(VK_SHIFT))
	{

		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;



			pCmd->tick_count += 10.95; // 10.95
			pCmd->command_number += 5.07 + pCmd->tick_count % 2 ? 0 : 1; // 5
	
			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 10; // 10
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}

void CRageBot::Draw()
{

}

bool IsAbleToShoot(IClientEntity* pLocal)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}

float hitchance(IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Menu::Window.RageBotTab.AccuracyHitchance.GetValue() > 1)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

bool CanOpenFire() 
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;

	C_BaseCombatWeapon* entwep = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());

	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}

void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{

	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!pLocalEntity || !Menu::Window.RageBotTab.Active.GetState() || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;

		C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;

		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE && pCmd->buttons & IN_ATTACK && CanOpenFire() && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);

			}
			ChokedPackets = -1;
		}
	}

	if (Menu::Window.RageBotTab.AimbotEnable.GetState())
		DoAimbot(pCmd, bSendPacket);

	if (Menu::Window.RageBotTab.AccuracyRecoil.GetState())
		DoNoRecoil(pCmd);

	if (Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 25;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

	LastAngle = pCmd->viewangles;
}

Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, 10), final);
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	final = tr.endpos;
	return final;
}

void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pTarget = nullptr;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector Start = pLocal->GetViewOffset() + pLocal->GetOrigin();
	bool FindNewTarget = true;
	CSWeaponInfo* weapInfo = ((C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle()))->GetCSWpnData();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (Menu::Window.RageBotTab.AutoRevolver.GetState())
		if (GameUtils::IsRevolver(pWeapon))
		{
			static int delay = 0;
			delay++;
			if (delay <= 15)pCmd->buttons |= IN_ATTACK;
			else delay = 0;
		}
	if (pWeapon)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon)) return;
	}
	else return;
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset(), View;
				Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Menu::Window.RageBotTab.AimbotFov.GetValue())	FindNewTarget = false;
			}
		}
	}


	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		switch (Menu::Window.RageBotTab.TargetSelection.GetIndex())
		{
		case 0:TargetID = GetTargetCrosshair(); break;
		case 1:TargetID = GetTargetDistance(); break;
		case 2:TargetID = GetTargetHealth(); break;
		case 3:TargetID = GetTargetThreat(pCmd); break;
		case 4:TargetID = GetTargetNextShot(); break;
		}
		if (TargetID >= 0) pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		else
		{


			pTarget = nullptr;
			HitBox = -1;
		}
	} 
	Globals::Target = pTarget;
	Globals::TargetID = TargetID;
	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		if (!CanOpenFire()) return;

		if (Menu::Window.RageBotTab.AimbotKeyPress.GetState())
		{


			int Key = Menu::Window.RageBotTab.AimbotKeyBind.GetKey();
			if (Key >= 0 && !GUI.GetKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}
		float pointscale = Menu::Window.RageBotTab.TargetPointscale.GetValue() - 5.f; 
		Vector Point;
		Vector AimPoint = GetHitboxPosition(pTarget, HitBox) + Vector(0, 0, pointscale);
		if (Menu::Window.RageBotTab.TargetMultipoint.GetState()) Point = BestPoint(pTarget, AimPoint);
		else Point = AimPoint;

		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Menu::Window.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((Menu::Window.RageBotTab.AccuracyHitchance.GetValue() * 1.5 <= hitchance(pLocal, pWeapon)) || Menu::Window.RageBotTab.AccuracyHitchance.GetValue() == 0 || *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64)
			{
				if (AimAtPoint(pLocal, Point, pCmd, bSendPacket))
					if (Menu::Window.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))pCmd->buttons |= IN_ATTACK;
					else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;
			}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK)
			Globals::Shots += 1;
		if (Menu::Window.RageBotTab.AccuracyBacktracking.GetState()) pCmd->tick_count += TICKS_TO_TIME(Interfaces::Globals->interval_per_tick);
	}

}

bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != hackManager.pLocal()->GetIndex())
	{

		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != hackManager.pLocal()->GetTeamNum() || Menu::Window.RageBotTab.TargetFriendlyFire.GetState())
			{
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}

	}

	return false;
}

float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);

	return (acos(DotProduct) * (MaxDegrees / PI));
}

int CRageBot::GetTargetCrosshair()
{

	int target = -1;
	float minFoV = Menu::Window.RageBotTab.AimbotFov.GetValue();

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetDistance()
{

	int target = -1;
	int minDist = 99999;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{

			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}

		}
	}

	return target;
}

int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);

	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{

		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}

		}
	}

	return target;
}

float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}

float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}

int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;

	IClientEntity* pLocal = hackManager.pLocal();

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{

				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}

int CRageBot::GetTargetHealth()
{

	int target = -1;
	int minHealth = 101;

	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);


	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Menu::Window.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}

	}

	return target;
}

int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;

#pragma region GetHitboxesToScan
	int huso = (pEntity->GetHealth());
	int health = Menu::Window.RageBotTab.BaimIfUnderXHealth.GetValue();
	bool AWall = Menu::Window.RageBotTab.AccuracyAutoWall.GetState();
	bool Multipoint = Menu::Window.RageBotTab.TargetMultipoint.GetState();
	int TargetHitbox = Menu::Window.RageBotTab.TargetHitbox.GetIndex();
	static bool enemyHP = false;
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());


	int AimbotBaimOnKey = Menu::Window.RageBotTab.AimbotBaimOnKey.GetKey();
	if (AimbotBaimOnKey >= 0 && GUI.GetKeyState(AimbotBaimOnKey))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach); // 4
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh); // 9
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh); // 8
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot); // 13
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot); // 12
	}


	if (huso < health)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
	}
	else if (Menu::Window.RageBotTab.AWPAtBody.GetState() && GameUtils::AWP(pWeapon))
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
		HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
		HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
	}
	else if (TargetHitbox)
	{
		switch (Menu::Window.RageBotTab.TargetHitbox.GetIndex())
		{
		case 1:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			break;
		case 2:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
			break;
		case 3:
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
			break;
		case 4:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			break;
		case 5:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			break;
		case 6:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Chest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			break;
		case 7:
			HitBoxesToScan.push_back((int)CSGOHitboxID::Head);
			HitBoxesToScan.push_back((int)CSGOHitboxID::NeckLower);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftFoot);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LowerChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Stomach);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Pelvis);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftLowerArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightLowerArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightUpperArm);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightThigh);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightShin);
			HitBoxesToScan.push_back((int)CSGOHitboxID::Neck);
			HitBoxesToScan.push_back((int)CSGOHitboxID::UpperChest);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RightHand);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LeftHand);
			break;
		}
	}
#pragma endregion Get the list of shit to scan
	for (auto HitBoxID : HitBoxesToScan)
	{
		if (AWall)
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float Damage = 0.f;
			Color c = Color(255, 255, 255, 255);
			if (CanHit(Point, &Damage))
			{
				c = Color(0, 255, 0, 255);
				if (Damage >= Menu::Window.RageBotTab.AccuracyMinimumDamage.GetValue())
				{
					return HitBoxID;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
				return HitBoxID;
		}
	}

	return -1;
}

void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{

	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}

}

void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (!pLocal || !pWeapon)
		return;

	Vector eye_position = pLocal->GetEyePosition();

	float best_dist = pWeapon->GetCSWpnData()->m_flRange;

	IClientEntity* target = nullptr;

	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;

			Vector target_position = target->GetEyePosition();

			float temp_dist = eye_position.DistTo(target_position);

			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}

	}
}

bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}

	IsLocked = true;

	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle; 

	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);

	if (fovLeft > 25.0f && Menu::Window.RageBotTab.AimbotAimStep.GetState())
	{

		Vector AddAngs = angles - LastAimstepAngle;
		Normalize(AddAngs, AddAngs);
		AddAngs *= 25;
		LastAimstepAngle += AddAngs;
		GameUtils::NormaliseViewAngle(LastAimstepAngle);
		angles = LastAimstepAngle;
	}
	else
	{
		ReturnValue = true;
	}

	if (Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;

	}

	if (!Menu::Window.RageBotTab.AimbotSilentAim.GetState())
	{

		Interfaces::Engine->SetViewAngles(angles);
	}

	return ReturnValue;
}

namespace AntiAims 
{
	static float pDance = 0.0f;

	void UpsideDown(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 130089.0f;
	}

	void UpsideDown2(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = -250078.0f;
	}
	void UpsideDownJitter(CUserCmd *pCmd)
	{
		pDance += 45.0f;
		if (pDance > 100)
			pDance = 0.0f;
		else if (pDance > 75.f)
			pCmd->viewangles.x = -130089.0f;
		else if (pDance < 75.f)
			pCmd->viewangles.x = 130089.0f;
	}
	void JitterPitch(CUserCmd *pCmd)
	{
		static bool up = true;
		if (up)
		{
			pCmd->viewangles.x = 45;
			up = !up;
		}
		else
		{
			pCmd->viewangles.x = 89;
			up = !up;
		}
	}

	void FakePitch(CUserCmd *pCmd, bool &bSendPacket)
	{	
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.x = 89;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.x = 51;
			ChokedPackets = -1;
		}
	}

	void StaticJitter(CUserCmd *pCmd)
	{
		static bool down = true;
		if (down)
		{
			pCmd->viewangles.x = 179.0f;
			down = !down;
		}
		else
		{
			pCmd->viewangles.x = 89.0f;
			down = !down;
		}
	}

	// Yaws

	void FakeSideLBY(CUserCmd *pCmd, bool &bSendPacket)
	{
		int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); ++i;
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		static bool isMoving;
		float PlayerIsMoving = abs(pLocal->GetVelocity().Length());
		if (PlayerIsMoving > 0.1) isMoving = true;
		else if (PlayerIsMoving <= 0.1) isMoving = false;

		int flip = (int)floorf(Interfaces::Globals->curtime / 1.1) % 2;
		static bool bFlipYaw;
		float flInterval = Interfaces::Globals->interval_per_tick;
		float flTickcount = pCmd->tick_count;
		float flTime = flInterval * flTickcount;
		if (std::fmod(flTime, 1) == 0.f)
			bFlipYaw = !bFlipYaw;

		if (PlayerIsMoving <= 0.1)
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += 179.f;
			}
			else
			{
				if (flip)
				{
					pCmd->viewangles.y += bFlipYaw ? 86 : -89.f;

				}
				else
				{
					pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + bFlipYaw ? 89.f : -90.f;
				}
			}
		}
		else if (PlayerIsMoving > 0.1)
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y += 177.f;
			}
			else
			{
				pCmd->viewangles.y += 93.f;
			}
		}
	}

	enum ADAPTIVE_SIDE {
		ADAPTIVE_UNKNOWN,
		ADAPTIVE_LEFT,
		ADAPTIVE_RIGHT
	};

	enum ADAPTIVE_SIDE2 {
		ADAPTIVE_UNKNOWN2,
		ADAPTIVE_LEFT2,
		ADAPTIVE_RIGHT2
	};

	void adaptive2(CUserCmd * pCmd, bool& bSendPacket) {
		auto fov_to_player = [](Vector view_offset, Vector view, IClientEntity* m_entity, int hitbox)
		{
			CONST FLOAT MaxDegrees = 180.0f;
			Vector Angles = view;
			Vector Origin = view_offset;
			Vector Delta(0, 0, 0);
			Vector Forward(0, 0, 0);
			AngleVectors3(Angles, &Forward);
			Vector AimPos = GetHitboxPosition(m_entity, hitbox);
			VectorSubtract(AimPos, Origin, Delta);
			Normalize(Delta, Delta);
			FLOAT DotProduct = Forward.Dot(Delta);
			return (acos(DotProduct) * (MaxDegrees / PI));
		};

		auto m_local = hackManager.pLocal();

		int target = -1;
		float mfov = 20;

		Vector viewoffset = m_local->GetOrigin() + m_local->GetViewOffset();
		Vector view; Interfaces::Engine->GetViewAngles(view);

		for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++) {
			IClientEntity* m_entity = Interfaces::EntList->GetClientEntity(i);

			if (m_entity && m_entity->IsDormant() == false && m_entity->IsAlive() && m_entity->GetIndex() != hackManager.pLocal()->GetIndex()) {

				float fov = fov_to_player(viewoffset, view, m_entity, 0);
				if (fov < mfov) {
					mfov = fov;
					target = i;
				}
			}
		}

		ADAPTIVE_SIDE2 side = ADAPTIVE_UNKNOWN2;

		Vector at_target_angle;

		if (target) {
			auto m_entity = Interfaces::EntList->GetClientEntity(target);

			if (m_entity && m_entity->IsDormant() == false && m_entity->IsAlive() && m_entity->GetIndex() != hackManager.pLocal()->GetIndex()) {
				Vector pos_enemy;
				if (Render::WorldToScreen(m_entity->GetOrigin(), pos_enemy)) {
					CalcAngle(m_local->GetOrigin(), m_entity->GetOrigin(), at_target_angle);

					POINT mouse = GUI.GetMouse();

					if (mouse.x > pos_enemy.x) side = ADAPTIVE_RIGHT2;
					else if (mouse.x < pos_enemy.x) side = ADAPTIVE_LEFT2;
					else side = ADAPTIVE_UNKNOWN2;
				}
			}
		}

		if (side == ADAPTIVE_RIGHT) {
			pCmd->viewangles.y = at_target_angle.y + 89;
		}
		else if (side == ADAPTIVE_LEFT) {
			pCmd->viewangles.y = at_target_angle.y - 88;
		}

		if (side == ADAPTIVE_UNKNOWN) {
			pCmd->viewangles.y -= 180;
		}
	}


	void adaptive(CUserCmd * pCmd, bool& bSendPacket) {
		auto fov_to_player = [](Vector view_offset, Vector view, IClientEntity* m_entity, int hitbox)
		{
			CONST FLOAT MaxDegrees = 180.0f;
			Vector Angles = view;
			Vector Origin = view_offset;
			Vector Delta(0, 0, 0);
			Vector Forward(0, 0, 0);
			AngleVectors3(Angles, &Forward);
			Vector AimPos = GetHitboxPosition(m_entity, hitbox);
			VectorSubtract(AimPos, Origin, Delta);
			Normalize(Delta, Delta);
			FLOAT DotProduct = Forward.Dot(Delta);
			return (acos(DotProduct) * (MaxDegrees / PI));
		};

		auto m_local = hackManager.pLocal();

		int target = -1;
		float mfov = 20;

		Vector viewoffset = m_local->GetOrigin() + m_local->GetViewOffset();
		Vector view; Interfaces::Engine->GetViewAngles(view);

		for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++) {
			IClientEntity* m_entity = Interfaces::EntList->GetClientEntity(i);

			if (m_entity && m_entity->IsDormant() == false && m_entity->IsAlive() && m_entity->GetIndex() != hackManager.pLocal()->GetIndex()) {

				float fov = fov_to_player(viewoffset, view, m_entity, 0);
				if (fov < mfov) {
					mfov = fov;
					target = i;
				}
			}
		}

		ADAPTIVE_SIDE side = ADAPTIVE_UNKNOWN;

		Vector at_target_angle;

		if (target) {
			auto m_entity = Interfaces::EntList->GetClientEntity(target);

			if (m_entity && m_entity->IsDormant() == false && m_entity->IsAlive() && m_entity->GetIndex() != hackManager.pLocal()->GetIndex()) {
				Vector pos_enemy;
				if (Render::WorldToScreen(m_entity->GetOrigin(), pos_enemy)) {
					CalcAngle(m_local->GetOrigin(), m_entity->GetOrigin(), at_target_angle);

					POINT mouse = GUI.GetMouse();

					if (mouse.x > pos_enemy.x) side = ADAPTIVE_RIGHT;
					else if (mouse.x < pos_enemy.x) side = ADAPTIVE_LEFT;
					else side = ADAPTIVE_UNKNOWN;
				}
			}
		}

		if (side == ADAPTIVE_RIGHT) {
			pCmd->viewangles.y = at_target_angle.y - 88;
		}
		else if (side == ADAPTIVE_LEFT) {
			pCmd->viewangles.y = at_target_angle.y + 89;
		}


		if (side == ADAPTIVE_UNKNOWN) {
			pCmd->viewangles.y -= 177;
		}
	}


	void FastSpin(CUserCmd *pCmd)
	{
		static int y2 = -179;
		int spinBotSpeedFast = 100;

		y2 += spinBotSpeedFast;

		if (y2 >= 179)
			y2 = -179;

		pCmd->viewangles.y = y2;
	}

	
	void BackJitter(CUserCmd *pCmd)
	{
		int random = rand() % 100;

		if (random < 98)

			pCmd->viewangles.y -= 180;

		if (random < 15)
		{
			float change = -70 + (rand() % (int)(140 + 1));
			pCmd->viewangles.y += change;
		}
		if (random == 69)
		{
			float change = -90 + (rand() % (int)(180 + 1));
			pCmd->viewangles.y += change;
		}
	}

	void AntiCorrection(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 1.09f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 10.f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated)
			yaw = 90;
		else
			yaw = -90;

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void FakeJitterTest(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool flip = true;
		static bool Fast = true;
		static int ChokedPackets = -1;
		ChokedPackets++;
		static float Fake = 0;
		static float Real = 0;
		static int testerino;

		if (GetAsyncKeyState(0xA4) && testerino == 4) // A -  0x41   // N - 0x4E
		{
			//pCmd->viewangles.y -= 90.0f;
			testerino = 3;
		}

		else if (GetAsyncKeyState(0xA4) && testerino == 3)
		{ // D - 0x44    H -0x48 
		  //pCmd->viewangles.y += 90.0f;
			testerino = 4;

		}

		else {
			if (testerino == 3) {
				testerino = 3;

				if (ChokedPackets < 1)
				{
					if (flip)
					{
						int random = rand() % 100;
						int random2 = rand() % 1000;
						int random3 = rand() % 100;

						static bool dir;
						static float current_y = 0;

						if (random == 1) dir = !dir;

						if (dir)
							current_y = 90.0f;
						else
							current_y = 90.0f;

						Real = current_y;

						if (random == random2)
							Real += random;
						if (random3 == 1)
							Real = current_y + 180.0f;
					}
					else
					{
						int random = rand() % 100;
						int random2 = rand() % 1000;
						int random3 = rand() % 100;

						static bool dir;
						static float current_y = 0;

						if (random == 1) dir = !dir;

						if (dir)
							current_y = -90.0f;
						else
							current_y = -90.0f;

						Real = current_y;

						if (random == random2)
							Real -= random;
						if (random3 == 1)
							Real = current_y + 180.0f;
					}
					bSendPacket = false;
					pCmd->viewangles.y += Real;
				}
				else
				{
					if (Fast)
					{
						Fake = 90.0;
						Fast = !Fast;
					}
					else
					{
						Fake = -90.0;
						Fast = !Fast;
					}
					bSendPacket = true;
					pCmd->viewangles.y += Fake;
					ChokedPackets = -1;
				}
			}
			else
			{
				testerino = 4;

				if (ChokedPackets < 1)
				{
					if (flip)
					{
						int random = rand() % 100;
						int random2 = rand() % 1000;
						int random3 = rand() % 100;

						static bool dir;
						static float current_y = 0;

						if (random == 1) dir = !dir;

						if (dir)
							current_y = 90.0f;
						else
							current_y = 90.0f;

						Real = current_y;

						if (random == random2)
							Real += random;
						if (random3 == 1)
							Real = current_y + 180.0f;
					}
					else
					{
						int random = rand() % 100;
						int random2 = rand() % 1000;
						int random3 = rand() % 100;

						static bool dir;
						static float current_y = 0;

						if (random == 1) dir = !dir;

						if (dir)
							current_y = -90.0f;
						else
							current_y = -90.0f;

						Real = current_y;

						if (random == random2)
							Real -= random;
						if (random3 == 1)
							Real = current_y + 180.0f;
					}
					bSendPacket = false;
					pCmd->viewangles.y -= Real;
				}
				else
				{
					if (Fast)
					{
						Fake = 90.0;
						Fast = !Fast;
					}
					else
					{
						Fake = -90.0;
						Fast = !Fast;
					}
					bSendPacket = true;
					pCmd->viewangles.y -= Fake;
					ChokedPackets = -1;
				}
			}
		}
	}

	void Switch(CUserCmd *pCmd)
	{
		static int testerino;

		if (GetAsyncKeyState(0xA4) && testerino == 4) // A -  0x41   // N - 0x4E
		{
			//pCmd->viewangles.y -= 90.0f;
			testerino = 3;
		}

		else if (GetAsyncKeyState(0xA4) && testerino == 3)
		{ // D - 0x44    H -0x48 
		  //pCmd->viewangles.y += 90.0f;
			testerino = 4;

		}

		else {
			if (testerino == 3) {
				testerino = 3;
				pCmd->viewangles.y += 90;
			}
			else
			{
				testerino = 4;
				pCmd->viewangles.y -= 90;
			}
		}
	}

	void SwitchAlt(CUserCmd *pCmd)
	{
		static int testerino;

		if (GetAsyncKeyState(0xA4) && testerino == 4) // A -  0x41   // N - 0x4E
		{
			//pCmd->viewangles.y -= 90.0f;
			testerino = 3;
		}

		else if (GetAsyncKeyState(0xA4) && testerino == 3)
		{ // D - 0x44    H -0x48 
		  //pCmd->viewangles.y += 90.0f;
			testerino = 4;

		}

		else {
			if (testerino == 3) {
				testerino = 3;
				pCmd->viewangles.y -= 90;
			}
			else
			{
				testerino = 4;
				pCmd->viewangles.y += 90;
			}
		}
	}

	void FakeJitterMoveTest(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool flip = true;
		static bool Fast = true;
		static int ChokedPackets = -1;
		ChokedPackets++;
		static float Fake = 0;
		static float Real = 0;
		static int testerino;

		if (GetAsyncKeyState(0xA4) && testerino == 4) // A -  0x41   // N - 0x4E
		{
			//pCmd->viewangles.y -= 90.0f;
			testerino = 3;
		}

		else if (GetAsyncKeyState(0xA4) && testerino == 3)
		{ // D - 0x44    H -0x48 
		  //pCmd->viewangles.y += 90.0f;
			testerino = 4;

		}

		else {
			if (testerino == 3) {
				testerino = 3;

				if (ChokedPackets < 1)
				{
					if (hackManager.pLocal()->GetVelocity().Length2D() <= 1.0f && hackManager.pLocal()->GetFlags() & FL_ONGROUND)
					{
						if (flip)
						{
							int random = rand() % 100;
							int random2 = rand() % 1000;
							int random3 = rand() % 100;

							static bool dir;
							static float current_y = 0;

							if (random == 1) dir = !dir;

							if (dir)
								current_y = 90.0f;
							else
								current_y = 90.0f;

							Real = current_y;

							if (random == random2)
								Real += random;
							if (random3 == 1)
								Real = current_y + 180.0f;
						}
						else
						{
							int random = rand() % 100;
							int random2 = rand() % 1000;
							int random3 = rand() % 100;

							static bool dir;
							static float current_y = 0;

							if (random == 1) dir = !dir;

							if (dir)
								current_y = -90.0f;
							else
								current_y = -90.0f;

							Real = current_y;

							if (random == random2)
								Real -= random;
							if (random3 == 1)
								Real = current_y + 180.0f;
						}
					}
					else
					{
						Real = 180 + rand() % 20;
					}
					bSendPacket = false;
					pCmd->viewangles.y += Real;
				}
				else
				{
					if (Fast)
					{
						Fake = 90.0;
						Fast = !Fast;
					}
					else
					{
						Fake = -90.0;
						Fast = !Fast;
					}
					bSendPacket = true;
					pCmd->viewangles.y += Fake;
					ChokedPackets = -1;
				}
			}
			else
			{
				testerino = 4;

				if (ChokedPackets < 1)
				{
					if (hackManager.pLocal()->GetVelocity().Length2D() <= 1.0f && hackManager.pLocal()->GetFlags() & FL_ONGROUND)
					{
						if (flip)
						{
							int random = rand() % 100;
							int random2 = rand() % 1000;
							int random3 = rand() % 100;

							static bool dir;
							static float current_y = 0;

							if (random == 1) dir = !dir;

							if (dir)
								current_y = 90.0f;
							else
								current_y = 90.0f;

							Real = current_y;

							if (random == random2)
								Real += random;
							if (random3 == 1)
								Real = current_y + 180.0f;
						}
						else
						{
							int random = rand() % 100;
							int random2 = rand() % 1000;
							int random3 = rand() % 100;

							static bool dir;
							static float current_y = 0;

							if (random == 1) dir = !dir;

							if (dir)
								current_y = -90.0f;
							else
								current_y = -90.0f;

							Real = current_y;

							if (random == random2)
								Real -= random;
							if (random3 == 1)
								Real = current_y + 180.0f;
						}
					}
					else
					{
						Real = 180 + rand() % 20;
					}
					bSendPacket = false;
					pCmd->viewangles.y -= Real;
				}
				else
				{
					if (Fast)
					{
						Fake = 90.0;
						Fast = !Fast;
					}
					else
					{
						Fake = -90.0;
						Fast = !Fast;
					}
					bSendPacket = true;
					pCmd->viewangles.y -= Fake;
					ChokedPackets = -1;
				}
			}
		}
	}

	void FakeMemes(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		static float Fake = 0;
		static float Real = 0;
		static int testerino = 3;
		if (GetAsyncKeyState(0xA4) && testerino == 4) // A -  0x41   // N - 0x4E
		{
			//pCmd->viewangles.y -= 90.0f;
			testerino = 3;
		}

		else if (GetAsyncKeyState(0xA4) && testerino == 3)
		{ // D - 0x44    H -0x48 
		  //pCmd->viewangles.y += 90.0f;
			testerino = 4;

		}

		else {
			if (testerino == 3)
			{
				testerino = 3;

				if (hackManager.pLocal()->GetVelocity().Length2D() <= 1.0f && hackManager.pLocal()->GetFlags() & FL_ONGROUND)
				{
					if (ChokedPackets < 1)
					{
						Real = -90;
						bSendPacket = false;
						pCmd->viewangles.y -= Real;
					}
					else
					{
						Fake = 90 + rand() % 20;
						bSendPacket = true;
						pCmd->viewangles.y -= Fake;
						ChokedPackets = -1;
					}
				}
				else
				{
					if (ChokedPackets < 1)
					{
						Real = -90 + rand() % 20;
						bSendPacket = false;
						pCmd->viewangles.y -= Real;
					}
					else
					{
						Fake = -90;
						bSendPacket = true;
						pCmd->viewangles.y -= Fake;
						ChokedPackets = -1;
					}
				}
			}
			else
			{
				testerino = 4;

				if (hackManager.pLocal()->GetVelocity().Length2D() <= 1.0f && hackManager.pLocal()->GetFlags() & FL_ONGROUND)
				{
					if (ChokedPackets < 1)
					{
						Real = 90;
						bSendPacket = false;
						pCmd->viewangles.y -= Real;
					}
					else
					{
						Fake = -90 + rand() % 20;
						bSendPacket = true;
						pCmd->viewangles.y -= Fake;
						ChokedPackets = -1;
					}
				}
				else
				{
					if (ChokedPackets < 1)
					{
						Real = 90 + rand() % 20;
						bSendPacket = false;
						pCmd->viewangles.y -= Real;
					}
					else
					{
						Fake = 90;
						bSendPacket = true;
						pCmd->viewangles.y -= Fake;
						ChokedPackets = -1;
					}
				}
			}
		}
	}

	void AntiCorrectionALT(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 1.09f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 10.f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated)
			yaw = -90;
		else
			yaw = 90;

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void FakeSideways(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.y += 90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180;
			ChokedPackets = -1;
		}
	}

	void FastSpint(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 15 + rand() % 10;
		else
			current_y -= 15 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}

	void BackwardJitter(CUserCmd *pCmd)
	{
		int random = rand() % 100;

		if (random < 98)

			pCmd->viewangles.y -= 180;

		if (random < 15)
		{
			float change = -70 + (rand() % (int)(140 + 1));
			pCmd->viewangles.y += change;
		}
		if (random == 69)
		{
			float change = -90 + (rand() % (int)(180 + 1));
			pCmd->viewangles.y += change;
		}
	}

	void Fake1(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 34;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y -= 36;
		}

		int re = rand() % 4 + 1;


		if (jitterangle <= 1)
		{
			if (re == 4)
				pCmd->viewangles.y += 91;
			jitterangle += 1;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			if (re == 4)
				pCmd->viewangles.y -= 89;
			jitterangle += 1;
		}
		else
		{
			jitterangle = 0;
		}
	}

	void Fake2(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.y += 23;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 244;
			ChokedPackets = -1;
		}
	}

	void FakeTank(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.y = -92;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y = 278;
			ChokedPackets = -1;

		}
	}

	void LbyBreakFakeLeft(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;
		int random = rand() % 100;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 0.09f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 0.9f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (random > 10 && random < 50)
			yaw = 159.3;
		else if (random < 50)
			yaw = -162.7;
		else if (LBYUpdated && random < 10 && random > 7)
			yaw = -91.2;
		else if (LBYUpdated && random < 6 && random > 3)
			yaw = -89.7;
		else if (LBYUpdated && random < 3)
			yaw = -90.3;

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void LbyBreakFakeRight(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;
		int random = rand() % 100;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 0.069f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 1.1f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (random > 10 && random < 50)
			yaw = -159.3;
		else if (random < 50)
			yaw = 153 - 7;
		else if (LBYUpdated && random < 10 && random > 7)
			yaw = 91.2;
		else if (LBYUpdated && random < 6 && random > 3)
			yaw = 89.7;
		else if (LBYUpdated && random < 3)
			yaw = 90.3;

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void LbyBreakRealRight(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;
		int random = rand() % 100;

		static int ChokedPackets = -1;
		ChokedPackets++;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 0.04f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 1.1f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated && random > 91)
			yaw = -178.1;
		else
			yaw = 91.8;

		/*			yaw = -91.1;
		else
		yaw = -179.1;*/

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void LbyBreakRealLeft(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;
		int random = rand() % 100;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 0.05f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 1.2f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated && random < 3)
			yaw = 177.9;
		else
			yaw = -92.1;
		/*			yaw = -91.1;
		else
		yaw = 178.3;*/

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void LBY180L(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;
		int random = rand() % 100;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 0.001f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 0.01f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated && random < 3)
			yaw = 90;
		else
			yaw = -91.1;
		/*			yaw = -91.1;
		else
		yaw = 178.3;*/

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void LBY180R(CUserCmd* pCmd)
	{
		Vector newAngle = pCmd->viewangles;

		static int ChokedPackets = -1;
		ChokedPackets++;
		int random = rand() % 100;

		float yaw;
		static int state = 0;
		static bool LBYUpdated = false;

		float flCurTime = Interfaces::Globals->curtime;
		static float flTimeUpdate = 0.001f;
		static float flNextTimeUpdate = flCurTime + flTimeUpdate;
		if (flCurTime >= flNextTimeUpdate) {
			LBYUpdated = !LBYUpdated;
			state = 0;
		}

		if (flNextTimeUpdate < flCurTime || flNextTimeUpdate - flCurTime > 0.002f)
			flNextTimeUpdate = flCurTime + flTimeUpdate;

		if (LBYUpdated && random < 3)
			yaw = -90;
		else
			yaw = 88.92;
		/*			yaw = -91.1;
		else
		yaw = 178.3;*/

		if (yaw)
			newAngle.y += yaw;

		pCmd->viewangles = newAngle;
	}

	void FakeLowerbodyMeme(CUserCmd *pCmd, bool &bSendPacket)
	{
		bSendPacket = true;
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y -= 90.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += 90.f;
		}
	}

	void FakeLowerbodyMeme2(CUserCmd *pCmd, bool &bSendPacket)
	{
		bSendPacket = true;
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y += 90.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y -= 90.f;
		}
	}

	void LowerbodyMeme(CUserCmd *pCmd)
	{
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y -= 90.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y += 90.f;
		}
	}

	void LowerbodyMeme2(CUserCmd *pCmd)
	{
		static bool wilupdate;
		static float LastLBYUpdateTime = 0;
		IClientEntity* pLocal = hackManager.pLocal();
		float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
		if (server_time >= LastLBYUpdateTime)
		{
			LastLBYUpdateTime = server_time + 1.125f;
			wilupdate = true;
			pCmd->viewangles.y += 90.f;
		}
		else
		{
			wilupdate = false;
			pCmd->viewangles.y -= 90.f;
		}
	}

	void Jitter(CUserCmd *pCmd)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 90;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y -= 90;
		}

		int re = rand() % 4 + 1;


		if (jitterangle <= 1)
		{
			if (re == 4)
				pCmd->viewangles.y += 180;
			jitterangle += 1;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			if (re == 4)
				pCmd->viewangles.y -= 180;
			jitterangle += 1;
		}
		else
		{
			jitterangle = 0;
		}
	}

	void FakeStatic(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			static int y2 = -179;
			int spinBotSpeedFast = 360.0f / 1.618033988749895f;;

			y2 += spinBotSpeedFast;

			if (y2 >= 179)
				y2 = -179;

			pCmd->viewangles.y = y2;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y -= 180;
			ChokedPackets = -1;
		}
	}

	void TJitter(CUserCmd *pCmd)
	{
		static bool Turbo = true;
		if (Turbo)
		{
			pCmd->viewangles.y -= 90;
			Turbo = !Turbo;
		}
		else
		{
			pCmd->viewangles.y += 90;
			Turbo = !Turbo;
		}
	}

	void TFake(CUserCmd *pCmd, bool &bSendPacket)
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{
			bSendPacket = false;
			pCmd->viewangles.y = -90;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles.y = 90;
			ChokedPackets = -1;
		}
	}

	void FakeJitter(CUserCmd* pCmd, bool &bSendPacket)
	{
		static int jitterangle = 0;

		if (jitterangle <= 1)
		{
			pCmd->viewangles.y += 135;
		}
		else if (jitterangle > 1 && jitterangle <= 3)
		{
			pCmd->viewangles.y += 225;
		}

		static int iChoked = -1;
		iChoked++;
		if (iChoked < 1)
		{
			bSendPacket = false;
			if (jitterangle <= 1)
			{
				pCmd->viewangles.y += 45;
				jitterangle += 1;
			}
			else if (jitterangle > 1 && jitterangle <= 3)
			{
				pCmd->viewangles.y -= 45;
				jitterangle += 1;
			}
			else
			{
				jitterangle = 0;
			}
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;
		}
	}


	void Up(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = -89.0f;
	}

	void Zero(CUserCmd *pCmd)
	{
		pCmd->viewangles.x = 0.f;
	}

	void Static(CUserCmd *pCmd)
	{
		static bool aa1 = false;
		aa1 = !aa1;
		if (aa1)
		{
			static bool turbo = false;
			turbo = !turbo;
			if (turbo)
			{
				pCmd->viewangles.y -= 90;
			}
			else
			{
				pCmd->viewangles.y += 90;
			}
		}
		else
		{
			pCmd->viewangles.y -= 180;
		}
	}

	void fakelowerbody(CUserCmd *pCmd, bool &bSendPacket)
	{
		static bool f_flip = true;
		f_flip = !f_flip;

		if (f_flip)
		{
			pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + 90.00f;
			bSendPacket = false;
		}
		else if (!f_flip)
		{
			pCmd->viewangles.y += hackManager.pLocal()->GetLowerBodyYaw() - 90.00f;
			bSendPacket = true;
		}
	}
	void LBYJitter(CUserCmd* cmd, bool& packet)
	{
		static bool ySwitch;
		static bool jbool;
		static bool jboolt;
		ySwitch = !ySwitch;
		jbool = !jbool;
		jboolt = !jbool;
		if (ySwitch)
		{
			if (jbool)
			{
				if (jboolt)
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 90.f;
					packet = false;
				}
				else
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90.f;
					packet = false;
				}
			}
			else
			{
				if (jboolt)
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 125.f;
					packet = false;
				}
				else
				{
					cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 125.f;
					packet = false;
				}
			}
		}
		else
		{
			cmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw();
			packet = true;
		}
	}

	void LBYSpin(CUserCmd *pCmd, bool &bSendPacket)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		static int skeet = 179;
		int SpinSpeed = 100;
		static int ChokedPackets = -1;
		ChokedPackets++;
		skeet += SpinSpeed;

		if
			(pCmd->command_number % 9)
		{
			bSendPacket = true;
			if (skeet >= pLocal->GetLowerBodyYaw() + 180);
			skeet = pLocal->GetLowerBodyYaw() - 0;
			ChokedPackets = -1;
		}
		else if
			(pCmd->command_number % 9)
		{
			bSendPacket = false;
			pCmd->viewangles.y += 179;
			ChokedPackets = -1;
		}
		pCmd->viewangles.y = skeet;
	}

	void SlowSpin(CUserCmd *pCmd)
	{
		int r1 = rand() % 100;
		int r2 = rand() % 1000;

		static bool dir;
		static float current_y = pCmd->viewangles.y;

		if (r1 == 1) dir = !dir;

		if (dir)
			current_y += 4 + rand() % 10;
		else
			current_y -= 4 + rand() % 10;

		pCmd->viewangles.y = current_y;

		if (r1 == r2)
			pCmd->viewangles.y += r1;
	}
}

void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;

	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;
	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;

	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);

	delta_view = 360.0f - delta_view;

	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}

float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{

		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{

		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{

		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{

		return 0.0f;
	}
}

float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;
float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}
bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();

	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);


	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{

		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}

	if (CurrentVelocity(LocalPlayer) > 0.5)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}

	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 1.1 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 1.1;
		}
		return true;
	}
	return false;
}

void SideJitterALT(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		static bool Fast2 = false;
		if (Fast2)
		{
			pCmd->viewangles.y += 75;
		}
		else
		{
			pCmd->viewangles.y += 105;
		}
		Fast2 = !Fast2;
	}
	else
	{
		pCmd->viewangles.y += 90;
	}
}

void SideJitter(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		static bool Fast2 = false;
		if (Fast2)
		{
			pCmd->viewangles.y -= 75;
		}
		else
		{
			pCmd->viewangles.y -= 105;
		}
		Fast2 = !Fast2;
	}
	else
	{
		pCmd->viewangles.y -= 90;
	}
}

void DoLBYBreak(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y -= 90;
	}
	else
	{
		pCmd->viewangles.y += 90;
	}
}

void DoLBYBreakReal(CUserCmd * pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	if (!bSendPacket)
	{
		pCmd->viewangles.y += 90;
	}
	else
	{
		pCmd->viewangles.y -= 90;
	}
}

extern float lineRealAngle;
extern float lineFakeAngle;

void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{
	static bool flip = true;
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;

	if (Menu::Window.RageBotTab.BreakLBY.GetIndex() == 0)
	{
		//nothing nigger
	}

	if (Menu::Window.RageBotTab.BreakLBY.GetIndex() == 1)
	{
		if (NextLBYUpdate())
		{
			//	if (side == AAYaw::ADAPTIVE_LEFT) //prob what u are here for, ignore the adaptive shit, it was for testing. hf
			pCmd->viewangles.y += 45;
			//	else if (side == AAYaw::ADAPTIVE_RIGHT)
			pCmd->viewangles.y -= 45;
			//else
			{
				pCmd->viewangles.y += 45;
			}
		}
	}

	if (Menu::Window.RageBotTab.BreakLBY.GetIndex() == 2)
	{
		if (NextLBYUpdate())
		{
			//	if (side == AAYaw::ADAPTIVE_LEFT) //prob what u are here for, ignore the adaptive shit, it was for testing. hf
			pCmd->viewangles.y += 90;
			//	else if (side == AAYaw::ADAPTIVE_RIGHT)
			pCmd->viewangles.y -= 90;
			//else
			{
				pCmd->viewangles.y += 90;
			}
		}
	}

	if (Menu::Window.RageBotTab.BreakLBY.GetIndex() == 3)
	{
		if (NextLBYUpdate())
		{
			//	if (side == AAYaw::ADAPTIVE_LEFT) //prob what u are here for, ignore the adaptive shit, it was for testing. hf
			pCmd->viewangles.y += 180;
			//	else if (side == AAYaw::ADAPTIVE_RIGHT)
			pCmd->viewangles.y -= 180;
			//else
			{
				pCmd->viewangles.y += 180;
			}
		}
	}

	if (Menu::Window.RageBotTab.BreakLBY.GetIndex() == 4)
	{
		if (NextLBYUpdate())
		{
			//	if (side == AAYaw::ADAPTIVE_LEFT) //prob what u are here for, ignore the adaptive shit, it was for testing. hf
			pCmd->viewangles.y = lineFakeAngle;
			//	else if (side == AAYaw::ADAPTIVE_RIGHT)
			pCmd->viewangles.y = lineRealAngle;
			//else
			{
				pCmd->viewangles.y = lineFakeAngle;
			}
		}
	}


	switch (Menu::Window.RageBotTab.AntiAimYaw.GetIndex())
	{
	case 0:
		break;
	case 1:
		// Fast Spin
		AntiAims::FastSpint(pCmd);
		break;
	case 2:
		// Slow Spin
		AntiAims::SlowSpin(pCmd);
		break;
	case 3:
		AntiAims::Jitter(pCmd);
		break;
	case 4:
		// 180 Jitter
		AntiAims::BackJitter(pCmd);
		break;
	case 5:
		//backwards
		pCmd->viewangles.y -= 180;
		break;
	case 6:
		AntiAims::BackwardJitter(pCmd);
		break;
	case 7:
		//Sideways-switch
		if (switch2)
			pCmd->viewangles.y = 90;
		else
			pCmd->viewangles.y = -90;

		switch2 = !switch2;
		break;
	case 8:
		//Sideways
		pCmd->viewangles.y -= 90;
		break;
	case 9:
		pCmd->viewangles.y += 90;
		break;
	case 10:
		pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
		break;
	case 11:
		AntiAims::LBYJitter(pCmd, bSendPacket);
		break;
	case 12:
		AntiAims::FakeSideLBY(pCmd, bSendPacket);
		break;
	case 13:
		AntiAims::LBYSpin(pCmd, bSendPacket);
		break;
	case 14:
		AntiAims::FakeJitterTest(pCmd, bSendPacket);
		break;
	case 15:
		AntiAims::FakeJitterMoveTest(pCmd, bSendPacket);
		break;
	case 16:
		AntiAims::FakeMemes(pCmd, bSendPacket);
		break;
	case 17:
		AntiAims::Switch(pCmd);
		break;
	case 18:
		if (flipBool)

		{
			AntiAims::adaptive2(pCmd, bSendPacket);
		}

		else if (!flipBool)
		{
			AntiAims::adaptive(pCmd, bSendPacket);
		}
		break;
	case 19:
		if (flipBool)

		{
			AntiAims::AntiCorrectionALT(pCmd);
		}

		else if (!flipBool)
		{
			AntiAims::AntiCorrection(pCmd);
		}
		break;
	case 20:
		AntiAims::fakelowerbody(pCmd, bSendPacket);
		break;
	case 21:
		if (flipBool)

		{
			AntiAims::LbyBreakRealRight(pCmd);
		}

		else if (!flipBool)
		{
			AntiAims::LbyBreakRealLeft(pCmd);
		}
		break;
	case 22:
		AntiAims::Fake1(pCmd, bSendPacket);
		break;
	case 23:
		AntiAims::Fake2(pCmd, bSendPacket);
		break;
	case 24:
		AntiAims::FakeTank(pCmd, bSendPacket);
		break;
	case 25:

		if (flipBool)

		{
			AntiAims::LBY180R(pCmd);
		}

		else if (!flipBool)
		{
			AntiAims::LBY180L(pCmd);
		}
		break;
	case 26:
		if (flipBool)
		{
			AntiAims::LowerbodyMeme(pCmd);
		}
		else if (!flipBool)
		{
			AntiAims::LowerbodyMeme2(pCmd);
		}
		break;
	}


	if (hackManager.pLocal()->GetVelocity().Length() > 0) {
		switch (Menu::Window.RageBotTab.MoveYaw.GetIndex())
		{
			//bSendPacket = false;
		case 0:
			break;
		case 1:
			// Fast Spin
			AntiAims::FastSpint(pCmd);
			break;
		case 2:
			// Slow Spin
			AntiAims::SlowSpin(pCmd);
			break;
		case 3:
			AntiAims::Jitter(pCmd);
			break;
		case 4:
			// 180 Jitter
			AntiAims::BackJitter(pCmd);
			break;
		case 5:
			//backwards
			pCmd->viewangles.y -= 180;
			break;
		case 6:
			AntiAims::BackwardJitter(pCmd);
			break;
		case 7:
			//Sideways-switch
			if (switch2)
				pCmd->viewangles.y = 90;
			else
				pCmd->viewangles.y = -90;

			switch2 = !switch2;
			break;
		case 8:
			//Sideways
			pCmd->viewangles.y -= 90;
			break;
		case 9:
			pCmd->viewangles.y += 90;
			break;
		case 10:
			pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
			break;
		case 11:
			AntiAims::LBYJitter(pCmd, bSendPacket);
			break;
		case 12:
			AntiAims::FakeSideLBY(pCmd, bSendPacket);
			break;
		case 13:
			AntiAims::LBYSpin(pCmd, bSendPacket);
			break;
		case 14:
			//Sideways
			pCmd->viewangles.y += 0;
			break;
		}
	}
}

void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{
	static bool flip = true;
	static bool switch2;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;
	if (!Menu::Window.RageBotTab.AntiAimEnable.GetState())
		return;
	switch (Menu::Window.RageBotTab.FakeYaw.GetIndex())
	{
	case 0:
		break;
	case 1:
		// Fast Spin 
		AntiAims::FastSpint(pCmd);
		break;
	case 2:
		// Slow Spin 
		AntiAims::SlowSpin(pCmd);
		break;
	case 3:
		AntiAims::Jitter(pCmd);
		break;
	case 4:
		// 180 Jitter 
		AntiAims::BackJitter(pCmd);
		break;
	case 5:
		//backwards
		pCmd->viewangles.y -= 180;
		break;
	case 6:
		AntiAims::BackwardJitter(pCmd);
		break;
	case 7:
		//Sideways-switch
		if (switch2)
			pCmd->viewangles.y = 90;
		else
			pCmd->viewangles.y = -90;

		switch2 = !switch2;
		break;
	case 8:
		pCmd->viewangles.y -= 90;
		break;
	case 9:
		pCmd->viewangles.y += 90;
		break;
	case 10:
		pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
		break;
	case 11:
		AntiAims::LBYJitter(pCmd, bSendPacket);
		break;
	case 12:
		AntiAims::FakeSideLBY(pCmd, bSendPacket);
		break;
	case 13:
		AntiAims::LBYSpin(pCmd, bSendPacket);
		break;
	case 14:
		AntiAims::SwitchAlt(pCmd);
		break;
	case 15:
		if (flipBool)

		{
			AntiAims::adaptive(pCmd, bSendPacket);
		}

		else if (!flipBool)
		{
			AntiAims::adaptive2(pCmd, bSendPacket);
		}
		break;
	case 16:
		if (flipBool)

		{
			AntiAims::AntiCorrection(pCmd);
		}

		else if (!flipBool)
		{
			AntiAims::AntiCorrectionALT(pCmd);
		}
		break;
	case 17:
		AntiAims::fakelowerbody(pCmd, bSendPacket);
		break;
	case 18:
		if (flipBool)

		{
			AntiAims::LbyBreakFakeRight(pCmd);
		}

		else if (!flipBool)
		{
			AntiAims::LbyBreakFakeLeft(pCmd);
		}
		break;
	case 19:
		AntiAims::Fake1(pCmd, bSendPacket);
		break;
	case 20:
		AntiAims::Fake2(pCmd, bSendPacket);
		break;
	case 21:
		AntiAims::FakeTank(pCmd, bSendPacket);
		break;
	case 22:
		if (flipBool)
		{
			AntiAims::FakeLowerbodyMeme(pCmd, bSendPacket);
		}
		else if (!flipBool)
		{
			AntiAims::FakeLowerbodyMeme2(pCmd, bSendPacket);
		}
		break;
	}


		if (hackManager.pLocal()->GetVelocity().Length() > 0) {
		switch (Menu::Window.RageBotTab.MoveYawFake.GetIndex())
		{
			//bSendPacket = false;
		case 0:
			break;
		case 1:
			// Fast Spin 
			AntiAims::FastSpint(pCmd);
			break;
		case 2:
			// Slow Spin 
			AntiAims::SlowSpin(pCmd);
			break;
		case 3:
			AntiAims::Jitter(pCmd);
			break;
		case 4:
			// 180 Jitter 
			AntiAims::BackJitter(pCmd);
			break;
		case 5:
			//backwards
			pCmd->viewangles.y -= 180;
			break;
		case 6:
			AntiAims::BackwardJitter(pCmd);
			break;
		case 7:
			//Sideways-switch
			if (switch2)
				pCmd->viewangles.y = 90;
			else
				pCmd->viewangles.y = -90;

			switch2 = !switch2;
			break;
		case 8:
			pCmd->viewangles.y -= 90;
			break;
		case 9:
			pCmd->viewangles.y += 90;
			break;
		case 10:
			pCmd->viewangles.y = pLocal->GetLowerBodyYaw() + rand() % 180 - rand() % 50;
			break;
		case 11:
			AntiAims::LBYJitter(pCmd, bSendPacket);
			break;
		case 12:
			AntiAims::FakeSideLBY(pCmd, bSendPacket);
			break;
		case 13:
			AntiAims::LBYSpin(pCmd, bSendPacket);
			break;
		case 14:
			pCmd->viewangles.y += 0;
			break;
		}
	}
}

void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (IsAimStepping || pCmd->buttons & IN_ATTACK)
		return;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();

		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
				return;

		}
	}
	if (Menu::Window.RageBotTab.AntiAimTarget.GetState())
	{
		aimAtPlayer(pCmd);

	}

	FakeWalk(pCmd, bSendPacket);

	switch (Menu::Window.RageBotTab.AntiAimPitch.GetIndex())
	{
	case 0:
		break;
	case 1:
		pCmd->viewangles.x = 45.f;
		break;
	case 2:
		AntiAims::JitterPitch(pCmd);
		break;
	case 3:
		pCmd->viewangles.x = 89.000000;
		break;
	case 4:
		AntiAims::Up(pCmd);
		break;
	case 5:
		AntiAims::Zero(pCmd);
		break;
	case 6:
		AntiAims::UpsideDown(pCmd);
		break;
	case 7:
		AntiAims::UpsideDown2(pCmd);
		break;
	case 8:
		AntiAims::UpsideDownJitter(pCmd);
		break;

	}

	if (Menu::Window.RageBotTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = -1;
		ChokedPackets++;
		if (ChokedPackets < 1)
		{

			bSendPacket = true;
			DoFakeAA(pCmd, bSendPacket, pLocal);
		}
		else
		{

			bSendPacket = false;
			DoRealAA(pCmd, pLocal, bSendPacket);
			ChokedPackets = -1;
		}

		if (flipAA)
		{
			pCmd->viewangles.y -= 25;
		}
	}

}

