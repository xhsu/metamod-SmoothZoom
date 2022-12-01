export module Weapons;

export import <cstdint>;

export import CBase;
export import Task;

export enum ETaskTypes : uint64_t
{
	TASK_ZOOM = (1 << 0),
};

export extern "C++" Task Task_Zoom(CBasePlayer * pPlayer, int const iFOV, int iSpeed) noexcept;
