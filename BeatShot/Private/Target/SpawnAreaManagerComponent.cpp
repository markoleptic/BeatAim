// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Target/SpawnAreaManagerComponent.h"
#include <stack>
#include "Algo/RandomShuffle.h"
#include "Target/MatrixFunctions.h"
#include "Target/SpawnArea.h"
#include "Target/Target.h"
#if !UE_BUILD_SHIPPING
#include "Target/TargetManager.h"
#endif

void FRectCandidate::MergeSubRectangles()
{
	if (SubRectangles.IsEmpty())
	{
		return;
	}

	TArray<FSubRectangle> SortedSubRectangles = SubRectangles.Array();
	SortedSubRectangles.Sort();

	if (SubRectangles.Num() > 1)
	{
		TArray<FSubRectangle> FilteredArray;
		int CurrentIndex = 1;
		FIndexPair LastStartEndIndex = SortedSubRectangles[0].BoundingIndices;

		if (LastStartEndIndex.EndIndex != SortedSubRectangles[1].BoundingIndices.EndIndex)
		{
			FilteredArray.Add(SortedSubRectangles[0]);
		}

		while (CurrentIndex < SortedSubRectangles.Num())
		{
			const FIndexPair CurrentStartEndIndex = SortedSubRectangles[CurrentIndex].BoundingIndices;
			if (LastStartEndIndex.EndIndex == CurrentStartEndIndex.EndIndex)
			{
				if (CurrentStartEndIndex.StartIndex != (LastStartEndIndex.StartIndex + 1))
				{
					FilteredArray.Add(SortedSubRectangles[CurrentIndex]);
				}
			}
			else
			{
				FilteredArray.Add(SortedSubRectangles[CurrentIndex]);
			}
			LastStartEndIndex = CurrentStartEndIndex;
			CurrentIndex += 1;
		}

		if (!FilteredArray.IsEmpty())
		{
			SortedSubRectangles = MoveTemp(FilteredArray);
		}
	}

	SubRectangles = TSet<FSubRectangle, FFSubRectangleKeyFuncs>(MoveTemp(SortedSubRectangles));
}

USpawnAreaManagerComponent::USpawnAreaManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

#if !UE_BUILD_SHIPPING
	bShowDebug_AllSpawnAreas = false;
	bShowDebug_ValidInvalidSpawnAreas = false;
	bPrintDebug_SpawnAreaStateInfo = false;
	bShowDebug_SpawnableSpawnAreas = false;
	bShowDebug_ActivatableSpawnAreas = false;
	bShowDebug_ActivatedSpawnAreas = false;
	bShowDebug_DeactivatedSpawnAreas = false;
	bShowDebug_RecentSpawnAreas = false;
	bShowDebug_RemovedFromExtremaChange = false;
	ShowDebug_Vertices = 0;
	bPrintDebug_Grid = false;
	bShowDebug_NonAdjacent = false;
#endif

	BSConfig = nullptr;
	TotalSpawnAreaSize = FIntVector3();
	SpawnAreaDimensions = FIntVector3();
	Origin = FVector();
	StaticExtents = FVector();
	StaticExtrema = FExtrema();

	SpawnAreas = TSet<USpawnArea*>();
	AreaKeyMap = TMap<FAreaKey, USpawnArea*>();
	GuidMap = TMap<FGuid, USpawnArea*>();
	CachedExtrema = TSet<USpawnArea*>();
	CachedManaged = TSet<USpawnArea*>();
	CachedActivated = TSet<USpawnArea*>();
	CachedRecent = TSet<USpawnArea*>();
	RecentGridBlocks = TArray<TSet<USpawnArea*>>();

	MostRecentSpawnArea = nullptr;
	OriginSpawnArea = nullptr;
}

void USpawnAreaManagerComponent::DestroyComponent(bool bPromoteChildren)
{
	BSConfig.Reset();
	Super::DestroyComponent(bPromoteChildren);
}

FIntVector3 USpawnAreaManagerComponent::Init(const TSharedPtr<FBSConfig>& InConfig, const FVector& InOrigin,
	const FVector& InStaticExtents, const FExtrema& InStaticExtrema)
{
	Clear();

	BSConfig = InConfig;
	Origin = InOrigin;
	StaticExtents = InStaticExtents;
	StaticExtrema = InStaticExtrema;

	SetSpawnAreaDimensions();
	InitializeSpawnAreas();

	OriginSpawnArea = GetSpawnArea(Origin);

#if !UE_BUILD_SHIPPING
	if (!GIsAutomationTesting)
	{
		UE_LOG(LogTargetManager, Display, TEXT("Origin: %s "), *Origin.ToString());
		UE_LOG(LogTargetManager, Display, TEXT("Extents: %s "), *StaticExtents.ToString());
		UE_LOG(LogTargetManager, Display, TEXT("Extrema Min: %s Max: %s"), *StaticExtrema.Min.ToString(),
			*StaticExtrema.Max.ToString());
		UE_LOG(LogTargetManager, Display, TEXT("SpawnAreaInc: Y: %d Z: %d"), SpawnAreaDimensions.Y,
			SpawnAreaDimensions.Z);
		UE_LOG(LogTargetManager, Display, TEXT("Num Spawn Areas: %d Allocated Size: %llu"), SpawnAreas.Num(),
			SpawnAreas.GetAllocatedSize());
	}
#endif

	return SpawnAreaDimensions;
}

void USpawnAreaManagerComponent::SetSpawnAreaDimensions()
{
	const int32 HalfWidth = StaticExtents.Y;
	const int32 HalfHeight = StaticExtents.Z;

	switch (BSConfig->TargetConfig.TargetDistributionPolicy)
	{
	case ETargetDistributionPolicy::HeadshotHeightOnly:
		{
			SpawnAreaDimensions.Y = Constants::HeadshotHeight_VerticalSpread;
			SpawnAreaDimensions.Z = Constants::HeadshotHeight_VerticalSpread;
		}
		break;
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::EdgeOnly:
	case ETargetDistributionPolicy::FullRange:
		{
			bool bWidthScaleSelected = false;
			bool bHeightScaleSelected = false;
			for (const int32 Scale : PreferredSpawnAreaDimensions)
			{
				if (!bWidthScaleSelected)
				{
					if (HalfWidth % Scale == 0)
					{
						SpawnAreaDimensions.Y = Scale;
						bWidthScaleSelected = true;
					}
				}
				if (!bHeightScaleSelected)
				{
					if (HalfHeight % Scale == 0)
					{
						SpawnAreaDimensions.Z = Scale;
						bHeightScaleSelected = true;
					}
				}
				if (bHeightScaleSelected && bWidthScaleSelected)
				{
					break;
				}
			}
			if (!bWidthScaleSelected || !bHeightScaleSelected)
			{
				UE_LOG(LogTargetManager, Warning, TEXT("Couldn't Find Height/Width for StaticExtents: Y:%f Z:%f"),
					StaticExtents.Y, StaticExtents.Z);
				// Prevent breaking everything even though it won't encompass full Spawn Area
				SpawnAreaDimensions.Y = Constants::DefaultSpawnAreaDimension;
				SpawnAreaDimensions.Z = Constants::DefaultSpawnAreaDimension;
			}
		}
		break;
	case ETargetDistributionPolicy::Grid:
		{
			const float MaxTargetSize = BSConfig->TargetConfig.MaxSpawnedTargetScale * Constants::SphereTargetDiameter;
			SpawnAreaDimensions.Y = BSConfig->GridConfig.GridSpacing.X + MaxTargetSize;
			SpawnAreaDimensions.Z = BSConfig->GridConfig.GridSpacing.Y + MaxTargetSize;
		}
		break;
	}
}

void USpawnAreaManagerComponent::InitializeSpawnAreas()
{
	int SizeY = 0;
	int SizeZ = 0;
	int Index = 0;

	// Add an extra row and column if using grid
	const bool bGrid = TargetConfig().TargetDistributionPolicy == ETargetDistributionPolicy::Grid;

	const float MinY = StaticExtrema.Min.Y;
	const float MaxY = bGrid ? StaticExtrema.Max.Y + 1.f : StaticExtrema.Max.Y;
	const float MinZ = StaticExtrema.Min.Z;
	const float MaxZ = bGrid ? StaticExtrema.Max.Z + 1.f : StaticExtrema.Max.Z;

	const float TotalWidth = FMath::Abs(MaxY - MinY);
	const float TotalHeight = FMath::Abs(MaxZ - MinZ);

	TotalSpawnAreaSize.Y = FMath::CeilToInt32(TotalWidth / SpawnAreaDimensions.Y);
	TotalSpawnAreaSize.Z = FMath::CeilToInt32(TotalHeight / SpawnAreaDimensions.Z);
	const int32 TotalSize = TotalSpawnAreaSize.Y * TotalSpawnAreaSize.Z;

	USpawnArea::SetWidth(SpawnAreaDimensions.Y);
	USpawnArea::SetHeight(SpawnAreaDimensions.Z);
	USpawnArea::SetTotalNumHorizontalSpawnAreas(TotalSpawnAreaSize.Y);
	USpawnArea::SetTotalNumVerticalSpawnAreas(TotalSpawnAreaSize.Z);
	USpawnArea::SetSize(TotalSize);
	USpawnArea::SetMinDistanceBetweenTargets(TargetConfig().MinDistanceBetweenTargets);
	USpawnArea::SetTotalSpawnAreaExtrema(StaticExtrema);

	SpawnAreas.Reserve(TotalSize);
	AreaKeyMap.Reserve(TotalSize);

	for (float Z = MinZ; Z < MaxZ; Z += SpawnAreaDimensions.Z)
	{
		SizeY = 0;
		for (float Y = MinY; Y < MaxY; Y += SpawnAreaDimensions.Y)
		{
			const FVector Loc(Origin.X, Y, Z);
			const FSetElementId ID = SpawnAreas.Emplace(NewObject<USpawnArea>(this));

			SpawnAreas[ID]->Init(Index, Loc);
			AreaKeyMap.Add(FAreaKey(Loc, SpawnAreaDimensions), SpawnAreas[ID]);

			Index++;
			SizeY++;
		}
		SizeZ++;
	}

	ensure(TotalSpawnAreaSize.Y == SizeY);
	ensure(TotalSpawnAreaSize.Z == SizeZ);

	CachedExtrema = SpawnAreas;
}

void USpawnAreaManagerComponent::Clear()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	BSConfig.Reset();

	TotalSpawnAreaSize = FIntVector3();
	SpawnAreaDimensions = FIntVector3();
	Origin = FVector();
	StaticExtents = FVector();
	StaticExtrema = FExtrema();

	SpawnAreas.Empty();
	AreaKeyMap.Empty();
	GuidMap.Empty();
	CachedExtrema.Empty();
	CachedManaged.Empty();
	CachedActivated.Empty();
	CachedRecent.Empty();
	RecentGridBlocks = TArray<TSet<USpawnArea*>>();

	MostRecentSpawnArea = nullptr;
	OriginSpawnArea = nullptr;

	RequestRLCSpawnArea.Unbind();

#if !UE_BUILD_SHIPPING
	FlushPersistentDebugLines(GetWorld());
#endif
}

bool USpawnAreaManagerComponent::ShouldConsiderManagedAsInvalid() const
{
	return TargetConfig().TargetSpawningPolicy == ETargetSpawningPolicy::RuntimeOnly;
}

int32 USpawnAreaManagerComponent::GetMostRecentSpawnAreaIndex() const
{
	if (!MostRecentSpawnArea)
	{
		return -1;
	}
	return MostRecentSpawnArea->GetIndex();
}

