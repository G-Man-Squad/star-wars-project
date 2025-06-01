//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		357 - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "weapon_baseblaster.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_plr_dmg_crossbow;
extern ConVar sk_npc_dmg_crossbow;

LINK_ENTITY_TO_CLASS( blaster_bolt, CBaseBlasterBolt );

BEGIN_DATADESC( CBaseBlasterBolt )
	// Function Pointers
	DEFINE_FUNCTION( BubbleThink ),
	DEFINE_FUNCTION( BoltTouch ),

	// These are recreated on reload, they don't need storage
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

CBaseBlasterBolt *CBaseBlasterBolt::BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner )
{
	// Create a new entity with CBaseBlasterBolt private data
	CBaseBlasterBolt *pBolt = (CBaseBlasterBolt *)CreateEntityByName( "blaster_bolt" );
	UTIL_SetOrigin( pBolt, vecOrigin );
	pBolt->SetAbsAngles( angAngles );
	pBolt->Spawn();
	pBolt->SetOwnerEntity( pentOwner );

	return pBolt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseBlasterBolt::~CBaseBlasterBolt( void )
{
	if ( m_pGlowTrail )
	{
		UTIL_Remove( m_pGlowTrail );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseBlasterBolt::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CBaseBlasterBolt::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseBlasterBolt::CreateSprites( void )
{
	// Start up the eye glow
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate("sprites/bluelaser1.vmt", GetLocalOrigin(), false);

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 8.0f );
		m_pGlowTrail->SetEndWidth( 8.0f );
		m_pGlowTrail->SetLifeTime( 0.05f );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseBlasterBolt::Spawn( void )
{
	Precache( );

	SetModel( "models/crossbow_bolt.mdl" );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(0.3f,0.3f,0.3f), Vector(0.3f,0.3f,0.3f) );
	SetSolid( SOLID_BBOX );
	SetGravity( 0.05f );
	SetRenderMode( kRenderTransColor );
	SetRenderColor( 0, 0, 0, 0 );
	
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CBaseBlasterBolt::BoltTouch );

	SetThink( &CBaseBlasterBolt::BubbleThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
	
	CreateSprites();
}


void CBaseBlasterBolt::Precache( void )
{
	PrecacheModel( "models/crossbow_bolt.mdl" );

	PrecacheModel( "sprites/bluelaser1.vmt" );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CBaseBlasterBolt::BoltTouch( CBaseEntity *pOther )
{
	if ( pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER) )
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ( ( pOther->m_takedamage == DAMAGE_NO ) || ( pOther->m_takedamage == DAMAGE_EVENTS_ONLY ) )
			return;
	}

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		// Allow crossbow bolts to pass through her bounding box when an ally is crouched in front of the player
		// (the player thinks they have clear line of sight because their ally is crouching, but the ally's BBOx is still full-height and blocks bolts.)
		if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->Classify() == CLASS_PLAYER_ALLY_VITAL )
		{
			// Change the owner to stop further collisions with the ally. We do this by making them the owner.
			// The player won't get credit for this kill but at least the bolt won't magically disappear!
			SetOwnerEntity( pOther );
			return;
		}

		if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC() )
		{
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_crossbow.GetFloat(), DMG_NEVERGIB );
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );

			CBasePlayer *pPlayer = ToBasePlayer( GetOwnerEntity() );
			if ( pPlayer )
			{
				gamestats->Event_WeaponHit( pPlayer, true, "weapon_crossbow", dmgInfo );
			}

		}
		else
		{
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_crossbow.GetFloat(), DMG_BULLET | DMG_NEVERGIB );
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		if ( !pOther->IsAlive() )
		{
			// We killed it! 
			const surfacedata_t *pdata = physprops->GetSurfaceData( tr.surface.surfaceProps );
			if ( pdata->game.material == CHAR_TEX_GLASS )
			{
				return;
			}
		}

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		EmitSound( "Weapon_BaseBlaster.BoltHitBody" );

		SetTouch( NULL );
		SetThink( NULL );

		UTIL_Remove( this );
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
		{
			EmitSound( "Weapon_BaseBlaster.BoltHitWorld" );

			/*
			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize( vecDir );

			// See if we should reflect off this surface
			float hitDot = DotProduct( tr.plane.normal, -vecDir );
			
			if ( ( hitDot < 0.5f ) && ( speed > 100 ) )
			{
				Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;
				
				QAngle reflectAngles;

				VectorAngles( vReflection, reflectAngles );

				SetLocalAngles( reflectAngles );

				SetAbsVelocity( vReflection * speed * 0.75f );

				// Start to sink faster
				SetGravity( 1.0f );
			}
			else*/
			{
				SetTouch( NULL );
				SetThink( NULL );

				UTIL_Remove( this );
			}
			
			// Shoot some sparks
			if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
			{
				g_pEffects->Sparks( GetAbsOrigin() );
			}
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ( ( tr.surface.flags & SURF_SKY ) == false )
			{
				UTIL_ImpactTrace( &tr, DMG_BULLET );
			}

			UTIL_Remove( this );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseBlasterBolt::BubbleThink( void )
{
	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime + 0.1f );

	// Make danger sounds out in front of me, to scare snipers back into their hole
	CSoundEnt::InsertSound( SOUND_DANGER_SNIPERONLY, GetAbsOrigin() + GetAbsVelocity() * 0.2, 120.0f, 0.5f, this, SOUNDENT_CHANNEL_REPEATED_DANGER );

	if ( GetWaterLevel()  == 0 )
		return;

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}

