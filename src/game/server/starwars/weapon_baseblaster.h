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

enum BlasterBoltType_E
{
	BLASTER_NORMAL = 0,
	BLASTER_SHIP,
	BLASTER_CANNON,
};

enum BlasterBoltClr_E
{
	BLASTER_CLR_RED = 0,
	BLASTER_CLR_GREEN,
	BLASTER_CLR_BLUE,
};

const char* g_pzsBlasterBoltTexture[3][3] =
{
	{ "sprites/blaster/redlaser1.vmt", "sprites/blaster/redlaser2.vmt", "sprites/blaster/redlaser3.vmt" },
	{ "sprites/blaster/greenlaser1.vmt", "sprites/blaster/greenlaser2.vmt", "sprites/blaster/greenlaser3.vmt" },
	{ "sprites/blaster/bluelaser1.vmt", "sprites/blaster/bluelaser2.vmt", "sprites/blaster/bluelaser3.vmt" },
};

//-----------------------------------------------------------------------------
class CBaseBlasterBolt : public CBaseCombatCharacter
{
	DECLARE_CLASS( CBaseBlasterBolt, CBaseCombatCharacter );

public:
	CBaseBlasterBolt();
	~CBaseBlasterBolt();

	Class_T Classify( void ) { return CLASS_NONE; }

	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );
	void BoltTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CBaseBlasterBolt *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, CBaseCombatCharacter *pentOwner = NULL, int iDmg = 8, int iType = 0, int iClr = 0 );

protected:

	bool	CreateSprites( void );

	int		m_iBoltDmg;
	int		m_iBoltType;
	int		m_iBoltColor;

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

	void	FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void	FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void	Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	
	virtual float GetFireRate( void ) { return m_flBlaster_FireRate; };
	float	WeaponAutoAimScale() { return m_flBlaster_Autoaim; }

	void	BlasterHeatingChange( float a );
	void	BlasterSetIsCoolingOff( bool a ) { m_bBlaster_Cooling = a; }

	DECLARE_SERVERCLASS();

protected:

	float				m_flBlaster_FireRate;
	float				m_flBlaster_Kickback;
	float				m_flBlaster_Autoaim;
	float				m_flBlaster_LastShot;

	float				m_flBlaster_HeatAdd;
	float				m_flBlaster_HeatSub;
	float				m_flBlaster_HeatTime;

	int					m_iBlaster_BoltSpeed;
	int					m_iBlaster_BoltDmg;
	int					m_iBlaster_BoltType;
	int					m_iBlaster_BoltClr;

	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
	
private:
	CNetworkVar( float, m_flBlaster_HeatCurrent );
	CNetworkVar( float, m_flBlaster_HeatMax );
	CNetworkVar( float, m_flBlaster_CoolingTime );
	CNetworkVar( bool,	m_bBlaster_Cooling );
};

#endif // WEAPON_RPG_H