int32 USpawnAreaManagerComponent::GetSpawnAreaIndex(const FGuid& TargetGuid) const
{
	const USpawnArea* SpawnArea = GetSpawnArea(TargetGuid);
	if (!SpawnArea)
	{
		return -1;
	}
	return SpawnArea->GetIndex();
}

void USpawnAreaManagerComponent::UpdateTotalTrackingDamagePossible(const FVector& InLocation) const
{
	if (USpawnArea* SpawnArea = GetSpawnArea(InLocation))
	{
		SpawnArea->IncrementTotalTrackingDamagePossible();
	}
}

void USpawnAreaManagerComponent::HandleTargetDamageEvent(const FTargetDamageEvent& DamageEvent)
{
	USpawnArea* SpawnArea = GetSpawnArea(DamageEvent.Guid);
	if (!SpawnArea)
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTargetManager, Warning, TEXT("Could not find SpawnArea from DamageEvent Guid."));
#endif
		return;
	}

	switch (DamageEvent.DamageType)
	{
	case ETargetDamageType::Tracking:
		{
			// Instead of using the spawn area where the target started, use the current location
			USpawnArea* SpawnAreaByLoc = GetSpawnArea(DamageEvent.Transform.GetLocation());
			if (!SpawnAreaByLoc)
			{
#if !UE_BUILD_SHIPPING
				UE_LOG(LogTargetManager, Warning, TEXT("Could not find SpawnArea from Transform: %s."),
					*DamageEvent.Transform.GetLocation().ToString());
#endif
				return;
			}

			// Total Tracking Damage Possible is done on tick in UpdateTotalTrackingDamagePossible

			// Only increment total tracking damage if damage came from player
			if (!DamageEvent.bDamagedSelf && DamageEvent.DamageDelta > 0.f)
			{
				SpawnAreaByLoc->IncrementTotalTrackingDamage();
			}
		}
		break;
	case ETargetDamageType::Hit:
		{
			// Always increment total spawns for Hit Damage Events
			SpawnArea->IncrementTotalSpawns();

			// Only increment total hits if damage came from player
			if (!DamageEvent.bDamagedSelf && DamageEvent.DamageDelta > 0.f)
			{
				SpawnArea->IncrementTotalHits();
			}
		}
		break;
	case ETargetDamageType::Self:
		{
			if (DamageEvent.VulnerableToDamageTypes.Contains(ETargetDamageType::Hit))
			{
				// Always increment total spawns for Hit Damage Events
				SpawnArea->IncrementTotalSpawns();
			}
			if (DamageEvent.VulnerableToDamageTypes.Contains(ETargetDamageType::Tracking))
			{
				// Nothing to do
			}
		}
		break;
	case ETargetDamageType::None:
	case ETargetDamageType::Combined:
		{
#if !UE_BUILD_SHIPPING
			UE_LOG(LogTargetManager, Warning, TEXT("DamageEvent with DamageType None or Combined."));
#endif
		}
		break;
	}

	// Applies to any damage type
	if (DamageEvent.bWillDeactivate || DamageEvent.bWillDestroy)
	{
		RemoveActivatedFlagFromSpawnArea(SpawnArea);
		FlagSpawnAreaAsRecent(SpawnArea);
	}

	if (DamageEvent.bWillDestroy)
	{
		RemoveManagedFlagFromSpawnArea(DamageEvent.Guid);
	}
}

void USpawnAreaManagerComponent::OnExtremaChanged(const FExtrema& Extrema)
{
	switch (TargetConfig().TargetDistributionPolicy)
	{
	case ETargetDistributionPolicy::None:
	case ETargetDistributionPolicy::Grid:
		return;
	case ETargetDistributionPolicy::EdgeOnly:
		{
			const float MaxY = Extrema.Max.Y - SpawnAreaDimensions.Y;
			const float MaxZ = Extrema.Max.Z - SpawnAreaDimensions.Z;

			const float MinY = Extrema.Min.Y;
			const float MinZ = Extrema.Min.Z;

			TSet<USpawnArea*> Temp;

			for (float Y = MinY; Y <= MaxY; Y += SpawnAreaDimensions.Y)
			{
				if (USpawnArea* SpawnArea_MinZ = GetSpawnArea(FVector(0, Y, MinZ)))
				{
					Temp.Add(SpawnArea_MinZ);
				}
				if (USpawnArea* SpawnArea_MaxZ = GetSpawnArea(FVector(0, Y, MaxZ)))
				{
					Temp.Add(SpawnArea_MaxZ);
				}
			}

			for (float Z = MinZ; Z <= MaxZ; Z += SpawnAreaDimensions.Z)
			{
				if (USpawnArea* SpawnArea_MinY = GetSpawnArea(FVector(0, MinY, Z)))
				{
					Temp.Add(SpawnArea_MinY);
				}
				if (USpawnArea* SpawnArea_MaxY = GetSpawnArea(FVector(0, MaxY, Z)))
				{
					Temp.Add(SpawnArea_MaxY);
				}
			}

			CachedExtrema = MoveTemp(Temp);
		}
		break;
	case ETargetDistributionPolicy::HeadshotHeightOnly:
	case ETargetDistributionPolicy::FullRange:
		{
			for (USpawnArea* SpawnArea : SpawnAreas)
			{
				const FVector Location = SpawnArea->GetBottomLeftVertex();
				if (Location.Y < Extrema.Min.Y || Location.Y >= Extrema.Max.Y || Location.Z < Extrema.Min.Z || Location.
					Z >= Extrema.Max.Z)
				{
					CachedExtrema.Remove(SpawnArea);
				}
				else
				{
					CachedExtrema.Add(SpawnArea);
				}
			}
		}
		break;
	}
}

/* ------------------------------- */
/* -- SpawnArea finders/getters -- */
/* ------------------------------- */

USpawnArea* USpawnAreaManagerComponent::GetSpawnArea(const int32 Index) const
{
	return IsSpawnAreaValid(Index) ? SpawnAreas[FSetElementId::FromInteger(Index)] : nullptr;
}

USpawnArea* USpawnAreaManagerComponent::GetSpawnArea(const FVector& InLocation) const
{
	// Adjust for the SpawnAreaInc being aligned to the BoxBounds Origin
	const FVector RelativeLocation = InLocation - Origin;

	// Snap to lowest SpawnAreaInc (49.9 -> 0 if SpawnAreaInc value is 50)
	int32 GridY = Origin.Y + SpawnAreaDimensions.Y * FMath::FloorToInt(RelativeLocation.Y / SpawnAreaDimensions.Y);
	int32 GridZ = Origin.Z + SpawnAreaDimensions.Z * FMath::FloorToInt(RelativeLocation.Z / SpawnAreaDimensions.Z);

	GridY = FMath::Clamp(GridY, StaticExtrema.Min.Y, StaticExtrema.Max.Y - SpawnAreaDimensions.Y);
	GridZ = FMath::Clamp(GridZ, StaticExtrema.Min.Z, StaticExtrema.Max.Z - SpawnAreaDimensions.Z);

	const auto Found = AreaKeyMap.Find(FAreaKey(FVector(0, GridY, GridZ), SpawnAreaDimensions));
	return Found ? *Found : nullptr;
}

USpawnArea* USpawnAreaManagerComponent::GetSpawnArea(const FGuid& TargetGuid) const
{
	const auto Found = GuidMap.Find(TargetGuid);
	return Found ? *Found : nullptr;
}

USpawnArea* USpawnAreaManagerComponent::GetOldestRecentSpawnArea() const
{
	TSet<USpawnArea*>&& RecentSpawnAreas = GetRecentSpawnAreas();

	if (RecentSpawnAreas.IsEmpty())
	{
		return nullptr;
	}

	USpawnArea* MostRecent = nullptr;

	for (USpawnArea* SpawnArea : RecentSpawnAreas)
	{
		if (!MostRecent)
		{
			MostRecent = SpawnArea;
			continue;
		}
		if (SpawnArea->GetTimeSetRecent() < MostRecent->GetTimeSetRecent())
		{
			MostRecent = SpawnArea;
		}
	}
	return MostRecent;
}

USpawnArea* USpawnAreaManagerComponent::GetOldestDeactivatedSpawnArea() const
{
	USpawnArea* MostRecent = nullptr;

	for (USpawnArea* SpawnArea : GetDeactivatedSpawnAreas())
	{
		if (!MostRecent)
		{
			MostRecent = SpawnArea;
			continue;
		}
		if (SpawnArea->GetTimeSetRecent() < MostRecent->GetTimeSetRecent())
		{
			MostRecent = SpawnArea;
		}
	}
	return MostRecent;
}

bool USpawnAreaManagerComponent::IsSpawnAreaValid(const int32 InIndex) const
{
	return InIndex >= 0 && InIndex < SpawnAreas.Num();
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetManagedSpawnAreas() const
{
	return CachedManaged;
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetDeactivatedSpawnAreas() const
{
	return CachedManaged.Difference(CachedActivated);
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetRecentSpawnAreas() const
{
	return CachedRecent;
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetActivatedSpawnAreas() const
{
	return CachedActivated;
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetActivatedOrRecentSpawnAreas() const
{
	return CachedActivated.Union(CachedRecent);
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetManagedActivatedOrRecentSpawnAreas() const
{
	return CachedManaged.Union(GetActivatedOrRecentSpawnAreas());
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetManagedDeactivatedNotRecentSpawnAreas() const
{
	return GetDeactivatedSpawnAreas().Difference(CachedRecent);
}

TSet<USpawnArea*> USpawnAreaManagerComponent::GetUnflaggedSpawnAreas() const
{
	return SpawnAreas.Difference(GetManagedActivatedOrRecentSpawnAreas());
}

/* ------------------------ */
/* -- SpawnArea flagging -- */
/* ------------------------ */

void USpawnAreaManagerComponent::FlagSpawnAreaAsManaged(const int32 SpawnAreaIndex, const FGuid TargetGuid)
{
	USpawnArea* SpawnArea = GetSpawnArea(SpawnAreaIndex);
	if (!SpawnArea)
	{
		return;
	}

	if (SpawnArea->IsManaged())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Tried to flag an already managed SpawnArea as managed."));
		return;
	}

	SpawnArea->SetGuid(TargetGuid);
	SpawnArea->SetIsManaged(true);

	// Add to caches and GuidMap
	GuidMap.Add(TargetGuid, SpawnArea);
	CachedManaged.Add(SpawnArea);
}

void USpawnAreaManagerComponent::FlagSpawnAreaAsActivated(const FGuid TargetGuid, const FVector& TargetScale)
{
	USpawnArea* SpawnArea = GetSpawnArea(TargetGuid);
	if (!SpawnArea)
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Failed to find target from Guid to activate."));
		return;
	}

	// Ignore already activated target that can be reactivated
	if (SpawnArea->CanActivateWhileActivated() && SpawnArea->IsActivated())
	{
		return;
	}

	// Should no longer be considered recent if activated
	if (SpawnArea->IsRecent())
	{
		RemoveRecentFlagFromSpawnArea(SpawnArea);
	}

	if (SpawnArea->IsActivated())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Tried to flag as Activated when already Activated."));
		return;
	}

	// Add to activated cache
	CachedActivated.Add(SpawnArea);

	// Set is activated
	SpawnArea->SetIsActivated(true, TargetConfig().bAllowActivationWhileActivated);

	// Set as the most recently activated SpawnArea
	SetMostRecentSpawnArea(SpawnArea);

	// Update scale
	SpawnArea->SetTargetScale(TargetScale);
}

void USpawnAreaManagerComponent::FlagSpawnAreaAsRecent(USpawnArea* SpawnArea)
{
	if (!SpawnArea)
	{
		return;
	}

	if (SpawnArea->IsRecent())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Tried to flag as Recent when already Recent."));
		return;
	}

	// Add to cache
	CachedRecent.Add(SpawnArea);

	SpawnArea->SetIsRecent(true);

	FTimerHandle TimerHandle;

	/** Handle removing recent flag from SpawnArea */
	switch (TargetConfig().RecentTargetMemoryPolicy)
	{
	case ERecentTargetMemoryPolicy::None:
		RemoveRecentFlagFromSpawnArea(SpawnArea);
		break;
	case ERecentTargetMemoryPolicy::CustomTimeBased:
		{
			RemoveFromRecentDelegate.BindUObject(this, &ThisClass::RemoveRecentFlagFromSpawnArea, SpawnArea);
			const float Time = TargetConfig().RecentTargetTimeLength;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, RemoveFromRecentDelegate, Time, false);
		}
		break;
	case ERecentTargetMemoryPolicy::NumTargetsBased:
		{
			RefreshRecentFlags();
		}
		break;
	case ERecentTargetMemoryPolicy::UseTargetSpawnCD:
		{
			RemoveFromRecentDelegate.BindUObject(this, &ThisClass::RemoveRecentFlagFromSpawnArea, SpawnArea);
			const float Time = TargetConfig().TargetSpawnCD;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, RemoveFromRecentDelegate, Time, false);
		}
		break;
	default:
		break;
	}
}

void USpawnAreaManagerComponent::RemoveManagedFlagFromSpawnArea(const FGuid TargetGuid)
{
	USpawnArea* SpawnArea = GetSpawnArea(TargetGuid);
	if (!SpawnArea)
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Failed to find target by Guid to remove from managed."));
		return;
	}

	const int32 NumRemoved = GuidMap.Remove(TargetGuid);
	const int32 NumRemovedManaged = CachedManaged.Remove(SpawnArea);

	if (!SpawnArea->IsManaged())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Tried to remove managed flag from from non-managed SpawnArea."));
		return;
	}

	if (NumRemoved == 0) UE_LOG(LogTargetManager, Warning, TEXT("Failed to remove from TargetGuidToSpawnArea map."));
	if (NumRemovedManaged == 0) UE_LOG(LogTargetManager, Warning, TEXT("Failed to remove from CachedManaged."));

	SpawnArea->SetIsManaged(false);
	SpawnArea->ResetGuid();
}

