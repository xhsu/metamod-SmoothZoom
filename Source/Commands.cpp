import <algorithm>;
import <string>;

import CBase;
import Hook;
import Plugin;
import Task;
import Weapons;

META_RES OnClientCommand(CBasePlayer *pPlayer, const std::string &szCommand) noexcept
{
	if (!pPlayer->IsAlive())
		return MRES_IGNORED;

	if (auto const pWeapon = (CBasePlayerWeapon *)pPlayer->m_pActiveItem)
	{
		switch (pWeapon->m_iId)
		{
		case WEAPON_SCOUT:
		case WEAPON_SG550:
		case WEAPON_AWP:
		case WEAPON_G3SG1:
			break;	// 'continue' to the next step.

		default:
			return MRES_IGNORED;
		}
	}
	else [[unlikely]]
		return MRES_IGNORED;

	// Must be in zoom mode.
	if (pPlayer->m_iFOV > 40)
		return MRES_IGNORED;

	int8_t ret = 0;

	if (szCommand == "zoomin")
	{
		ret = -(int)std::roundf(g_engfuncs.pfnCVarGetFloat(gcvarScrollStep.name));
	}
	else if (szCommand == "zoomout")
	{
		ret = (int)std::roundf(g_engfuncs.pfnCVarGetFloat(gcvarScrollStep.name));
	}

	if (!ret)
		return MRES_IGNORED;

	auto const iPlayerTaskId = TASK_ZOOM | (1ull << ((uint64_t)pPlayer->entindex() + 32ull));
	auto const iNewFOV = std::clamp(pPlayer->m_iFOV + ret, 10, 40);

	TaskScheduler::Delist(iPlayerTaskId);
	TaskScheduler::Enroll(Task_Zoom(pPlayer, iNewFOV, 1), iPlayerTaskId);

	return MRES_SUPERCEDE;
}
