#include <cassert>

import <array>;
import <string>;

import Hook;
import Plugin;
import Task;

import UtlHook;

using std::array;

// Weapons.cpp
extern void __fastcall HamF_Entity_Spawn(CBasePlayerWeapon *pThis, int) noexcept;
extern void __fastcall HamF_Weapon_SecondaryAttack(CBasePlayerWeapon *pThis, int) noexcept;
//

void DeployHooks(void) noexcept
{
	static bool bHooksPerformed = false;

	[[likely]]
	if (bHooksPerformed)
		return;

	for (auto &[psz, pfnSpawn, pfnSecondaryAttack] : g_rgWeaponSecondaryAttackHooks)
	{
		edict_t *pEnt = g_engfuncs.pfnCreateNamedEntity(MAKE_STRING(psz));

#ifdef _DEBUG
		assert(pEnt && pEnt->pvPrivateData);
#else
		if (!pEnt || !pEnt->pvPrivateData) [[unlikely]]
		{
			if (pEnt)
				g_engfuncs.pfnRemoveEntity(pEnt);

			gpMetaUtilFuncs->pfnLogError(PLID, "Failed to retrieve classtype for \"%s\".", psz);
			return;
		}
#endif

		auto const rgpfn = UTIL_RetrieveVirtualFunctionTable(pEnt->pvPrivateData);

		UTIL_VirtualTableInjection(rgpfn, VFTIDX_ENTITY_SPAWN, &HamF_Entity_Spawn, (void **)&pfnSpawn);
		UTIL_VirtualTableInjection(rgpfn, VFTIDX_WEAPON_SECONDARYATTACK, &HamF_Weapon_SecondaryAttack, (void **)&pfnSecondaryAttack);
	}

	bHooksPerformed = true;
}

void RegisterCVar(void) noexcept
{
	g_engfuncs.pfnCVarRegister(&gcvarZoomStep);
	g_engfuncs.pfnCVarRegister(&gcvarScrollStep);
	g_engfuncs.pfnCVarRegister(&gcvarCooldown);
}

// Meta API

extern META_RES OnClientCommand(CBasePlayer *pPlayer, const std::string &szCommand) noexcept;
void fw_ClientCommand(edict_t *pEdict) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	[[unlikely]]
	if (pev_valid(pEdict) != 2)
		return;

	if (auto const pEntity = (CBaseEntity *)pEdict->pvPrivateData; !pEntity->IsPlayer())
		return;

	gpMetaGlobals->mres = OnClientCommand((CBasePlayer *)pEdict->pvPrivateData, g_engfuncs.pfnCmd_Argv(0));
}

void fw_ServerActivate_Post(edict_t *pEdictList, int edictCount, int clientMax) noexcept
{
	gpMetaGlobals->mres = MRES_IGNORED;

	// plugin_init

	DeployHooks();

	// plugin_cfg
}

void fw_ServerDeactivate_Post(void) noexcept
{
	// Remove ALL existing tasks.
	TaskScheduler::Clear();

	/************ Regular Re-zero Actions ************/
}

// Register Meta Hooks

inline constexpr DLL_FUNCTIONS gFunctionTable =
{
	.pfnGameInit = nullptr,
	.pfnSpawn = nullptr,
	.pfnThink = nullptr,
	.pfnUse = nullptr,
	.pfnTouch = nullptr,
	.pfnBlocked = nullptr,
	.pfnKeyValue = nullptr,
	.pfnSave = nullptr,
	.pfnRestore = nullptr,
	.pfnSetAbsBox = nullptr,

	.pfnSaveWriteFields = nullptr,
	.pfnSaveReadFields = nullptr,

	.pfnSaveGlobalState = nullptr,
	.pfnRestoreGlobalState = nullptr,
	.pfnResetGlobalState = nullptr,

	.pfnClientConnect = nullptr,
	.pfnClientDisconnect = nullptr,
	.pfnClientKill = nullptr,
	.pfnClientPutInServer = nullptr,
	.pfnClientCommand = &fw_ClientCommand,
	.pfnClientUserInfoChanged = nullptr,
	.pfnServerActivate = nullptr,
	.pfnServerDeactivate = nullptr,

	.pfnPlayerPreThink = nullptr,
	.pfnPlayerPostThink = nullptr,

	.pfnStartFrame = nullptr,
	.pfnParmsNewLevel = nullptr,
	.pfnParmsChangeLevel = nullptr,

	.pfnGetGameDescription = nullptr,
	.pfnPlayerCustomization = nullptr,

	.pfnSpectatorConnect = nullptr,
	.pfnSpectatorDisconnect = nullptr,
	.pfnSpectatorThink = nullptr,

	.pfnSys_Error = nullptr,

	.pfnPM_Move = nullptr,
	.pfnPM_Init = nullptr,
	.pfnPM_FindTextureType = nullptr,

	.pfnSetupVisibility = nullptr,
	.pfnUpdateClientData = nullptr,
	.pfnAddToFullPack = nullptr,
	.pfnCreateBaseline = nullptr,
	.pfnRegisterEncoders = nullptr,
	.pfnGetWeaponData = nullptr,
	.pfnCmdStart = nullptr,
	.pfnCmdEnd = nullptr,
	.pfnConnectionlessPacket = nullptr,
	.pfnGetHullBounds = nullptr,
	.pfnCreateInstancedBaselines = nullptr,
	.pfnInconsistentFile = nullptr,
	.pfnAllowLagCompensation = nullptr,
};