void USpawnAreaManagerComponent::RemoveActivatedFlagFromSpawnArea(USpawnArea* SpawnArea)
{
	if (!SpawnArea)
	{
		return;
	}

	// Remove from activated cache
	const int32 NumRemovedFromCache = CachedActivated.Remove(SpawnArea);

	if (!SpawnArea->IsActivated())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Tried to remove an activated flag from non-activated SpawnArea."));
		return;
	}

	if (NumRemovedFromCache == 0) UE_LOG(LogTargetManager, Warning, TEXT("Failed to remove from CachedActivated set."));

	SpawnArea->SetIsActivated(false);
}

void USpawnAreaManagerComponent::RemoveRecentFlagFromSpawnArea(USpawnArea* SpawnArea)
{
	if (!SpawnArea)
	{
		return;
	}

	const int32 NumRemovedFromCache = CachedRecent.Remove(SpawnArea);

	if (!SpawnArea->IsRecent())
	{
		UE_LOG(LogTargetManager, Warning, TEXT("Tried to remove a recent flag from non-recent SpawnArea."));
		return;
	}

	if (NumRemovedFromCache == 0) UE_LOG(LogTargetManager, Warning, TEXT("Failed to remove from CachedRecent set."));

	SpawnArea->SetIsRecent(false);
}

void USpawnAreaManagerComponent::RefreshRecentFlags()
{
#if !UE_BUILD_SHIPPING
	if (bPrintDebug_SpawnAreaStateInfo)
	{
		PrintDebug_SpawnAreaStateInfo();
	}
#endif

	if (TargetConfig().RecentTargetMemoryPolicy != ERecentTargetMemoryPolicy::NumTargetsBased)
	{
		return;
	}

	const int32 NumToRemove = CachedRecent.Num() - TargetConfig().MaxNumRecentTargets;
	if (NumToRemove <= 0)
	{
		return;
	}

	for (int32 CurrentRemoveNum = 0; CurrentRemoveNum < NumToRemove; CurrentRemoveNum++)
	{
		if (USpawnArea* Found = GetOldestRecentSpawnArea())
		{
			RemoveRecentFlagFromSpawnArea(Found);
		}
	}
}

/* --------------------------------------------------- */
/* -- Finding Valid SpawnAreas for Spawn/Activation -- */
/* --------------------------------------------------- */

TSet<FGuid> USpawnAreaManagerComponent::GetActivatableTargets(const int32 NumToActivate) const
{
	// Assumes that we cannot activate an already activated target

	TSet<USpawnArea*> ValidSpawnAreas = GetManagedDeactivatedNotRecentSpawnAreas();

	/* TODO: Might need to have separate "Recent" for spawning and activation
	 * For game modes like ChargedBeatTrack, there will be 4 managed targets but also 4 recent targets at some point,
	 * which is why this condition exists */
	if (ValidSpawnAreas.IsEmpty())
	{
		ValidSpawnAreas = GetDeactivatedSpawnAreas();

		if (ValidSpawnAreas.IsEmpty() && TargetConfig().bAllowActivationWhileActivated)
		{
			ValidSpawnAreas = GetActivatedSpawnAreas();
		}
	}

	// ReSharper disable once CppLocalVariableMayBeConst
	USpawnArea* PreviousSpawnArea = GetMostRecentSpawnArea();

	switch (TargetConfig().TargetActivationSelectionPolicy)
	{
	case ETargetActivationSelectionPolicy::Bordering:
		{
			TSet<USpawnArea*> Filtered = ValidSpawnAreas;
			RemoveNonAdjacentIndices(Filtered, PreviousSpawnArea);
			if (Filtered.Num() >= NumToActivate)
			{
				ValidSpawnAreas = MoveTemp(Filtered);
			}
		}
		break;
	case ETargetActivationSelectionPolicy::None:
	case ETargetActivationSelectionPolicy::Random:
		break;
	default:
		break;
	}

	TSet<USpawnArea*> ChosenSpawnAreas;
	ChosenSpawnAreas.Reserve(NumToActivate);

	// Main loop for choosing spawn areas
	for (int i = 0; i < NumToActivate; i++)
	{
		if (USpawnArea* Chosen = ChooseActivatableSpawnArea(PreviousSpawnArea, ValidSpawnAreas, ChosenSpawnAreas))
		{
			// Add to the return array
			ChosenSpawnAreas.Add(Chosen);

			// Remove from options available to choose
			ValidSpawnAreas.Remove(Chosen);

			// Set as the previous SpawnArea since it will be activated before any chosen later
			PreviousSpawnArea = Chosen;
		}
	}

	TSet<FGuid> Out;
	for (const USpawnArea* SpawnArea : ChosenSpawnAreas)
	{
		Out.Add(SpawnArea->GetGuid());
	}

	return Out;
}

TSet<FTargetSpawnParams> USpawnAreaManagerComponent::GetTargetSpawnParams(const TArray<FVector>& Scales,
	const int32 NumToSpawn) const
{
	TSet<USpawnArea*> ValidSpawnAreas;

	/* ------------------------------------ */
	/* -- Grid-Based Target Distribution -- */
	/* ------------------------------------ */

	if (TargetConfig().TargetDistributionPolicy == ETargetDistributionPolicy::Grid)
	{
		// Get all SpawnAreas that are managed, deactivated, and not recent
		ValidSpawnAreas = GetUnflaggedSpawnAreas();

#if !UE_BUILD_SHIPPING
		if (bShowDebug_SpawnableSpawnAreas && !GIsAutomationTesting)
		{
			DebugCached_SpawnableValidSpawnAreas = ValidSpawnAreas;
		}
#endif

		// Don't make every function have to check this
		if (ValidSpawnAreas.IsEmpty())
		{
			return TSet<FTargetSpawnParams>();
		}

		switch (TargetConfig().RuntimeTargetSpawningLocationSelectionMode)
		{
		case ERuntimeTargetSpawningLocationSelectionMode::None:
		case ERuntimeTargetSpawningLocationSelectionMode::Random:
			{
				TArray<USpawnArea*> Temp = ValidSpawnAreas.Array();
				Algo::RandomShuffle(Temp);
				ValidSpawnAreas = TSet(MoveTemp(Temp));
			}
			break;
		case ERuntimeTargetSpawningLocationSelectionMode::Bordering:
			{
				FindAdjacentGridUsingDFS(ValidSpawnAreas, NumToSpawn);
			}
			break;
		case ERuntimeTargetSpawningLocationSelectionMode::RandomGridBlock:
			{
				FindGridBlockUsingLargestRectangle(ValidSpawnAreas,
					CreateIndexValidityArray(ValidSpawnAreas, SpawnAreas.Num()), NumToSpawn, false);
			}
			break;
		case ERuntimeTargetSpawningLocationSelectionMode::NearbyGridBlock:
			{
				FindGridBlockUsingLargestRectangle(ValidSpawnAreas,
					CreateIndexValidityArray(ValidSpawnAreas, SpawnAreas.Num()), NumToSpawn, true);
			}
			break;
		case ERuntimeTargetSpawningLocationSelectionMode::RandomVertical: // TODO: NYI
			{
			}
			break;
		case ERuntimeTargetSpawningLocationSelectionMode::RandomHorizontal: // TODO: NYI
			{
			}
			break;
		}

		// Make sure number of elements is no more than number to spawn
		check(ValidSpawnAreas.Num() <= NumToSpawn);

		// Set the target scales
		int i = 0;
		for (USpawnArea* SpawnArea : ValidSpawnAreas)
		{
			SpawnArea->SetTargetScale(Scales[i++]);
			if (i >= Scales.Num())
			{
				break;
			}
		}

		UpdateMostRecentGridBlocks(ValidSpawnAreas, NumToSpawn);
	}
	/* ------------------------------------ */
	/* -- All Other Target Distributions -- */
	/* ------------------------------------ */
	else
	{
		// Start with all SpawnAreas within the current box bounds
		ValidSpawnAreas = CachedExtrema;

		USpawnArea* PreviousSpawnArea = GetMostRecentSpawnArea();

		// Only consider Managed Targets to be invalid if runtime
		TSet<USpawnArea*> InvalidSpawnAreas = ShouldConsiderManagedAsInvalid()
			? GetManagedActivatedOrRecentSpawnAreas()
			: GetActivatedOrRecentSpawnAreas();

		if (RequestMovingTargetLocations.IsBound())
		{
			FMovingTargetLocations MovingTargetLocations;
			TSet<USpawnArea*> TempMoved;
			RequestMovingTargetLocations.Execute(MovingTargetLocations);

			for (const TPair<FGuid, FVector>& Pair : MovingTargetLocations.Map)
			{
				USpawnArea* FoundByLocation = GetSpawnArea(Pair.Value);
				USpawnArea* FoundByGuid = GetSpawnArea(Pair.Key);

				check(FoundByLocation);
				check(FoundByGuid);

				if (!FoundByGuid || !FoundByLocation)
				{
					continue;
				}

				// If target has moved from its original location, don't make overlapping vertices at original
				if (FoundByLocation->GetIndex() != FoundByGuid->GetIndex())
				{
					InvalidSpawnAreas.Remove(FoundByGuid);
					InvalidSpawnAreas.Add(FoundByLocation);
					TempMoved.Add(FoundByGuid);
				}
			}
			// Add back original Spawn Areas that may have been removed due to moving targets
			ValidSpawnAreas = ValidSpawnAreas.Difference(TempMoved);
		}

		TSet<USpawnArea*> ChosenSpawnAreas;

		// Main loop for choosing Spawn Areas
		for (int i = 0; i < NumToSpawn; i++)
		{
			TSet<USpawnArea*> ValidSpawnAreasCopy = ValidSpawnAreas;

			// Remove any overlap caused by any managed/activated Spawn Areas or any already chosen Spawn Areas.
			// Done at every iteration in case current scale > Spawn Area's scale being compared
			RemoveOverlappingSpawnAreas(ValidSpawnAreasCopy, InvalidSpawnAreas, Scales[i]);

			// If multiple are spawning with different scales, one further along might be able to fit
			if (ValidSpawnAreasCopy.IsEmpty())
			{
				continue;
			}

#if !UE_BUILD_SHIPPING
			if (bShowDebug_SpawnableSpawnAreas && i == 0 && !GIsAutomationTesting)
			{
				DebugCached_SpawnableValidSpawnAreas = ValidSpawnAreas;
			}
#endif

			if (USpawnArea* Chosen = ChooseSpawnableSpawnArea(PreviousSpawnArea, ValidSpawnAreasCopy, ChosenSpawnAreas))
			{
				if (TargetConfig().TargetDistributionPolicy != ETargetDistributionPolicy::HeadshotHeightOnly)
				{
					if (GetOriginSpawnArea() && Chosen->GetIndex() != GetOriginSpawnArea()->GetIndex())
					{
						Chosen->SetChosenPoint(USpawnArea::GenerateRandomOffset());
					}
				}
				// Set the scale for the target to be spawned
				Chosen->SetTargetScale(Scales[i]);

				// Add to the return array
				ChosenSpawnAreas.Add(Chosen);

				// Don't allow to be chosen again
				InvalidSpawnAreas.Add(Chosen);

				// Remove from options available
				ValidSpawnAreas.Remove(Chosen);

				// Set as the previous SpawnArea since it will be spawned before any chosen later
				PreviousSpawnArea = Chosen;
			}
		}

		ValidSpawnAreas = MoveTemp(ChosenSpawnAreas);
	}

	TSet<FTargetSpawnParams> Out;
	for (const USpawnArea* SpawnArea : ValidSpawnAreas)
	{
		Out.Emplace(FTargetSpawnParams(SpawnArea->GetChosenPoint(), SpawnArea->GetTargetScale(),
			SpawnArea->GetIndex()));
	}

	return Out;
}

