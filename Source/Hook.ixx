export module Hook;

export import <array>;

import cvardef;

export import CBase;

// VFT indeces
export inline constexpr size_t VFTIDX_ENTITY_SPAWN = 0;
export inline constexpr size_t VFTIDX_WEAPON_SECONDARYATTACK = 88;

// VFT fn prototypes
export using fnSpawn_t = void(__thiscall *)(CBasePlayerWeapon *) noexcept;
export using fnWeaponSecondaryAttack_t = void(__thiscall *)(CBasePlayerWeapon *) noexcept;

// CVars #INVESTIGATE why can't I just use these but have to get it back from engine??
export inline cvar_t gcvarZoomStep = { "smoothzoom_speed", "2", FCVAR_EXTDLL, 2.f, nullptr };
export inline cvar_t gcvarScrollStep = { "smoothzoom_scrollstep", "3", FCVAR_EXTDLL, 3.f, nullptr };
export inline cvar_t gcvarCooldown = { "smoothzoom_cooldown", "0.3", FCVAR_EXTDLL, 0.3f, nullptr };

export struct WpnHook_t
{
	const char *m_pszClassName{};
	fnSpawn_t m_pfnSpawn{};
	fnWeaponSecondaryAttack_t m_pfnSecondaryAttack{};
};

export enum EHookedWpn
{
	IDX_AUG,
	IDX_SG552,
	IDX_SCOUT,
	IDX_SG550,
	IDX_G3SG1,
	IDX_AWP,

	EHookedWpn_Count
};

export inline std::array<WpnHook_t, EHookedWpn_Count> g_rgWeaponSecondaryAttackHooks =
{ {
	{"weapon_aug", nullptr},
	{"weapon_sg552", nullptr},
	{"weapon_scout", nullptr},
	{"weapon_sg550", nullptr},
	{"weapon_g3sg1", nullptr},
	{"weapon_awp", nullptr},
} };
