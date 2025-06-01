//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_BASEBLASTER_H
#define WEAPON_BASEBLASTER_H

#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "SpriteTrail.h"
#include "npcevent.h"

//-----------------------------------------------------------------------------
class CBaseBlasterBolt : public CBaseCombatCharacter
{
	DECLARE_CLASS( CBaseBlasterBolt, CBaseCombatCharacter );

public:
	CBaseBlasterBolt() { };
	~CBaseBlasterBolt();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CBaseBlasterBolt *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL );

protected:

	bool	CreateSprites( void );

	CHandle<CSpriteTrail>	m_pGlowTrail;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
// CWeaponBaseBlaster
//-----------------------------------------------------------------------------

class CWeaponBaseBlaster : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponBaseBlaster, CBaseHLCombatWeapon);
public:

	CWeaponBaseBlaster(void);

	void	PrimaryAttack(void);
	virtual void	Precache( void );
	virtual void	ItemPostFrame( void );
	void	Operator_HandleAnimEvent( animevent_t* pEvent, CBaseCombatCharacter* pOperator );
	
	virtual float GetFireRate( void ) { return m_flBlaster_FireRate; };
	float	WeaponAutoAimScale() { return m_flBlaster_Autoaim; }

	float	BlasterGetHeatingAmount() { return m_flBlaster_HeatCurrent; }
	float	BlasterGetHeatingAdd() { return m_flBlaster_HeatAdd; }
	float	BlasterGetHeatingSub() { return m_flBlaster_HeatSub; }
	float	BlasterGetHeatingMax() { return m_flBlaster_HeatMax; }
	float	BlasterGetHeatingTime() { return m_flBlaster_HeatTime; }
	float	BlasterGetKickback() { return m_flBlaster_Kickback; }
	bool	BlasterIsCoolingOff() { return m_bBlaster_Cooling; }
	void	BlasterHeatingChange( float a );
	void	BlasterSetIsCoolingOff( bool a ) { m_bBlaster_Cooling = a; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

protected:

	float				m_flBlaster_HeatCurrent;
	float				m_flBlaster_HeatAdd;
	float				m_flBlaster_HeatSub;
	float				m_flBlaster_HeatMax;
	float				m_flBlaster_HeatTime;
	bool				m_bBlaster_Cooling;
	float				m_flBlaster_FireRate;
	float				m_flBlaster_Kickback;
	float				m_flBlaster_Autoaim;

	float				m_flBlaster_CoolingTime;
};

#endif // WEAPON_RPG_H