USpawnArea* USpawnAreaManagerComponent::ChooseActivatableSpawnArea(const USpawnArea* PreviousSpawnArea,
	const TSet<USpawnArea*>& ValidSpawnAreas, const TSet<USpawnArea*>& SelectedSpawnAreas) const
{
	// 1st priority: force activate at origin
	// Requirements: Not the previous SpawnArea, not in selected, and corresponds to a spawned target (Valid Guid).
	// Unique exception that does not check ValidSpawnAreas.
	if (TargetConfig().bSpawnEveryOtherTargetInCenter)
	{
		USpawnArea* Candidate = GetOriginSpawnArea();
		if (Candidate && Candidate != PreviousSpawnArea)
		{
			if (Candidate->GetGuid().IsValid() && !SelectedSpawnAreas.Contains(Candidate))
			{
				return Candidate;
			}
		}
	}

	// 2st priority: origin if settings permit
	if (TargetConfig().bSpawnAtOriginWheneverPossible)
	{
		USpawnArea* Candidate = GetOriginSpawnArea();
		if (ValidSpawnAreas.Contains(Candidate) && Candidate->GetGuid().IsValid())
		{
			return Candidate;
		}
	}

	// 3rd priority: Let RLC choose the SpawnArea if settings permit
	if (RequestRLCSpawnArea.IsBound())
	{
		const int32 PreviousIndex = !PreviousSpawnArea ? -1 : PreviousSpawnArea->GetIndex();

		// Transform to array of indices
		TArray<int32> ValidIndices;
		ValidIndices.Reserve(ValidIndices.Num());
		Algo::Transform(ValidSpawnAreas, ValidIndices, [](const USpawnArea* SpawnArea)
		{
			return SpawnArea->GetIndex();
		});

		const int32 CandidateIndex = RequestRLCSpawnArea.Execute(PreviousIndex, ValidIndices);
		USpawnArea* Candidate = GetSpawnArea(CandidateIndex);
		if (Candidate && Candidate->GetGuid().IsValid())
		{
			return Candidate;
		}
	}

	// 4th priority: Randomly select an index from ValidSpawnAreas
	if (!ValidSpawnAreas.IsEmpty())
	{
		const int32 RandomIndex = FMath::RandRange(0, ValidSpawnAreas.Num() - 1);
		USpawnArea* RandomSpawnArea = ValidSpawnAreas.Array()[RandomIndex];
		if (RandomSpawnArea && RandomSpawnArea->GetGuid().IsValid())
		{
			return RandomSpawnArea;
		}
	}

	// No valid spawn area found
	return nullptr;
}

USpawnArea* USpawnAreaManagerComponent::ChooseSpawnableSpawnArea(const USpawnArea* PreviousSpawnArea,
	const TSet<USpawnArea*>& ValidSpawnAreas, const TSet<USpawnArea*>& SelectedSpawnAreas) const
{
	// 1st priority: force spawn at origin
	// Requirements: Not the previous SpawnArea, not managed, and not in selected.
	// Unique exception that does not check ValidSpawnAreas
	if (TargetConfig().bSpawnEveryOtherTargetInCenter)
	{
		USpawnArea* Candidate = GetOriginSpawnArea();
		if (Candidate && !Candidate->IsManaged() && PreviousSpawnArea != Candidate && !SelectedSpawnAreas.
			Contains(Candidate))
		{
			return Candidate;
		}
	}

	// 2st priority: origin if settings permit
	if (TargetConfig().bSpawnAtOriginWheneverPossible)
	{
		USpawnArea* Candidate = GetOriginSpawnArea();
		if (Candidate && ValidSpawnAreas.Contains(Candidate))
		{
			return Candidate;
		}
	}

	// 3rd priority: Let RLC choose the SpawnArea if settings permit
	if (RequestRLCSpawnArea.IsBound())
	{
		const int32 PreviousIndex = PreviousSpawnArea ? PreviousSpawnArea->GetIndex() : -1;

		// Transform to array of indices
		TArray<int32> ValidIndices;
		ValidIndices.Reserve(ValidIndices.Num());
		Algo::Transform(ValidSpawnAreas, ValidIndices, [](const USpawnArea* SpawnArea)
		{
			return SpawnArea->GetIndex();
		});

		const int32 CandidateIndex = RequestRLCSpawnArea.Execute(PreviousIndex, ValidIndices);
		if (USpawnArea* Candidate = GetSpawnArea(CandidateIndex))
		{
			return Candidate;
		}
		UE_LOG(LogTargetManager, Warning, TEXT("Unable to Spawn at SpawnArea suggested by RLAgent."));
	}

	// 4th priority: Randomly select an index from ValidSpawnAreas
	if (!ValidSpawnAreas.IsEmpty())
	{
		return ValidSpawnAreas.Array()[FMath::RandRange(0, ValidSpawnAreas.Num() - 1)];
	}

	// No valid spawn area found
	return nullptr;
}

/* ---------------------------------------------------------------- */
/* -- Helper functions for Valid SpawnAreas for Spawn/Activation -- */
/* ---------------------------------------------------------------- */

void USpawnAreaManagerComponent::FindAdjacentGridUsingDFS(TSet<USpawnArea*>& ValidSpawnAreas,
	const int32 NumToSpawn) const
{
	TArray<USpawnArea*> StartNodeCandidates;

	if (RecentGridBlocks.IsEmpty())
	{
		StartNodeCandidates = ValidSpawnAreas.Array();
	}
	else
	{
		TSet<USpawnArea*> Adjacent;
		for (const TSet<USpawnArea*>& GridBlock : RecentGridBlocks)
		{
			const TSet<USpawnArea*>&& NewAdjacent = GetAdjacentSpawnAreas<USpawnArea*>(GridBlock, DirectionTypes::All);
			const TSet<USpawnArea*>&& Common = Adjacent.Intersect(NewAdjacent);
			Adjacent = Adjacent.Union(NewAdjacent).Difference(Common);
		}
		StartNodeCandidates = Adjacent.Array();
	}

	TSet<USpawnArea*> ValidPath;

	while (ValidPath.Num() < NumToSpawn)
	{
		// Explore all candidate paths if necessary
		if (StartNodeCandidates.IsEmpty())
		{
			break;
		}
		USpawnArea* StartNode = StartNodeCandidates[FMath::RandRange(0, StartNodeCandidates.Num() - 1)];
		StartNodeCandidates.RemoveSwap(StartNode);

		TSet<USpawnArea*> Visited;
		TSet<USpawnArea*> CurrentPath;
		TArray<USpawnArea*> Stack;
		Stack.Push(StartNode);

		while (!Stack.IsEmpty())
		{
			USpawnArea* Vertex = Stack.Pop(false);
			if (Visited.Contains(Vertex))
			{
				continue;
			}

			CurrentPath.Add(Vertex);

			// Exit loop as soon as current path is filled with num to spawn elements
			if (CurrentPath.Num() == NumToSpawn)
			{
				ValidPath = MoveTemp(CurrentPath);
				break;
			}
			// If not possible to spawn NumToSpawn, keep track of best attempt
			if (CurrentPath.Num() > ValidPath.Num())
			{
				ValidPath = CurrentPath;
			}

			TArray<int32> AdjacentIndices = Vertex->GetAdjacentIndices().Array();
			Algo::RandomShuffle(AdjacentIndices);

			for (const int32 Index : AdjacentIndices)
			{
				if (USpawnArea* Adjacent = GetSpawnArea(Index))
				{
					if (!Visited.Contains(Adjacent) && ValidSpawnAreas.Contains(Adjacent))
					{
						Stack.Push(Adjacent);
					}
				}
			}
			Visited.Add(Vertex);
		}
	}
	ValidSpawnAreas = MoveTemp(ValidPath);
}

