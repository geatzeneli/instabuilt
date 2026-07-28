// WorldPresentation.h — VS M5+M6: World manager, Camera interfaces, UI data, Audio events
// M5: District/plot creation, showcase setup
// M6: Camera modes, UI screen specs, audio trigger definitions
// M7: Full assembly controller

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/WorldData.h"
#include "Components/CompanyComponents.h"

// ============================================================
// CAMERA MODE DEFINITIONS (M6 — 7 modes per GSS Section 2)
// ============================================================

/** Camera mode identifiers */
UENUM()
enum class ECameraMode : uint8
{
	Orbit,          // Default: isometric-adjacent, orbit around focal point
	Free,           // Unconstrained 3D movement
	FirstPerson,    // Eye-level walkthrough
	Drone,          // Flying camera drone
	Interior,       // Inside buildings, transparent walls
	Cinematic,      // Pre-set cinematic angles
	Site            // Fixed site cameras with construction overlay
};

/** Camera mode activation event */
struct INSTABUILTSIMULATION_API FCameraModeEvent : public FGameEvent
{
	ECameraMode Mode;
	FString Description;
	FEntityId FocusTarget;     // Entity to focus on
	INSTABUILT_EVENT(CameraModeChanged)
};

/** Camera mode configuration */
struct INSTABUILTSIMULATION_API C_CameraConfig : public FComponentBase
{
	ECameraMode CurrentMode = ECameraMode::Orbit;
	ECameraMode PreviousMode = ECameraMode::Orbit;
	
	// Orbit camera
	float OrbitDistance = 40.0f;
	float OrbitHeight = 25.0f;
	float OrbitAngle = 45.0f;
	
	// First person
	float FOV = 90.0f;
	float WalkSpeed = 5.0f;
	float RunSpeed = 12.0f;
	
	// Drone
	float DroneAltitude = 50.0f;
	float DroneSpeed = 8.0f;
	bool bDroneRecording = false;
	
	// Bookmarks
	TMap<FString, FVector> Bookmarks; // Named camera positions
	
	virtual FString GetTypeName() const override { return TEXT("CameraConfig"); }
	virtual void Serialize(FArchive& Ar) override
	{
		int32 ModeInt = (int32)CurrentMode; Ar << ModeInt;
		CurrentMode = (ECameraMode)ModeInt;
		Ar << OrbitDistance << OrbitHeight << OrbitAngle << FOV << WalkSpeed << RunSpeed;
	}
};

// ============================================================
// UI SCREEN SPECIFICATIONS (M6 — GSS Section 4)
// ============================================================

/** UI screen identifiers for data binding */
UENUM()
enum class EUIScreen : uint8
{
	MainMenu,
	CompanyDashboard,
	ContractBrowser,
	BuildingDesigner,
	ConstructionMonitor,
	FinancialReport,
	ProjectCompletion,
	SaveLoadMenu,
	SettingsMenu
};

/** UI data binding contract — what data each screen needs */
struct INSTABUILTSIMULATION_API C_UIScreenData : public FComponentBase
{
	EUIScreen ActiveScreen = EUIScreen::MainMenu;
	
	// Data fields required by each screen (keys match UI widget bindings)
	TMap<FString, FString> BoundData;
	
	void SetData(const FString& Key, const FString& Value)
	{
		BoundData.Add(Key, Value);
	}
	
	FString GetData(const FString& Key) const
	{
		const FString* Found = BoundData.Find(Key);
		return Found ? *Found : TEXT("");
	}
	
	virtual FString GetTypeName() const override { return TEXT("UIScreenData"); }
	virtual void Serialize(FArchive& Ar) override
	{
		int32 ScreenInt = (int32)ActiveScreen; Ar << ScreenInt;
		ActiveScreen = (EUIScreen)ScreenInt;
	}
};

/** UI notification event */
struct INSTABUILTSIMULATION_API FUIEvent : public FGameEvent
{
	EUIScreen TargetScreen;
	FString Action;           // "Open", "Close", "Refresh", "Highlight"
	FString DataKey;
	FString DataValue;
	INSTABUILT_EVENT(UIEvent)
};

// ============================================================
// AUDIO EVENT DEFINITIONS (M6)
// ============================================================

/** Audio event types for Wwise integration */
UENUM()
enum class EAudioEvent : uint8
{
	// UI
	UI_ButtonClick,
	UI_PanelOpen,
	UI_PanelClose,
	UI_Notification,
	UI_Achievement,
	
