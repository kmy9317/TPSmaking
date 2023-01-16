// Fill out your copyright notice in the Description page of Project Settings.


#include "StatOverlay.h"
#include "Components/ProgressBar.h"

void UStatOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void UStatOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}