void USpawnAreaManagerComponent::FindGridBlockUsingLargestRectangle(TSet<USpawnArea*>& ValidSpawnAreas,
	const TArray<int32>& IndexValidity, const int32 BlockSize, const bool bBordering) const
{
	ValidSpawnAreas.Empty();

	// Get all factors for the block size so that FindLargestValidRectangles can make informed decision
	const TSet<FFactor>&& RectangleFactors = IsPrime(BlockSize)
		? FindAllFactors(BlockSize - 1).Union(FindAllFactors(BlockSize + 1))
		: FindAllFactors(BlockSize);

	TArray<FFactor> SortedRectangleFactors = RectangleFactors.Array();
	SortedRectangleFactors.Sort();

	// Get all rectangle candidates
	FRectangleSet&& Rectangles = FindLargestValidRectangles(IndexValidity, SortedRectangleFactors, TotalSpawnAreaSize.Z,
		TotalSpawnAreaSize.Y);

	// If bordering, find the adjacent indices from recent Spawn Areas, and add them to rectangles they intersect with
	if (bBordering)
	{
		TSet<int32> Adjacent;
		for (const TSet<USpawnArea*>& GridBlock : RecentGridBlocks)
		{
			const TSet<int32>&& NewAdjacent = GetAdjacentSpawnAreas<int32>(GridBlock, DirectionTypes::All);
			const TSet<int32>&& Common = Adjacent.Intersect(NewAdjacent);
			Adjacent = Adjacent.Union(NewAdjacent).Difference(Common);
		}
		UpdateRectangleCandidateAdjacentIndices(Rectangles, Adjacent);

#if !UE_BUILD_SHIPPING
		if (bPrintDebug_Grid)
		{
			UE_LOG(LogTemp, Display, TEXT("Number Of Adjacent Spawn Areas: %d"), Adjacent.Num());
		}
#endif
	}

	// Choose a rectangle
	FRectCandidate&& ChosenRectangle = ChooseRectangleCandidate(Rectangles, bBordering, BlockSize);
	if (!ChosenRectangle.HasChosenSubRectangle())
	{
#if !UE_BUILD_SHIPPING
		UE_LOG(LogTargetManager, Warning, TEXT("Chosen Rectangle does not have Chosen SubRectangle"));
#endif
		return;
	}

	// Choose a rectangle orientation
	const FIndexPair Orientation = ChooseRectangleOrientation(ChosenRectangle, ChosenRectangle.Factor);

	// Choose the position and start/end indices within the chosen rectangle's available area
	const auto [bIAsRow, bIncrement] = ChooseRectanglePosition(ChosenRectangle, Orientation, bBordering);

#if !UE_BUILD_SHIPPING
	if (bPrintDebug_Grid)
	{
		PrintDebug_Matrix(IndexValidity, TotalSpawnAreaSize.Z, TotalSpawnAreaSize.Y);
		PrintDebug_GridLargestRect(Rectangles, ChosenRectangle, TotalSpawnAreaSize.Y, Orientation);
	}
#endif

	auto ICheck = [&](const int32 Index)
	{
		return (bIncrement && Index <= ChosenRectangle.ChosenRow.EndIndex) || (!bIncrement && Index >= ChosenRectangle.
			ChosenRow.EndIndex);
	};

	auto JCheck = [&](const int32 Index)
	{
		return (bIncrement && Index <= ChosenRectangle.ChosenCol.EndIndex) || (!bIncrement && Index >= ChosenRectangle.
			ChosenCol.EndIndex);
	};

	// Iterate through the chosen block, adding the SpawnAreas to the set
	for (int i = ChosenRectangle.ChosenRow.StartIndex; ICheck(i); bIncrement ? ++i : --i)
	{
		for (int j = ChosenRectangle.ChosenCol.StartIndex; JCheck(j); bIncrement ? ++j : --j)
		{
			// Choosing a larger block size can lead to having to exit early
			if (ValidSpawnAreas.Num() >= ChosenRectangle.ActualBlockSize)
			{
				break;
			}
			const int32 Index = bIAsRow ? i * TotalSpawnAreaSize.Y + j : j * TotalSpawnAreaSize.Y + i;
			if (USpawnArea* SpawnArea = GetSpawnArea(Index))
			{
				ValidSpawnAreas.Add(SpawnArea);
			}
#if !UE_BUILD_SHIPPING
			else
			{
				UE_LOG(LogTargetManager, Warning, TEXT("Invalid Index: %d"), Index);
			}
#endif
		}
	}

	// Choose a remainder index if ActualBlockSize is prime and a smaller grid is chosen
	if (ValidSpawnAreas.Num() < ChosenRectangle.ActualBlockSize)
	{
		const TSet<int32>&& RemainderSet = GetAdjacentSpawnAreas<int32>(ValidSpawnAreas, DirectionTypes::GridBlock);
		if (!RemainderSet.IsEmpty())
		{
			if (USpawnArea* SpawnArea = GetSpawnArea(RemainderSet.Array()[FMath::RandRange(0, RemainderSet.Num() - 1)]))
			{
				ValidSpawnAreas.Add(SpawnArea);
			}
		}
	}
}

void USpawnAreaManagerComponent::RemoveOverlappingSpawnAreas(TSet<USpawnArea*>& ValidSpawnAreas,
	const TSet<USpawnArea*>& InvalidSpawnAreas, const FVector& NewScale) const
{
	// TODO: This is terribly expensive if going from small target scale to large target scale
	TSet<FVector> Invalid;

	for (USpawnArea* SpawnArea : InvalidSpawnAreas)
	{
		// Choose larger of target scale to be spawned and existing spawned target scale
		if (SpawnArea->GetTargetScale().Length() >= NewScale.Length())
		{
			if (!SpawnArea->GetOccupiedVertices().IsEmpty())
			{
				Invalid.Append(SpawnArea->GetOccupiedVertices());
			}
			else
			{
				Invalid.Append(SpawnArea->MakeOccupiedVertices(SpawnArea->GetTargetScale()));
			}
		}
		else
		{
#if !UE_BUILD_SHIPPING
			if (!GIsAutomationTesting)
			{
				Invalid.Append(SpawnArea->SetMakeDebugOccupiedVertices(NewScale));
			}
			else
			{
				Invalid.Append(SpawnArea->MakeOccupiedVertices(NewScale));
			}
#else
			Invalid.Append(SpawnArea->MakeOccupiedVertices(NewScale));
#endif
		}
	}

	for (const FVector& BotLeft : Invalid)
	{
		if (const USpawnArea* Found = GetSpawnArea(BotLeft))
		{
			ValidSpawnAreas.Remove(Found);
		}
	}
}

int32 USpawnAreaManagerComponent::RemoveNonAdjacentIndices(TSet<USpawnArea*>& ValidSpawnAreas,
	const USpawnArea* Current) const
{
	if (!Current)
	{
		return 0;
	}

	const int32 PreviousSize = ValidSpawnAreas.Num();
	TSet<USpawnArea*> BorderingSpawnAreas;

	for (const int32 Index : Current->GetAdjacentIndices())
	{
		USpawnArea* SpawnArea = GetSpawnArea(Index);
		check(SpawnArea);
		if (ValidSpawnAreas.Contains(SpawnArea))
		{
			BorderingSpawnAreas.Add(SpawnArea);
		}
	}

#if !UE_BUILD_SHIPPING
	if (bShowDebug_NonAdjacent)
	{
		DebugCached_NonAdjacentSpawnAreas = ValidSpawnAreas.Difference(BorderingSpawnAreas);
	}
#endif

	ValidSpawnAreas = MoveTemp(BorderingSpawnAreas);
	return PreviousSize - ValidSpawnAreas.Num();
}

void USpawnAreaManagerComponent::UpdateMostRecentGridBlocks(const TSet<USpawnArea*>& ValidSpawnAreas,
	const int32 NumToSpawn) const
{
	if (NumToSpawn <= 0 || ValidSpawnAreas.IsEmpty())
	{
		return;
	}

	int32 CurrentNumRecentSpawnAreas = 0;
	for (const auto& GridBlock : RecentGridBlocks)
	{
		CurrentNumRecentSpawnAreas += GridBlock.Num();
	}

	int32 NewNumRecentSpawnAreas = CurrentNumRecentSpawnAreas + NumToSpawn;

	if (TargetConfig().RecentTargetMemoryPolicy == ERecentTargetMemoryPolicy::NumTargetsBased)
	{
		while (!RecentGridBlocks.IsEmpty() && NewNumRecentSpawnAreas > TargetConfig().MaxNumRecentTargets)
		{
			NewNumRecentSpawnAreas -= RecentGridBlocks.Pop().Num();
		}
	}
	else
	{
		while (!RecentGridBlocks.IsEmpty())
		{
			RecentGridBlocks.Pop();
		}
	}

	RecentGridBlocks.Push(ValidSpawnAreas);
}

/* ------------- */
/* -- Utility -- */
/* ------------- */

template <typename OutType>
TSet<OutType> USpawnAreaManagerComponent::GetAdjacentSpawnAreas(const TSet<USpawnArea*>& InSpawnAreas,
	const TSet<EAdjacentDirection>& Directions) const
{
	return TSet<OutType>();
}

template <>
TSet<int32> USpawnAreaManagerComponent::GetAdjacentSpawnAreas<int32>(const TSet<USpawnArea*>& InSpawnAreas,
	const TSet<EAdjacentDirection>& Directions) const
{
	TSet<int32> Out;

	for (const USpawnArea* SpawnArea : InSpawnAreas)
	{
		Out.Append(SpawnArea->GetAdjacentIndices(Directions));
	}

	// Don't return any SpawnAreas in the original input
	TSet<int32> Temp;
	Algo::Transform(InSpawnAreas, Temp, [](const USpawnArea* SpawnArea)
	{
		return SpawnArea->GetIndex();
	});

	return Out.Difference(Temp);
}

template <>
TSet<USpawnArea*> USpawnAreaManagerComponent::GetAdjacentSpawnAreas<USpawnArea*>(const TSet<USpawnArea*>& InSpawnAreas,
	const TSet<EAdjacentDirection>& Directions) const
{
	TSet<USpawnArea*> Out;

	for (const USpawnArea* SpawnArea : InSpawnAreas)
	{
		for (const int32 Index : SpawnArea->GetAdjacentIndices(Directions))
		{
			if (USpawnArea* Adjacent = GetSpawnArea(Index))
			{
				Out.Add(Adjacent);
			}
		}
	}

	// Don't return any SpawnAreas in the original input
	return Out.Difference(InSpawnAreas);
}

TArray<int32> USpawnAreaManagerComponent::CreateIndexValidityArray(const TSet<USpawnArea*>& ValidSpawnAreas,
	const int32 NumSpawnAreas)
{
	TArray<int32> IndexValidity;
	IndexValidity.Init(0, NumSpawnAreas);
	for (const USpawnArea* SpawnArea : ValidSpawnAreas)
	{
		IndexValidity[SpawnArea->GetIndex()] = 1;
	}
	return IndexValidity;
}

FRectangleSet USpawnAreaManagerComponent::FindLargestValidRectangles(const TArray<int32>& IndexValidity,
	const TArray<FFactor>& Factors, const int32 NumRows, const int32 NumCols)
{
	FRectangleSet ValidRectangles;

	// Create a histogram of column heights initialized with zeros
	TArray<int32> Heights;
	Heights.Init(0, NumCols);
	FSubRectangle::SetNumCols(NumCols);

	for (int32 Row = 0; Row < NumRows; Row++)
	{
		// Update the column heights based on the current row's validity
		for (int32 Col = 0; Col < NumCols; Col++)
		{
			const int32 Index = Row * NumCols + Col;
			// If the cell is valid, increment the column height; otherwise, reset to 0
			Heights[Col] = IndexValidity[Index] ? Heights[Col] + 1 : 0;
		}
		UpdateLargestRectangles(ValidRectangles, Factors, Heights, Row);
	}

	for (auto& Rectangle : ValidRectangles)
	{
		Rectangle.MergeSubRectangles();
	}

	return ValidRectangles;
}

void USpawnAreaManagerComponent::UpdateLargestRectangles(FRectangleSet& Rectangles, const TArray<FFactor>& Factors,
	TArray<int32>& Heights, const int32 CurrentRow)
{
	std::stack<FSubRectangle> Stack;

	// Iterate through the columns to identify potential rectangles
	for (int32 i = 0; i < Heights.Num(); ++i)
	{
		int32 StartColIndex = i;

		// Compare the current column's height with the height of the column at the top of the stack
		// If the current height is less than the stack's height, it suggests the potential end of a rectangle
		while (!Stack.empty() && Stack.top().Dimensions.Height > Heights[i])
		{
			// Area of the potential rectangle with the current column as the right boundary
			if (Stack.top().Dimensions.Height > 0)
			{
				// Update the width and start end index
				Stack.top().UpdateDimensions(i - Stack.top().ColIndex);
				UpdateSubRectangles(Rectangles, Factors, Stack.top());
			}

			// Update StartColIndex to the column index at the top of the stack since measuring width from there to i
			StartColIndex = Stack.top().ColIndex;

			Stack.pop();
		}
		Stack.push(FSubRectangle(CurrentRow * FSubRectangle::NumCols + StartColIndex, StartColIndex, Heights[i]));
	}
	// After processing all columns, check if there are remaining elements in the stack
	while (!Stack.empty())
	{
		if (Stack.top().Dimensions.Height > 0)
		{
			// Update the width and start end index
			Stack.top().UpdateDimensions(FSubRectangle::NumCols - Stack.top().ColIndex);
			UpdateSubRectangles(Rectangles, Factors, Stack.top());
		}
		Stack.pop();
	}
}

