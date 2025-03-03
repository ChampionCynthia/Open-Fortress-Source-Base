// ========= Copyright Open Fortress Developers, CC-BY-NC-SA ============
// Purpose: Impl COFWeaponBase, the root of all OF weapons
// Author(s): Nopey, Fenteale
//
#include "cbase.h"
#include "of_shareddefs.h"
#ifdef CLIENT_DLL
#include "c_of_player.h"
#else
#include "of_player.h"
#endif
#include "of_weapon_base_gun.h"
#include "of_fx_shared.h"

// ----------------------------------------------------------------------------- //
// COFWeaponBase tables.
// ----------------------------------------------------------------------------- //

IMPLEMENT_NETWORKCLASS_ALIASED( OFWeaponBaseGun, DT_OFWeaponBaseGun )

BEGIN_NETWORK_TABLE( COFWeaponBaseGun, DT_OFWeaponBaseGun )
#ifdef CLIENT_DLL
  
#else
/* SDK Garbo that might or might not be useful
    // world weapon models have no animations
      SendPropExclude( "DT_AnimTimeMustBeFirst", "m_flAnimTime" ),
    SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
*/
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( COFWeaponBaseGun )

END_PREDICTION_DATA()
#endif

// Do we need this? Weapon bases never get used on their own - Kay
LINK_ENTITY_TO_CLASS( tf_weapon_base_gun, COFWeaponBaseGun );


#ifdef GAME_DLL

    BEGIN_DATADESC( COFWeaponBaseGun )

        // New weapon Think and Touch Functions go here..

    END_DATADESC()

#endif

COFWeaponBaseGun::COFWeaponBaseGun() {}
COFWeaponBaseGun::~COFWeaponBaseGun() {}

void COFWeaponBaseGun::PrimaryAttack()
{
	// Something with UberChargeAmmoPerShot(), im not sure exactly what this means
	// (*this->vtable->CTFWeaponBase::UberChargeAmmoPerShot)((CTFWeaponBase *)this); - Kay
	
	// Abort if we dont have any reserve ammo
	if( /* 0 < (float)extraout_ST0 Checking reserve ammo?, but wouldnt that be >= then? - Kay */ 
	!HasPrimaryAmmo() )
		return;
	/*
	Since we check reserve ammo previously, im gonna assume that this checks if we have anything in the clip left
	aka that (int)this[0x17b].vtable is the clip size - Kay
	  if (((int)this[0x17b].vtable < 1) && (this[0x17b].vtable != (vtableCTFWeaponBaseGun *)0xffffffff))
	  {
		return;
	  }	
	*/
	
	// Abort if we have less than 1 bullet in the clip
	if( Clip1() < 1 )
		return;
	
	/*
		Im honestly not exactly sure but from what i can tell, this is the Fire time,
		so its checking against the CURRTIME ˇˇˇˇ to see if we can fire now - Kay
	  if (*(float *)(*(int *)PTR__gpGlobals_00e31080 + 0xc) <= (float)this[0x16a].vtable &&
		  (float)this[0x16a].vtable != *(float *)(*(int *)PTR__gpGlobals_00e31080 + 0xc)) {
		return;
	  }	
	*/
	
	// Dont shoot faster than the fire rate
	if( gpGlobals->curtime <= m_flNextPrimaryAttack )
		return;
	
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if( !pPlayer )
		return;
	
/*
  cVar7 = (*this_01->vtable->CBaseEntity::IsPlayer)(this_01);
  if (cVar7 == '\0') {
    return;
  }
  Skipped IsPlayer since GetTFPlayerOwner already does that for us
*/

	if( !CanAttack() )
		return;

	// OFTODO: Replace this with CTFWeaponBase::ApplyFireDelay whenever that function gets added - Kay
	m_flNextPrimaryAttack += GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flTimeFireDelay;
	DevMsg("Next attack %f\n", GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flTimeFireDelay);
	
	m_iClip1 -= GetAmmoPerShot();
	
	DoFireEffects();
	
//	CalcIsAttackCritical();
	
	/* Uncomment these when we have the functions for it
	if( ShouldRemoveInvisibilityOnPrimaryAttack() )
		pPlayer->RemoveInvisibility(); 
	
	pPlayer->SpeakWeaponFire();
	
	globalTFStatsThing->Event_PlayerFiredWeapon(pPlayer, Some bool, not sure exactly); -Kay	*/
	
	SendWeaponAnim( GetPrimaryAttackActivity() );
	
	// Uncomment when we implement these -Kay
	// CBaseEntity *pProjectile =
	FireProjectile( pPlayer );
	
	m_iReloadStage.Set( OF_RELOAD_STAGE_NONE );	
	
	//ModifyProjectile( pProjectile );

	// (*this_01->vtable[1].CBaseEntity::NetworkStateChanged_m_hGroundEntity)(this_01,5); Whats this used for?? We like, change our ground state or what? -Kay
	
	/*
	Everything beyond here is just French to me, if someone else can decypther it go ahead 
	By the looks of it its somethign todo with the animation playback speed - Kay
	if( !UsesClipsForAmmo1() )
	{
		
	}
	*/

	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	
/*	Uncomment when we have these functions
	if( ShouldRemoveDisguiseOnPrimaryAttack() )
		pPlayer->RemoveDisguise(); */
	
//	pPlayer->OnAttack();
}