	// Construction
	Construction_Ambient,
	Construction_Hammering,
	Construction_Excavation,
	Construction_Crane,
	Construction_PhaseComplete,
	Construction_Complete,
	
	// Environment
	Env_Morning,
	Env_Afternoon,
	Env_Night,
	Env_Rain,
	Env_Wind,
	
	// Music
	Music_Menu,
	Music_Design,
	Music_Construction,
	Music_Completion,
	Music_Dashboard,
	
	// Feedback
	SFX_MoneyEarned,
	SFX_MoneySpent,
	SFX_ReputationUp,
	SFX_ReputationDown,
	SFX_ContractAccepted,
	SFX_InspectionPass,
	SFX_InspectionFail
};

/** Audio trigger event */
struct INSTABUILTSIMULATION_API FAudioTriggerEvent : public FGameEvent
{
	EAudioEvent AudioEvent;
	float Volume = 1.0f;
	float Pitch = 1.0f;
	FVector Location;          // 3D position for spatial audio
	bool bIs3D = false;
	FString RTPCName;          // Real-Time Parameter Control (Wwise)
	float RTPCValue = 0.0f;
	INSTABUILT_EVENT(AudioTrigger)
};

/** Music state event */
struct INSTABUILTSIMULATION_API FMusicStateEvent : public FGameEvent
{
	FString StateGroup;        // "GameMode", "ConstructionPhase", "TimeOfDay"
	FString StateValue;        // "Design", "Foundation", "Morning"
	float TransitionTime = 2.0f;
	INSTABUILT_EVENT(MusicStateChanged)
};

// ============================================================
// WORLD MANAGER (M5) — District/Plot/Showcase creation
// ============================================================

class INSTABUILTSIMULATION_API FWorldManager : public FInstaBuiltSystem
{
public:
	FWorldManager();
	virtual void OnInitialize() override;
	
	/** Create the vertical slice showcase district */
	FEntityId CreateShowcaseDistrict();
	
	/** Create a construction plot */
	FEntityId CreatePlot(const FString& Address, float X, float Y, float Width, float Depth);
	
	/** Create a road segment */
	FEntityId CreateRoad(const FString& Name, const FString& Type, float SX, float SY, float EX, float EY);
	
	/** Get district summary */
	FString GetDistrictSummary() const;
	
	/** Get showcase plot info for the design screen */
	FString GetShowcasePlotInfo() const;
	
	/** Place the player's building on a plot */
	bool PlaceBuilding(FEntityId BuildingId, FEntityId PlotId);
	
	/** Get all available plots */
	TArray<FEntityId> GetAvailablePlots() const;
	
	/** Get plot details */
	FString GetPlotDetails(FEntityId PlotId) const;
	
private:
	FEntityId DistrictId;
	TArray<FEntityId> Plots;
	TArray<FEntityId> Roads;
};

// ============================================================
// VERTICAL SLICE ASSEMBLER (M7) — Full journey controller
// ============================================================

class INSTABUILTSIMULATION_API FVerticalSliceAssembler : public FInstaBuiltSystem
{
public:
	FVerticalSliceAssembler();
	virtual void OnInitialize() override;
	
	// ============================================================
	// FULL JOURNEY COMMANDS
	// ============================================================
	
	/** Start the complete vertical slice experience */
	FString Cmd_StartVerticalSlice();
	
	/** Run the full demo with preset choices */
	FString Cmd_RunDemo();
	
	/** Get the current step in the journey */
	int32 GetCurrentStep() const { return CurrentStep; }
	
	/** Get journey status */
	FString GetJourneyStatus() const;
	
	/** Get full step-by-step walkthrough */
	FString GetWalkthrough() const;
	
private:
	int32 CurrentStep = 0;
	bool bSliceStarted = false;
	
	// Journey steps
	enum class EStep
	{
		CreateCompany,
		ReceiveContract,
		ReviewRequirements,
		DesignBuilding,
		SubmitBlueprint,
		PrepareConstruction,
		AssignWorkers,
		MonitorConstruction,
		InspectComplete,
		ClientApproval,
		ReceivePayment,
		ReputationIncrease,
		SaveGame,
		Complete
	};
	
	EStep GetStepEnum() const { return (EStep)CurrentStep; }
	FString StepToString(EStep Step) const;
	
	// Internal flow control
	FString ExecuteStep(EStep Step);
	void AdvanceStep();
};