void USpawnAreaManagerComponent::UpdateSubRectangles(FRectangleSet& Rectangles, const TArray<FFactor>& Factors,
	const FSubRectangle& SubRectangle)
{
	for (const FFactor& Factor : Factors)
	{
		if ((SubRectangle.Dimensions.Width >= Factor.Factor1 && SubRectangle.Dimensions.Height >= Factor.Factor2) || (
			SubRectangle.Dimensions.Width >= Factor.Factor2 && SubRectangle.Dimensions.Height >= Factor.Factor1))
		{
			if (FRectCandidate* FoundRectCandidate = Rectangles.Find(Factor))
			{
				FoundRectCandidate->UpdateSubRectangles(SubRectangle);
			}
			else
			{
				Rectangles.Emplace(FRectCandidate(Factor));
			}
		}
	}
}

FRectCandidate USpawnAreaManagerComponent::ChooseRectangleCandidate(const FRectangleSet& Rectangles,
	const bool bBordering, const int32 BlockSize)
{
	// Convert to array and sort based on FRectCandidate < operator
	TArray<FRectCandidate> RectanglesArr = Rectangles.Array();
	RectanglesArr.Sort();

	if (bBordering)
	{
		for (auto& Rectangle : RectanglesArr)
		{
			TArray<FSubRectangle> SortedSubRectangles = Rectangle.SubRectangles.Array();
			Algo::RandomShuffle(SortedSubRectangles);
			for (const auto& SubRectangle : SortedSubRectangles)
			{
				if (!SubRectangle.StartIndexCandidates.IsEmpty())
				{
					// choose the first encountered sub rectangle that has start index candidates
					Rectangle.SetChosenSubRectangle(SubRectangle, BlockSize);
					return Rectangle;
				}
			}
		}
	}

	// Choose a random one that has SubRectangles
	for (auto& Rectangle : RectanglesArr)
	{
		if (!Rectangle.SubRectangles.IsEmpty())
		{
			TArray<FSubRectangle> SubRectangles = Rectangle.SubRectangles.Array();
			Rectangle.SetChosenSubRectangle(SubRectangles[FMath::RandRange(0, SubRectangles.Num() - 1)], BlockSize);
			return Rectangle;
		}
	}

	return FRectCandidate();
}

FIndexPair USpawnAreaManagerComponent::ChooseRectangleOrientation(const FRectCandidate& Rect, const FFactor& Factor)
{
	int32 SubRowSize = -1;
	int32 SubColSize = -1;

	// All fit, choose random
	if (Rect.AllFactorsFit())
	{
		const bool bRandom = FMath::RandBool();
		SubRowSize = bRandom ? Factor.Factor1 : Factor.Factor2;
		SubColSize = bRandom ? Factor.Factor2 : Factor.Factor1;
	}
	else if (Rect.FirstFactorComboFits())
	{
		SubRowSize = Factor.Factor1;
		SubColSize = Factor.Factor2;
	}
	else if (Rect.SecondFactorComboFits())
	{
		SubRowSize = Factor.Factor2;
		SubColSize = Factor.Factor1;
	}
	return FIndexPair(SubRowSize, SubColSize);
}

std::pair<bool, bool> USpawnAreaManagerComponent::ChooseRectanglePosition(FRectCandidate& ChosenRectangle,
	const FIndexPair& Orientation, const bool bBordering)
{
	// ChosenRow and ChosenCol are initialized to the chosen sub rectangles full Row, Col
	const int32 MaxStartRowIndex = ChosenRectangle.ChosenRow.EndIndex - Orientation.StartIndex + 1;
	const int32 MaxStartColIndex = ChosenRectangle.ChosenCol.EndIndex - Orientation.EndIndex + 1;
	ChosenRectangle.ChosenBlockSize = Orientation.StartIndex * Orientation.EndIndex;

	if (bBordering && !ChosenRectangle.ChosenSubRectangle.StartIndexCandidates.IsEmpty())
	{
		const auto RandomAdjacent = ChosenRectangle.ChosenSubRectangle.StartIndexCandidates[FMath::RandRange(0,
			ChosenRectangle.ChosenSubRectangle.StartIndexCandidates.Num() - 1)];
		ChosenRectangle.ChosenRow.StartIndex = RandomAdjacent.StartIndex;
		ChosenRectangle.ChosenCol.StartIndex = RandomAdjacent.EndIndex;
	}
	else
	{
		ChosenRectangle.ChosenRow.StartIndex = FMath::RandRange(ChosenRectangle.ChosenRow.StartIndex, MaxStartRowIndex);
		ChosenRectangle.ChosenCol.StartIndex = FMath::RandRange(ChosenRectangle.ChosenRow.StartIndex, MaxStartColIndex);
	}

	ChosenRectangle.ChosenRow.EndIndex = ChosenRectangle.ChosenRow.StartIndex + Orientation.StartIndex - 1;
	ChosenRectangle.ChosenCol.EndIndex = ChosenRectangle.ChosenCol.StartIndex + Orientation.EndIndex - 1;

	bool bIAsRow = true;
	bool bIncrement = true;

	// Randomize the start indices if it will get chopped off
	if (ChosenRectangle.ChosenBlockSize > ChosenRectangle.ActualBlockSize)
	{
		bIAsRow = FMath::RandBool();
		bIncrement = FMath::RandBool();

		// Swap rows and columns
		if (!bIAsRow)
		{
			Swap(ChosenRectangle.ChosenRow.StartIndex, ChosenRectangle.ChosenCol.StartIndex);
			Swap(ChosenRectangle.ChosenRow.EndIndex, ChosenRectangle.ChosenCol.EndIndex);
		}

		// Swap start and end
		if (!bIncrement)
		{
			Swap(ChosenRectangle.ChosenRow.StartIndex, ChosenRectangle.ChosenRow.EndIndex);
			Swap(ChosenRectangle.ChosenCol.StartIndex, ChosenRectangle.ChosenCol.EndIndex);
		}
	}

	return std::pair(bIAsRow, bIncrement);
}

TSet<FFactor> USpawnAreaManagerComponent::GetPreferredRectangleDimensions(const int32 BlockSize, const int32 NumRows,
	const int32 NumCols)
{
	TSet<FFactor> MinFactors;
	// If the block size is prime, find the min factors of one less and one greater than the ActualBlockSize
	if (IsPrime(BlockSize))
	{
		const TSet<FFactor> OneLessBlockSize = FindBestFittingFactors(BlockSize - 1, NumRows, NumCols);
		const TSet<FFactor> OneGreaterBlockSize = FindBestFittingFactors(BlockSize + 1, NumRows, NumCols);

		if (OneLessBlockSize.IsEmpty() && OneGreaterBlockSize.IsEmpty())
		{
			return MinFactors;
		}

		FFactor MinDifferenceFactor = FFactor(BlockSize);
		for (const FFactor Factor : OneLessBlockSize.Union(OneGreaterBlockSize))
		{
			if (Factor.Distance < MinDifferenceFactor.Distance)
			{
				MinDifferenceFactor = Factor;
				MinFactors.Empty();
			}
			if (Factor.Distance <= MinDifferenceFactor.Distance)
			{
				MinFactors.Add(Factor);
			}
		}
	}
	else
	{
		MinFactors = FindBestFittingFactors(BlockSize, NumRows, NumCols);
	}

	return MinFactors;
}

void USpawnAreaManagerComponent::UpdateRectangleCandidateAdjacentIndices(FRectangleSet& Rectangles,
	const TSet<int32>& Adjacent)
{
	for (FRectCandidate& Rectangle : Rectangles)
	{
		for (FSubRectangle& SubRectangle : Rectangle.SubRectangles)
		{
			for (const int32 Index : Adjacent)
			{
				const int32 Row = Index / FSubRectangle::NumCols;
				const int32 Col = Index % FSubRectangle::NumCols;

				const bool bGrEqualToStart = Row >= SubRectangle.Row.StartIndex && Col >= SubRectangle.Col.StartIndex;
				const bool bLeEqualToEnd = Row <= SubRectangle.Row.EndIndex && Col <= SubRectangle.Col.EndIndex;
				if (bGrEqualToStart && bLeEqualToEnd)
				{
					SubRectangle.AdjacentIndices.Add(Index);
				}
				for (int i = SubRectangle.Row.StartIndex; i <= SubRectangle.Row.EndIndex; ++i)
				{
					for (int j = SubRectangle.Col.StartIndex; j <= SubRectangle.Col.EndIndex; ++j)
					{
						const int32 BlockRowEnd = i + Rectangle.Factor.Factor1 - 1;
						const int32 BlockColEnd = j + Rectangle.Factor.Factor2 - 1;
						if (BlockRowEnd <= SubRectangle.Row.EndIndex && BlockColEnd <= SubRectangle.Col.EndIndex)
						{
							if (Row >= i && Col >= j && Row <= BlockRowEnd && Col <= BlockColEnd)
							{
								SubRectangle.StartIndexCandidates.Add(FIndexPair(i, j));
							}
						}
					}
				}
			}
		}
	}
}

TSet<FFactor> USpawnAreaManagerComponent::FindAllFactors(const int32 Number)
{
	TSet<FFactor> Out;
	if (Number == 0)
	{
		return Out;
	}
	if (Number == 1)
	{
		Out.Add(FFactor(1, 1));
		return Out;
	}
	for (int i = 1; i <= Number / 2; i++)
	{
		if (Number % i == 0)
		{
			Out.Add(FFactor(i, Number / i));
		}
	}
	return Out;
}

FFactor USpawnAreaManagerComponent::FindLargestFactors(const int32 Number)
{
	FFactor Factor(-1, -1);
	float MinDistance = Number;
	for (int i = 2; i <= Number / 2; i++)
	{
		if (Number % i == 0)
		{
			const int32 OtherNumber = Number / i;
			const float Distance = abs(i - OtherNumber);
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				Factor.Factor1 = i;
				Factor.Factor2 = OtherNumber;
			}
		}
	}
	return Factor;
}

TSet<FFactor> USpawnAreaManagerComponent::FindBestFittingFactors(const int32 Number, const int32 Constraint1,
	const int32 Constraint2)
{
	TSet<FFactor> FittingFactors;
	float MinDistance = Number;
	// Add any factors that that fit within constraints, and record the minimum distance between them
	for (const FFactor& Factor : FindAllFactors(Number))
	{
		const float Dist = abs(Factor.Factor1 - Factor.Factor2);
		if ((Factor.Factor1 <= Constraint1 && Factor.Factor2 <= Constraint2) || (Factor.Factor2 <= Constraint1 && Factor
			.Factor1 <= Constraint2))
		{
			FittingFactors.Add(Factor);
			if (Dist < MinDistance)
			{
				MinDistance = Dist;
			}
		}
	}

	TSet<FFactor> BestFactors;
	// Add the factors with the minimum distance, since there might be more than one
	for (const FFactor& Factor : FittingFactors)
	{
		if (abs(Factor.Factor1 - Factor.Factor2) == MinDistance)
		{
			BestFactors.Add(Factor);
		}
	}

	return BestFactors;
}

