//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "c_basehlcombatweapon.h"

#ifndef C_WEAPON_BASEBLASTER_H
#define C_WEAPON_BASEBLASTER_H
#ifdef _WIN32
#pragma once
#endif

class C_WeaponBaseBlaster : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_WeaponBaseBlaster, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	float GetHeatCurrent( void ) { return m_flBlaster_HeatCurrent; }
	float GetHeatMax( void ) { return m_flBlaster_HeatMax; }
	float GetCoolTime( void ) { return m_flBlaster_CoolingTime; }
	bool IsCooling( void ) { return m_bBlaster_Cooling; }
	
private:
	CNetworkVar( float, m_flBlaster_HeatCurrent );
	CNetworkVar( float, m_flBlaster_HeatMax );
	CNetworkVar( float, m_flBlaster_CoolingTime );
	CNetworkVar( bool,	m_bBlaster_Cooling );
};

#endif // C_BASEHLCOMBATWEAPON_H
