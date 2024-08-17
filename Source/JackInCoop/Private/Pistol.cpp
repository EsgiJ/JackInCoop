// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

APistol::APistol()
{
    MyPawn = GetPawnOwner();

    BaseDamage = 5.f;
    WeaponConfig.MaxAmmo = 20;
    WeaponConfig.AmmoPerClip = 10;
    WeaponConfig.RateOfFire = 120.f;
    WeaponConfig.TimeBetweenShots = WeaponConfig.TimeBetweenShots = 60 / WeaponConfig.RateOfFire;

    SetReplicates(true);
    SetReplicateMovement(true);
}