constexpr bool USpawnAreaManagerComponent::IsPrime(const int32 Number)
{
	if (Number <= 1)
	{
		return false;
	}
	if (Number == 2 || Number == 3)
	{
		return true;
	}
	if (Number % 2 == 0 || Number % 3 == 0)
	{
		return false;
	}

	// All prime numbers > 3 can be expressed by 6k ± 1, where k is a positive integer
	int i = 5;

	while (i * i <= Number)
	{
		// Check divisibility from (5, sqrt(number)), or by an integer that is 2 greater
		if (Number % i == 0 || Number % (i + 2) == 0)
		{
			return false;
		}
		i += 6;
	}

	return true;
}

FAccuracyData USpawnAreaManagerComponent::GetLocationAccuracy()
{
	TArray<int32> TotalSpawns;
	TArray<int32> TotalHits;

	TotalSpawns.Init(-1, SpawnAreas.Num());
	TotalHits.Init(0, SpawnAreas.Num());

#if !UE_BUILD_SHIPPING
	int32 TotalSpawnsValueRef = 0;
	int32 TotalHitsValueRef = 0;
	int32 TotalSpawnsValueTest = 0;
	int32 TotalHitsValueTest = 0;
#endif

	const bool bHitDamage = TargetConfig().TargetDamageType == ETargetDamageType::Hit;
	// For now only handle separate Hit and Tracking Damage
	int32 (USpawnArea::*TotalFunc)() const = bHitDamage
		? &USpawnArea::GetTotalSpawns
		: &USpawnArea::GetTotalTrackingDamagePossible;
	int32 (USpawnArea::*HitFunc)() const = bHitDamage ? &USpawnArea::GetTotalHits : &USpawnArea::GetTotalTrackingDamage;

	for (USpawnArea* SpawnArea : SpawnAreas)
	{
		const int32 Index = SpawnArea->GetIndex();
		const int32 SpawnsValue = (SpawnArea->*TotalFunc)();
		const int32 HitsValue = (SpawnArea->*HitFunc)();

		TotalSpawns[Index] = SpawnsValue;
		TotalHits[Index] = HitsValue;

#if !UE_BUILD_SHIPPING
		if (SpawnsValue >= 0)
		{
			TotalSpawnsValueRef += SpawnsValue;
		}
		if (HitsValue > 0)
		{
			TotalHitsValueRef += HitsValue;
		}
#endif
	}

	FAccuracyData OutData = GetAveragedAccuracyData(TotalSpawns, TotalHits, TotalSpawnAreaSize.Z, TotalSpawnAreaSize.Y);

	OutData.SpawnAreaSize = FVector(TotalSpawnAreaSize.X, TotalSpawnAreaSize.Y, TotalSpawnAreaSize.Z);
	OutData.CalculateAccuracy();

	if (TotalSpawnAreaSize.Y < 5 || TotalSpawnAreaSize.Z < 5)
	{
		OutData.ModifyForSmallerInput();
	}

#if !UE_BUILD_SHIPPING
	for (int i = 0; i < OutData.AccuracyRows.Num(); i++)
	{
		for (int j = 0; j < OutData.AccuracyRows[i].Size; j++)
		{
			const int32 TotalSpawnsValue = OutData.AccuracyRows[i].TotalSpawns[j];
			const int32 TotalHitsValue = OutData.AccuracyRows[i].TotalHits[j];

			if (TotalSpawnsValue > 0)
			{
				TotalSpawnsValueTest += TotalSpawnsValue;
			}
			if (TotalHitsValue > 0)
			{
				TotalHitsValueTest += TotalHitsValue;
			}
		}
	}
	UE_LOG(LogTargetManager, Display, TEXT("Total Spawns/Hits Ref: %d %d"), TotalSpawnsValueRef, TotalHitsValueRef);
	UE_LOG(LogTargetManager, Display, TEXT("Total Spawns/Hits Test: %d %d"), TotalSpawnsValueTest, TotalHitsValueTest);
#endif

	return OutData;
}

int32 USpawnAreaManagerComponent::CalcManhattanDist(const int32 Index1, const int32 Index2, const int32 NumCols)
{
	const int32 Row1 = Index1 / NumCols;
	const int32 Col1 = Index1 % NumCols;
	const int32 Row2 = Index2 / NumCols;
	const int32 Col2 = Index2 % NumCols;

	// Calculate Manhattan distance (sum of horizontal and vertical distances)
	return FMath::Abs(Row2 - Row1) + FMath::Abs(Col2 - Col1);
}

TArray<int32> USpawnAreaManagerComponent::FilterIndices(TArray<USpawnArea*>& ValidSpawnAreas,
	bool (USpawnArea::*FilterFunc)() const)
{
	TArray<int32> Removed;
	ValidSpawnAreas = ValidSpawnAreas.FilterByPredicate([&](const USpawnArea* SpawnArea)
	{
		if ((SpawnArea->*FilterFunc)())
		{
			Removed.Add(SpawnArea->GetIndex());
			return false;
		}
		return true;
	});
	return Removed;
}

/* ----------- */
/* -- Debug -- */
/* ----------- */

#if !UE_BUILD_SHIPPING

void USpawnAreaManagerComponent::DrawDebug() const
{
	if (bShowDebug_AllSpawnAreas)
	{
		DrawDebug_Boxes(SpawnAreas, DebugColor_AllSpawnAreas, DebugBoxLineThickness, true);
	}
	if (bShowDebug_ValidInvalidSpawnAreas)
	{
		const TSet<USpawnArea*> InvalidSpawnAreas = ShouldConsiderManagedAsInvalid()
			? CachedManaged.Union(CachedActivated)
			: CachedActivated;
		const TSet<USpawnArea*> RecentSpawnAreas = CachedRecent;

		TSet<FVector> InvalidLocations;
		TSet<FVector> RecentLocations;

		TSet<USpawnArea*> OverlappingInvalid;
		TSet<USpawnArea*> OverlappingRecent;

		for (USpawnArea* SpawnArea : InvalidSpawnAreas)
		{
			TSet Occupied = SpawnArea->GetOccupiedVertices();
			if (Occupied.IsEmpty())
			{
				Occupied = SpawnArea->MakeOccupiedVertices(SpawnArea->GetTargetScale());
			}
			InvalidLocations.Append(Occupied);
		}
		for (const FVector& BotLeft : InvalidLocations)
		{
			if (USpawnArea* Found = GetSpawnArea(BotLeft))
			{
				OverlappingInvalid.Add(Found);
			}
		}

		for (USpawnArea* SpawnArea : RecentSpawnAreas)
		{
			TSet Occupied = SpawnArea->GetOccupiedVertices();
			if (Occupied.IsEmpty())
			{
				Occupied = SpawnArea->MakeOccupiedVertices(SpawnArea->GetTargetScale());
			}
			RecentLocations.Append(Occupied);
		}
		for (const FVector& BotLeft : RecentLocations)
		{
			if (USpawnArea* Found = GetSpawnArea(BotLeft))
			{
				OverlappingRecent.Add(Found);
			}
		}

		const TSet<USpawnArea*> OverlappingValid = CachedExtrema.Difference(
			OverlappingInvalid.Union(OverlappingRecent));
		DrawDebug_Boxes(OverlappingValid, DebugColor_ValidOverlap, DebugBoxLineThickness, true);
		DrawDebug_Boxes(OverlappingRecent, DebugColor_RecentSpawnAreas, DebugBoxLineThickness, true);
		DrawDebug_Boxes(OverlappingInvalid, DebugColor_InvalidOverlap, DebugBoxLineThickness, true);
	}
	if (bShowDebug_RemovedFromExtremaChange)
	{
		const TSet<USpawnArea*> RemovedExtrema = SpawnAreas.Difference(CachedExtrema);
		DrawDebug_Boxes(RemovedExtrema, DebugColor_RemovedFromExtremaChange, DebugBoxLineThickness, true);
	}
	if (bShowDebug_NonAdjacent)
	{
		const TSet<USpawnArea*> NonAdjacent = DebugCached_NonAdjacentSpawnAreas;
		DrawDebug_Boxes(NonAdjacent, DebugColor_NonAdjacent, DebugBoxLineThickness, true);
	}
	if (bShowDebug_SpawnableSpawnAreas)
	{
		const TSet<USpawnArea*> ValidSpawnable = DebugCached_SpawnableValidSpawnAreas;
		DrawDebug_Boxes(ValidSpawnable, DebugColor_SpawnableSpawnAreas, DebugBoxLineThickness, true);
	}
	if (bShowDebug_ActivatableSpawnAreas)
	{
		TSet<USpawnArea*> ValidActivatable = GetManagedDeactivatedNotRecentSpawnAreas();
		if (ValidActivatable.IsEmpty())
		{
			ValidActivatable = GetDeactivatedSpawnAreas();
		}
		DrawDebug_Boxes(ValidActivatable, DebugColor_ActivatableSpawnAreas, DebugBoxLineThickness, true);
	}
	if (bShowDebug_DeactivatedSpawnAreas)
	{
		DrawDebug_Boxes(GetDeactivatedSpawnAreas(), DebugColor_DeactivatedSpawnAreas, DebugBoxLineThickness, true);
	}
	if (bShowDebug_RecentSpawnAreas)
	{
		DrawDebug_Boxes(GetRecentSpawnAreas(), DebugColor_RecentSpawnAreas, DebugBoxLineThickness, true);
	}
	if (bShowDebug_ActivatedSpawnAreas)
	{
		DrawDebug_Boxes(GetActivatedSpawnAreas(), DebugColor_ActivatedSpawnAreas, DebugBoxLineThickness, true);
	}
	if (ShowDebug_Vertices > 0)
	{
		DrawDebug_Vertices(GetActivatedSpawnAreas(), ShowDebug_Vertices == 1 || ShowDebug_Vertices == 2,
			ShowDebug_Vertices == 1 || ShowDebug_Vertices == 3);
	}
}

void USpawnAreaManagerComponent::DrawDebug_Boxes(const TSet<USpawnArea*>& InSpawnAreas, const FColor& Color,
	const int32 Thickness, const bool bPersistent) const
{
	const float Time = bPersistent ? -1.f : TargetConfig().TargetSpawnCD;
	const FVector HalfInc = {0.f, GetSpawnAreaDimensions().Y * 0.5f, GetSpawnAreaDimensions().Z * 0.5f};
	const FVector Offset = {DebugBoxXOffset, 0.f, 0.f};
	const UWorld* World = GetWorld();
	for (const USpawnArea* SpawnArea : InSpawnAreas)
	{
		DrawDebugBox(World, SpawnArea->GetCenterPoint() + Offset, HalfInc, Color, bPersistent, Time, 0, Thickness);
	}
}

