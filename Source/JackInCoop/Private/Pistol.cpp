// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

APistol::APistol()
{
    MyPawn = GetPawnOwner();

    BaseDamage = 10.f;
    WeaponConfig.MaxAmmo = 200;
    WeaponConfig.InitialClips = 5;
    WeaponConfig.AmmoPerClip = 40;
    WeaponConfig.RateOfFire = 180;
    WeaponConfig.TimeBetweenShots = WeaponConfig.TimeBetweenShots = 60 / WeaponConfig.RateOfFire;

    SetReplicates(true);
    SetReplicateMovement(true);
}