qboolean HookGameDLLExportedFn(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLExportedFn' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(&gPluginInfo, "Function 'HookGameDLLExportedFn' called with version mismatch. Expected: %d, receiving: %d.", INTERFACE_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gFunctionTable, sizeof(DLL_FUNCTIONS));
	return true;
}

inline constexpr DLL_FUNCTIONS gFunctionTable_Post =
{
	.pfnGameInit = &RegisterCVar,
	.pfnSpawn = nullptr,
	.pfnThink = nullptr,
	.pfnUse = nullptr,
	.pfnTouch = nullptr,
	.pfnBlocked = nullptr,
	.pfnKeyValue = nullptr,
	.pfnSave = nullptr,
	.pfnRestore = nullptr,
	.pfnSetAbsBox = nullptr,

	.pfnSaveWriteFields = nullptr,
	.pfnSaveReadFields = nullptr,

	.pfnSaveGlobalState = nullptr,
	.pfnRestoreGlobalState = nullptr,
	.pfnResetGlobalState = nullptr,

	.pfnClientConnect = nullptr,
	.pfnClientDisconnect = nullptr,
	.pfnClientKill = nullptr,
	.pfnClientPutInServer = nullptr,
	.pfnClientCommand = nullptr,
	.pfnClientUserInfoChanged = nullptr,
	.pfnServerActivate = &fw_ServerActivate_Post,
	.pfnServerDeactivate = &fw_ServerDeactivate_Post,

	.pfnPlayerPreThink = nullptr,
	.pfnPlayerPostThink = nullptr,

	.pfnStartFrame = &TaskScheduler::Think,
	.pfnParmsNewLevel = nullptr,
	.pfnParmsChangeLevel = nullptr,

	.pfnGetGameDescription = nullptr,
	.pfnPlayerCustomization = nullptr,

	.pfnSpectatorConnect = nullptr,
	.pfnSpectatorDisconnect = nullptr,
	.pfnSpectatorThink = nullptr,

	.pfnSys_Error = nullptr,

	.pfnPM_Move = nullptr,
	.pfnPM_Init = nullptr,
	.pfnPM_FindTextureType = nullptr,

	.pfnSetupVisibility = nullptr,
	.pfnUpdateClientData = nullptr,
	.pfnAddToFullPack = nullptr,
	.pfnCreateBaseline = nullptr,
	.pfnRegisterEncoders = nullptr,
	.pfnGetWeaponData = nullptr,
	.pfnCmdStart = nullptr,
	.pfnCmdEnd = nullptr,
	.pfnConnectionlessPacket = nullptr,
	.pfnGetHullBounds = nullptr,
	.pfnCreateInstancedBaselines = nullptr,
	.pfnInconsistentFile = nullptr,
	.pfnAllowLagCompensation = nullptr,
};

qboolean HookGameDLLExportedFn_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion) noexcept
{
	if (!pFunctionTable) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookGameDLLExportedFn_Post' called with null 'pFunctionTable' parameter.");
		return false;
	}
	else if (*interfaceVersion != INTERFACE_VERSION) [[unlikely]]
	{
		gpMetaUtilFuncs->pfnLogError(PLID, "Function 'HookGameDLLExportedFn_Post' called with version mismatch. Expected: %d, receiving: %d.", INTERFACE_VERSION, *interfaceVersion);

		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy(pFunctionTable, &gFunctionTable_Post, sizeof(DLL_FUNCTIONS));
	return true;
}