void USpawnAreaManagerComponent::DrawDebug_Vertices(const TSet<USpawnArea*>& InSpawnAreas, const bool bGenerateNew,
	const bool bDrawSphere) const
{
	TSet<FVector> InvalidVertices, ValidVertices, SpawnAreaVertices;
	for (const USpawnArea* SpawnArea : InSpawnAreas)
	{
		const FVector Scale = bGenerateNew || SpawnArea->DebugOccupiedVertices.IsEmpty()
			? SpawnArea->GetTargetScale()
			: SpawnArea->LastOccupiedVerticesTargetScale;

		if (bDrawSphere)
		{
			const float Radius = USpawnArea::CalcTraceRadius(Scale);
			DrawDebugSphere(GetWorld(), SpawnArea->GetBottomLeftVertex(), Radius, DebugSphereSegments, FColor::Magenta,
				true);
		}

		if (bGenerateNew || SpawnArea->DebugOccupiedVertices.IsEmpty())
		{
			InvalidVertices.Append(SpawnArea->MakeOccupiedVertices(Scale));
		}
		else
		{
			InvalidVertices.Append(SpawnArea->DebugOccupiedVertices);
		}

		ValidVertices.Append(SpawnArea->MakeUnoccupiedVertices(Scale));
		SpawnAreaVertices.Add(SpawnArea->GetBottomLeftVertex());
	}

	InvalidVertices = InvalidVertices.Difference(SpawnAreaVertices);
	ValidVertices = ValidVertices.Difference(InvalidVertices).Difference(SpawnAreaVertices);

	for (const FVector& Vertex : ValidVertices)
	{
		DrawDebugPoint(GetWorld(), Vertex, DebugVertexSize, FColor::Green, true);
	}
	for (const FVector& Vertex : InvalidVertices)
	{
		DrawDebugPoint(GetWorld(), Vertex, DebugVertexSize, FColor::Red, true);
	}
	for (const FVector& Vertex : SpawnAreaVertices)
	{
		DrawDebugPoint(GetWorld(), Vertex, DebugVertexSize, FColor::Cyan, true);
	}
}

void USpawnAreaManagerComponent::PrintDebug_SpawnAreaStateInfo() const
{
	const int NumRecent = GetRecentSpawnAreas().Num();
	const int NumAct = GetActivatedSpawnAreas().Num();
	const int NumManaged = GetManagedSpawnAreas().Num();
	UE_LOG(LogTargetManager, Display, TEXT("NumRecent: %d NumActivated: %d NumManaged: %d"), NumRecent, NumAct,
		NumManaged);
}

void USpawnAreaManagerComponent::PrintDebug_SpawnArea(const USpawnArea* SpawnArea)
{
	UE_LOG(LogTargetManager, Display, TEXT("SpawnArea:"));
	UE_LOG(LogTargetManager, Display, TEXT("Index %d GridIndexType %s"), SpawnArea->GetIndex(),
		*UEnum::GetDisplayValueAsText(SpawnArea->GetIndexType()).ToString());
	UE_LOG(LogTargetManager, Display, TEXT("Vertex_BottomLeft: %s CenterPoint: %s ChosenPoint: %s"),
		*SpawnArea->GetBottomLeftVertex().ToCompactString(), *SpawnArea->GetCenterPoint().ToCompactString(),
		*SpawnArea->GetChosenPoint().ToCompactString());
	FString String;

	for (const int32 Border : SpawnArea->GetAdjacentIndices())
	{
		String.Append(" " + FString::FromInt(Border));
	}

	UE_LOG(LogTargetManager, Display, TEXT("AdjacentIndices %s"), *String);
	UE_LOG(LogTargetManager, Display, TEXT("IsActivated %hhd IsRecent %hhd"), SpawnArea->IsActivated(),
		SpawnArea->IsRecent());
	UE_LOG(LogTargetManager, Display, TEXT("TotalSpawns %d TotalHits %d"), SpawnArea->GetTotalSpawns(),
		SpawnArea->GetTotalHits());
}

void USpawnAreaManagerComponent::PrintDebug_SpawnAreaDist(const USpawnArea* SpawnArea) const
{
	const float MaxAllowedDistance = SpawnArea->GetTargetScale().X * Constants::SphereTargetRadius;
	for (const USpawnArea* ActivatedOrRecent : GetActivatedOrRecentSpawnAreas())
	{
		const double Distance = FVector::Distance(ActivatedOrRecent->GetChosenPoint(), SpawnArea->GetChosenPoint());

		if (Distance < MaxAllowedDistance)
		{
			UE_LOG(LogTargetManager, Display, TEXT("Distance between targets (%.2f) less than max allowed: %.2f"),
				Distance, MaxAllowedDistance);
		}
	}
}

void USpawnAreaManagerComponent::PrintDebug_GridLargestRect(const FRectangleSet& Rectangles,
	const FRectCandidate& Chosen, const int32 NumCols, const FIndexPair& Orientation)
{
	const int32 StartRowIndex = Chosen.ChosenSubRectangle.BoundingIndices.StartIndex / NumCols;
	const int32 StartColumnIndex = Chosen.ChosenSubRectangle.BoundingIndices.StartIndex % NumCols;
	const int32 EndRowIndex = Chosen.ChosenSubRectangle.BoundingIndices.EndIndex / NumCols;
	const int32 EndColumnIndex = Chosen.ChosenSubRectangle.BoundingIndices.EndIndex % NumCols;
	const int32 MaxAllowedStartRowIndex = Chosen.ChosenRow.EndIndex - Orientation.StartIndex + 1;
	const int32 MaxAllowedStartColIndex = Chosen.ChosenCol.EndIndex - Orientation.EndIndex + 1;

	UE_LOG(LogTargetManager, Display, TEXT("LargestRect MaxArea: %d"), Chosen.ChosenSubRectangle.Area);
	UE_LOG(LogTargetManager, Display, TEXT("LargestRect StartIndices: [%d, %d](%d), EndIndices: [%d, %d](%d)"),
		StartRowIndex, StartColumnIndex, Chosen.ChosenSubRectangle.BoundingIndices.StartIndex, EndRowIndex,
		EndColumnIndex, Chosen.ChosenSubRectangle.BoundingIndices.EndIndex);
	UE_LOG(LogTargetManager, Display, TEXT("LargestRect ChosenStart: [%d, %d](%d), ChosenEnd: [%d, %d](%d)"),
		Chosen.ChosenRow.StartIndex, Chosen.ChosenCol.StartIndex,
		Chosen.ChosenRow.StartIndex * NumCols + Chosen.ChosenCol.StartIndex, Chosen.ChosenRow.EndIndex,
		Chosen.ChosenCol.EndIndex, Chosen.ChosenRow.EndIndex * NumCols + Chosen.ChosenCol.EndIndex);
	UE_LOG(LogTargetManager, Display, TEXT("NumRowsAvail: %d NumColsAvail: %d F1: %d F2: %d"), Chosen.NumRowsAvailable,
		Chosen.NumColsAvailable, Chosen.Factor.Factor1, Chosen.Factor.Factor2);
	UE_LOG(LogTargetManager, Display, TEXT("MaxAllowedStartIndex: %d"),
		MaxAllowedStartRowIndex * NumCols + MaxAllowedStartColIndex);

	int Idx = 0;
	FString Line;

	FNumberFormattingOptions Options;
	Options.MinimumIntegralDigits = 3;
	Options.MaximumIntegralDigits = 3;

	FNumberFormattingOptions Options2;
	Options2.MinimumIntegralDigits = 2;
	Options2.MaximumIntegralDigits = 2;

	UE_LOG(LogTargetManager, Display, TEXT("Rectangles length: %d"), Rectangles.Num());
	for (const auto& Rectangle : Rectangles)
	{
		UE_LOG(LogTargetManager, Display, TEXT("%s"), *Rectangle.ToString());
		for (const auto& SubRectangle : Rectangle.SubRectangles)
		{
			if (Idx < 5)
			{
				FString Number = FText::AsNumber(SubRectangle.AdjacentIndices.Num(), &Options2).ToString();
				FString Number2 = FText::AsNumber(SubRectangle.BoundingIndices.StartIndex, &Options).ToString();
				FString Number3 = FText::AsNumber(SubRectangle.BoundingIndices.EndIndex, &Options).ToString();
				Line += FString::Printf(TEXT("%s [%s, %s]"), *Number, *Number2, *Number3) + " ";
				Idx++;
			}
			else
			{
				UE_LOG(LogTargetManager, Display, TEXT("%s"), *Line);
				Line.Empty();
				Idx = 0;
			}
		}
		if (Idx > 0)
		{
			UE_LOG(LogTargetManager, Display, TEXT("%s"), *Line);
			Line.Empty();
			Idx = 0;
		}
	}

	check(Chosen.ChosenRow.StartIndex >= Chosen.ChosenSubRectangle.BoundingIndices.StartIndex);
	check(Chosen.ChosenCol.StartIndex >= Chosen.ChosenSubRectangle.BoundingIndices.StartIndex);
	check(Chosen.ChosenRow.EndIndex <= Chosen.ChosenSubRectangle.BoundingIndices.EndIndex);
	check(Chosen.ChosenCol.EndIndex <= Chosen.ChosenSubRectangle.BoundingIndices.EndIndex);
}

void USpawnAreaManagerComponent::PrintDebug_Matrix(const TArray<int32>& Matrix, const int32 NumRows,
	const int32 NumCols)
{
	FString Line;
	TArray<FString> Lines;
	Lines.Init("", NumRows);

	const FString Xs = NumRows * NumCols >= 100 ? "___" : "__";

	FNumberFormattingOptions Options;
	Options.MinimumIntegralDigits = NumRows * NumCols >= 100 ? 3 : 2;
	Options.MaximumIntegralDigits = NumRows * NumCols >= 100 ? 3 : 2;

	FNumberFormattingOptions RowNumberOptions;
	RowNumberOptions.MinimumIntegralDigits = NumRows >= 10 ? 2 : 1;
	RowNumberOptions.MaximumIntegralDigits = NumRows >= 10 ? 2 : 1;

	int32 CurrentIndex = 0;
	int32 CurrentRow = NumRows - 1;

	for (const int32 i : Matrix)
	{
		FString Number = FText::AsNumber(CurrentIndex, &Options).ToString();
		Line += (i == 1 ? Number : Xs) + " ";
		if ((CurrentIndex + 1) % NumCols == 0 && (CurrentIndex + 1) >= NumCols)
		{
			Lines[CurrentRow] = Line;
			CurrentRow--;
			Line.Empty();
		}
		CurrentIndex++;
	}
	Line.Empty();

	FString DashedLine;
	FString ColumnNumberLine = "Cols";

	for (int i = 0; i < RowNumberOptions.MinimumIntegralDigits; i++)
	{
		ColumnNumberLine += " ";
	}

	ColumnNumberLine += "| ";

	for (int i = 0; i < NumCols; i++)
	{
		ColumnNumberLine += FText::AsNumber(i, &Options).ToString() + " ";
	}

	for (int i = 0; i < ColumnNumberLine.Len() - 1; i++)
	{
		DashedLine += "-";
	}

	UE_LOG(LogTargetManager, Display, TEXT("Index Validity:"));
	UE_LOG(LogTargetManager, Display, TEXT("%s"), *ColumnNumberLine);
	UE_LOG(LogTargetManager, Display, TEXT("%s"), *DashedLine);
	CurrentRow = NumRows - 1;
	for (auto String : Lines)
	{
		FString CurrentRowString = FText::AsNumber(CurrentRow, &RowNumberOptions).ToString();
		UE_LOG(LogTargetManager, Display, TEXT("Row %s| %s"), *CurrentRowString, *String);
		CurrentRow--;
	}
}

#endif

/*TArray<USpawnArea*> InvalidSpawnAreasArr = InvalidSpawnAreas.Array();
UE::FSpinLock Lock;
ParallelFor(InvalidSpawnAreasArr.Num(), [&](const int32 Index)
{
	const FVector Scale = InvalidSpawnAreasArr[Index]->GetTargetScale().Length() > NewScale.Length()
		? InvalidSpawnAreasArr[Index]->GetTargetScale()
		: NewScale;
	auto Vertices = InvalidSpawnAreasArr[Index]->MakeOccupiedVertices(Scale);
	UE::TScopeLock ScopeLock(Lock);
	Invalid.Append(MoveTemp(Vertices));
});*/
