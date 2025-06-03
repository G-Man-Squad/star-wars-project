//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_weapon_baseblaster.h"
#include "c_weapon__stubs.h"
#include "materialsystem/imaterial.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS_IMPLEMENT( weapon_baseblaster, C_WeaponBaseBlaster );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponBaseBlaster, DT_WeaponBaseBlaster, CWeaponBaseBlaster )
	RecvPropFloat( RECVINFO(m_flBlaster_HeatCurrent) ),
	RecvPropFloat( RECVINFO(m_flBlaster_HeatMax) ),
	RecvPropFloat( RECVINFO(m_flBlaster_CoolingTime) ),
	RecvPropBool( RECVINFO(m_bBlaster_Cooling) ),
END_RECV_TABLE()