// OFSTATUS: INCOMPLETE, only supports FireBullet for now, and some functions are missing at the end
CBaseEntity *COFWeaponBaseGun::FireProjectile( COFPlayer *pPlayer )
{
	CBaseEntity *pProjectile = NULL;
	int iProjectileType = GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_iProjectileType;
	
	switch( iProjectileType )
	{
		case OF_PROJECTILE_TYPE_BULLET:
			FireBullet( pPlayer );
			break;
/*		case OF_PROJECTILE_TYPE_ROCKET:
		case OF_PROJECTILE_TYPE_MANGLER:
			pProjectile = FireRocket();
			break;
		case OF_PROJECTILE_TYPE_FLAMEROCKET:
			pProjectile = FireFlameRocket();
			break;
		case OF_PROJECTILE_TYPE_PIPEBOMB:
		case OF_PROJECTILE_TYPE_STICKYBOMB:
		case OF_PROJECTILE_TYPE_STICKYJUMPER:
		case OF_PROJECTILE_TYPE_CANNONBALL:
			pProjectile = FirePipeBomb();
			break;
		case OF_PROJECTILE_TYPE_SYRINGE:
			pProjectile = FireNail();
			break;
		case OF_PROJECTILE_TYPE_FLARE:
			pProjectile = FireFlare();
			break;
		case OF_PROJECTILE_TYPE_JARATE:
		case OF_PROJECTILE_TYPE_MADMILK:
		case OF_PROJECTILE_TYPE_CLEAVER:
		case OF_PROJECTILE_TYPE_JARATE_FESTIVE:
		case OF_PROJECTILE_TYPE_CROSSBOW_FESTIVE:
		case OF_PROJECTILE_TYPE_JARATE_BREAD:
		case OF_PROJECTILE_TYPE_MADMILK_BREAD:
			pProjectile = FireJar();
			break;
		case OF_PROJECTILE_TYPE_ARROW:
		case OF_PROJECTILE_TYPE_CROSSBOW:
		case OF_PROJECTILE_TYPE_RANGER:
		case OF_PROJECTILE_TYPE_ARROW_FESTIVE:
		case OF_PROJECTILE_TYPE_GRAPPLE:
			pProjectile = FireArrow();
			break;
		case OF_PROJECTILE_TYPE_BISON:
			pProjectile = FireEnergyBall();
			break;*/
		default:
		case OF_PROJECTILE_TYPE_NONE:
		case OF_PROJECTILE_TYPE_UNKNOWN1:
		case OF_PROJECTILE_TYPE_UNKNOWN2:
		case OF_PROJECTILE_TYPE_UNKNOWN3:
			DevMsg("Weapon does not have a projectile type set\n");
			break;
	}
	
	if( ShouldPlayFireAnim() )
		pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
	
	return pProjectile;
}

// OFSTATUS: ~50% comlpete, there's some functions before FX_FireBullets 
void COFWeaponBaseGun::FireBullet( COFPlayer *pPlayer )
{
	PlayWeaponShootSound();
	
	FX_FireBullets(
		this,
		GetOwner()->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->EyeAngles() + pPlayer->GetPunchAngle(),
		GetWeaponID(),
		m_iWeaponMode,
		CBaseEntity::GetPredictionRandomSeed() & 255,
		GetWeaponSpread(),
		GetProjectileDamage(),
		false);
}

//OFSTATUS: INCOMPLETE
int COFWeaponBaseGun::GetAmmoPerShot()
{
	return GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_iAmmoPerShot;
}

//OFSTATUS: COMPLETE
void COFWeaponBaseGun::DoFireEffects()
{
	// From what i can tell this just casts to the player's active weapon and checks if it can do the muzzleflash
	// Question is, WHY? We're literally in the weapon itself, half of this is useless, so i just did the check the sane way - Kay
	COFPlayer *pPlayer = GetOFPlayerOwner();
	if( !pPlayer )
		return;

	if( ShouldDoMuzzleFlash() )
		pPlayer->DoMuzzleFlash();
}

//OFSTATUS: INCOMPLETE
float COFWeaponBaseGun::GetWeaponSpread()
{
	return GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_flSpread;
}

//OFSTATUS: INCOMPLETE
int COFWeaponBaseGun::GetProjectileDamage()
{
	return GetOFWpnData().m_WeaponModeInfo[m_iWeaponMode].m_iDamage;
};

//OFSTATUS: INCOMPLETE, DoAnimationEvent seems to cause crashes, so we disable this for now
bool COFWeaponBaseGun::ShouldPlayFireAnim()
{
	return false;
};

//OFSTATUS: INCOMPLETE
void COFWeaponBaseGun::PlayWeaponShootSound()
{
	// Default sound is single
	WeaponSound_t iSound = SINGLE;

	// Check if the current attack is a critical, and if so, play the burst ( crit ) sound
	// OFTODO: Replace this with a variable or function that checks if we're critting
	if( false )
		iSound = BURST;
	
	WeaponSound(iSound);
}

//OFSTATUS: INCOMPLETE
bool COFWeaponBaseGun::ShouldRemoveDisguiseOnPrimaryAttack() const
{
	return true;
};