LINK_ENTITY_TO_CLASS( weapon_baseblaster, CWeaponBaseBlaster );

PRECACHE_WEAPON_REGISTER( weapon_baseblaster );

IMPLEMENT_SERVERCLASS_ST( CWeaponBaseBlaster, DT_WeaponBaseBlaster )
END_SEND_TABLE()

BEGIN_DATADESC( CWeaponBaseBlaster )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponBaseBlaster::CWeaponBaseBlaster( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;

	m_flBlaster_FireRate = 0.35f; // How much delay between shots
	m_flBlaster_Kickback = 1.0f; // How much kickback (default 1.0f)
	m_flBlaster_Autoaim = 1.0f; // How much autoaim (default 1.0f)

	m_flBlaster_HeatCurrent = 0.0f;
	m_flBlaster_HeatAdd = 0.1f; // How much heat to add each shot
	m_flBlaster_HeatSub = 0.05f; // How much heat to subtract when cooling
	m_flBlaster_HeatMax = 1.0f; // How much heat can the blaster handle
	m_flBlaster_HeatTime = 3.0f; // How long the blaster waits before cooling

	m_bBlaster_Cooling = false;
	m_flBlaster_CoolingTime = 0.0f; // How long the blaster waits before cooling
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBaseBlaster::Precache( void )
{
	UTIL_PrecacheOther( "blaster_bolt" );

	PrecacheScriptSound( "Weapon_BaseBlaster.BoltHitBody" );
	PrecacheScriptSound( "Weapon_BaseBlaster.BoltHitWorld" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBaseBlaster::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	switch( pEvent->event )
	{
		case EVENT_WEAPON_RELOAD:
			{
				// Add Smoke Effect
				/*
				CEffectData data;

				// Emit six spent shells
				for ( int i = 0; i < 6; i++ )
				{
					data.m_vOrigin = pOwner->WorldSpaceCenter() + RandomVector( -4, 4 );
					data.m_vAngles = QAngle( 90, random->RandomInt( 0, 360 ), 0 );
					data.m_nEntIndex = entindex();

					DispatchEffect( "ShellEject", data );
				}
				*/
				break;
			}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponBaseBlaster::PrimaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if ( BlasterIsCoolingOff() )
		return;

	pOwner->RumbleEffect( RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART );

	Vector vecAiming	= pOwner->GetAutoaimVector( 1.0f );
	Vector vecSrc		= pOwner->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles( vecAiming, angAiming );

	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * 24.0f, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );

	if( tr.m_pEnt != NULL && tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY_VITAL )
	{
		// If Alyx is right in front of the player, make sure the bolt starts outside of the player's BBOX, or the bolt
		// will instantly collide with the player after the owner of the bolt is switched to Alyx in ::BoltTouch(). We 
		// avoid this altogether by making it impossible for the bolt to collide with the player.
		vecSrc += vecAiming * 24.0f;
	}
	
	Vector	vForward, vRight, vUp;
	pOwner->EyeVectors( &vForward, &vRight, &vUp );
	Vector vecMuzzle	= vecSrc + vForward * 2.0f + vRight * 6.0f + vUp * -4.0f;

	CBaseBlasterBolt *pBolt = CBaseBlasterBolt::BoltCreate( vecSrc, angAiming, pOwner );
	pBolt->SetAbsVelocity( vecAiming * 2500 );

	pOwner->ViewPunch( QAngle( -2, 0, 0 ) * BlasterGetKickback() );

	WeaponSound( SINGLE );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 200, 0.2 );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	if ( !m_iClip1 && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	BlasterHeatingChange( BlasterGetHeatingAdd() );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack	= gpGlobals->curtime + GetFireRate();
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBaseBlaster::ItemPostFrame( void )
{
	if ( BlasterIsCoolingOff() && gpGlobals->curtime >= m_flBlaster_CoolingTime )
	{
		if ( BlasterGetHeatingAmount() >= BlasterGetHeatingMax() )
			WeaponSound( RELOAD );

		if ( BlasterGetHeatingAmount() > 0.0f )
		{
			BlasterHeatingChange( -BlasterGetHeatingSub() );
		}
		else
		{
			BlasterSetIsCoolingOff( false );
		}
	}
	else if ( BlasterGetHeatingAmount() >= BlasterGetHeatingMax() )
	{
		if ( !BlasterIsCoolingOff() )
		{
			BlasterSetIsCoolingOff( true );
			m_flBlaster_CoolingTime = gpGlobals->curtime + BlasterGetHeatingTime();
		}
	}

	BaseClass::ItemPostFrame();
}

void CWeaponBaseBlaster::BlasterHeatingChange( float a )
{
	m_flBlaster_HeatCurrent += a;
	if( m_flBlaster_HeatCurrent > m_flBlaster_HeatMax )
		m_flBlaster_HeatCurrent = m_flBlaster_HeatMax;
	if( m_flBlaster_HeatCurrent < 0.0f )
		m_flBlaster_HeatCurrent = 0.0f;
}
