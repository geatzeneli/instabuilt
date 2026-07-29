// BuildingValidator.h — Phase 6 M2: Complete building validation system
// GSS Section 6 Step 13: Design Validation Report
// Validates: structural, code compliance, accessibility, energy, client requirements

#pragma once
#include "CoreMinimal.h"
#include "ECS/InstaBuiltSystem.h"
#include "Components/BuildingDesign.h"
#include "Components/ProductLines.h"
#include "Components/QualityComponents.h"

/**
 * FBuildingValidator
 * 
 * Centralized validation engine. Takes a building design and runs
 * all applicable validation rules. Returns structured results.
 * 
 * Validation categories:
 * 1. Structural integrity (load paths, spans, foundations)
 * 2. Code compliance (zoning, fire, egress, seismic, wind)
 * 3. Accessibility (ADA/EN standards)
 * 4. Energy efficiency (KfW ratings, insulation, HVAC)
 * 5. Client requirements (rooms, area, features, materials)
 */
class INSTABUILTSIMULATION_API FBuildingValidator : public FInstaBuiltSystem
{
public:
	FBuildingValidator();
	
	// ============================================================
	// VALIDATION RESULT
	// ============================================================
	
	struct FValidationIssue
	{
		FString Category;        // "Structural", "Code", "Accessibility", "Energy", "Client"
		FString Severity;        // "Error", "Warning", "Info"
		FString Description;
		FString FixSuggestion;
		FEntityId RelatedEntity; // Wall, room, etc. that caused the issue
	};
	
	struct FValidationReport
	{
		bool bPassed = false;
		TArray<FValidationIssue> Issues;
		int32 ErrorCount = 0;
		int32 WarningCount = 0;
		
		float StructuralScore = 0.0f;
		float CodeComplianceScore = 0.0f;
		float AccessibilityScore = 0.0f;
		float EnergyScore = 0.0f;
		float ClientRequirementScore = 0.0f;
		float OverallScore = 0.0f;
		
		FString GetSummary() const;
		FString GetDetailedReport() const;
	};
	
	// ============================================================
	// VALIDATION API
	// ============================================================
	
	/** Run all validation on a building design */
	FValidationReport ValidateDesign(FEntityId BuildingId) const;
	
	/** Run a specific validation category */
	void ValidateStructural(FEntityId BuildingId, FValidationReport& Report) const;
	void ValidateCodeCompliance(FEntityId BuildingId, FValidationReport& Report) const;
	void ValidateAccessibility(FEntityId BuildingId, FValidationReport& Report) const;
	void ValidateEnergyEfficiency(FEntityId BuildingId, FValidationReport& Report) const;
	void ValidateClientRequirements(FEntityId BuildingId, FValidationReport& Report) const;
	
	// ============================================================
	// PRODUCT LINE VALIDATION
	// ============================================================
	
	/** Validate POP UP constraints */
	void ValidatePopUp(FEntityId BuildingId, FValidationReport& Report) const;
	
	/** Validate multifamily requirements */
	void ValidateMultifamily(FEntityId BuildingId, FValidationReport& Report) const;
	
	/** Validate senior housing accessibility requirements */
	void ValidateSeniorHousing(FEntityId BuildingId, FValidationReport& Report) const;
	
	/** Validate micro apartment density */
	void ValidateMicroApartments(FEntityId BuildingId, FValidationReport& Report) const;
	
	/** Validate traditional home style authenticity */
	void ValidateTraditionalHome(FEntityId BuildingId, FValidationReport& Report) const;
	
	/** Validate signature home minimum budget */
	void ValidateSignatureHome(FEntityId BuildingId, FValidationReport& Report) const;
	
	/** Validate bathpod compatibility */
	void ValidateBathpod(FEntityId BuildingId, FValidationReport& Report) const;
	
	// ============================================================
	// HELPERS
	// ============================================================
	
	/** Get the product line type from a building entity */
	static FString GetProductLine(FEntityId BuildingId);
	
	/** Get all required rooms for a product line */
	static TArray<FString> GetRequiredRooms(const FString& ProductLine);
	
	/** Calculate energy efficiency rating (KfW scale) */
	static float CalculateEnergyRating(FEntityId BuildingId);
	
	/** Calculate structural integrity score */
	static float CalculateStructuralScore(FEntityId BuildingId);
	
private:
	// Default span limits per structural system
	TMap<FString, float> MaxSpans;
	
	// Energy rating thresholds
	static constexpr float KfW40_Threshold = 40.0f;
	static constexpr float KfW55_Threshold = 55.0f;
	static constexpr float KfW70_Threshold = 70.0f;
	
	/** Add an issue to the report */
	void AddIssue(FValidationReport& Report, const FString& Category,
		const FString& Severity, const FString& Desc, const FString& Fix,
		FEntityId Entity = FEntityId::Invalid()) const;
};
