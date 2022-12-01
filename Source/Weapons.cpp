import <cstdint>;

import <unordered_map>;

import Hook;
import Task;
import Weapons;

import util;

inline std::unordered_map<int, int> g_rgiLastZoomLevel = {};

void __fastcall HamF_Entity_Spawn(CBasePlayerWeapon *pThis, int) noexcept
{
	auto const pszClassName = STRING(pThis->pev->classname);

	for (auto &[psz, pfnSpawn, pfnSecondaryAttack] : g_rgWeaponSecondaryAttackHooks)
	{
		if (!strcmp(psz, pszClassName))
		{
			pfnSpawn(pThis);
			break;
		}
	}

	// aug and sg552 will never use it, just doesn't matter.
	g_rgiLastZoomLevel[pThis->entindex()] = 40;
}

Task Task_Zoom(CBasePlayer *pPlayer, int const iFOV, int iSpeed) noexcept
{
	if (iSpeed == 0)
		co_return;

	if (pPlayer->m_iFOV > iFOV)
		iSpeed = -iSpeed;

	auto const iOrder = pPlayer->entindex() % TaskScheduler::NextFrame::Rank.size();

	for (; pPlayer->IsAlive() && pPlayer->m_iFOV != iFOV;)
	{
		if ((iSpeed < 0 && pPlayer->m_iFOV + iSpeed < iFOV) ||
			(iSpeed > 0 && pPlayer->m_iFOV + iSpeed > iFOV))
		{
			pPlayer->m_iFOV = iFOV;
			co_return;
		}

		pPlayer->m_iFOV += iSpeed;
		co_await TaskScheduler::NextFrame::Rank[iOrder];
	}
}

void __fastcall HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis, int) noexcept
{
	auto &m_iId = pThis->m_iId;
	auto &m_pPlayer = pThis->m_pPlayer;
	auto &m_flNextSecondaryAttack = pThis->m_flNextSecondaryAttack;

	if (m_flNextSecondaryAttack > 0)
		return;

	auto const iPlayerTaskId = TASK_ZOOM | (1ull << ((uint64_t)m_pPlayer->entindex() + 32ull));
	auto const iSpeed = (int)std::roundf(g_engfuncs.pfnCVarGetFloat(gcvarZoomStep.name));

	if (TaskScheduler::Exist(iPlayerTaskId))
		return;

	switch (m_iId)
	{
	case WEAPON_AUG:
	case WEAPON_SG552:
		TaskScheduler::Enroll(Task_Zoom(m_pPlayer, m_pPlayer->m_iFOV < 90 ? 90 : 55, iSpeed), iPlayerTaskId);

		m_flNextSecondaryAttack = g_engfuncs.pfnCVarGetFloat(gcvarCooldown.name);
		g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2f, 2.4f, 0, 100);
		break;

	case WEAPON_SCOUT:
	case WEAPON_SG550:
	case WEAPON_G3SG1:
	case WEAPON_AWP:
		if (m_pPlayer, m_pPlayer->m_iFOV < 90)
		{
			g_rgiLastZoomLevel[pThis->entindex()] = m_pPlayer->m_iFOV;
			TaskScheduler::Enroll(Task_Zoom(m_pPlayer, 90, iSpeed), iPlayerTaskId);
		}
		else
			TaskScheduler::Enroll(Task_Zoom(m_pPlayer, g_rgiLastZoomLevel[pThis->entindex()], iSpeed), iPlayerTaskId);

		m_flNextSecondaryAttack = g_engfuncs.pfnCVarGetFloat(gcvarCooldown.name);
		g_engfuncs.pfnEmitSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2f, 2.4f, 0, 100);
		break;

	default:
		std::unreachable();
	}
}
