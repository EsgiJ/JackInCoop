// Fill out your copyright notice in the Description page of Project Settings.


#include "Pistol.h"

APistol::APistol()
{
    MyPawn = GetPawnOwner();

    WeaponConfig.MaxAmmo = 20;
    WeaponConfig.AmmoPerClip = 10;
    WeaponConfig.RateOfFire = 60.f;

    SetReplicates(true);
    SetReplicateMovement(true);
}
