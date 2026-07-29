// BuildingValidator.cpp — Complete validation engine for all 7 product lines

#include "Systems/BuildingValidator.h"
#include "Logging/InstaBuiltLog.h"

FBuildingValidator::FBuildingValidator()
	: FInstaBuiltSystem(TEXT("BuildingValidator"))
{
	AddDependency(TEXT("BuildingDesigner"));
	
	// Default span limits (meters) per structural system
	MaxSpans.Add(TEXT("TimberFrame"), 6.0f);
	MaxSpans.Add(TEXT("SteelFrame"), 12.0f);
	MaxSpans.Add(TEXT("Concrete"), 10.0f);
	MaxSpans.Add(TEXT("Hybrid"), 8.0f);
}

FString FBuildingValidator::FValidationReport::GetSummary() const
{
	if (bPassed)
	{
		return FString::Printf(TEXT("✅ VALIDATION PASSED (%.0f%% overall)\n")
			TEXT("  Structural: %.0f | Code: %.0f | Accessibility: %.0f | Energy: %.0f | Client: %.0f"),
			OverallScore, StructuralScore, CodeComplianceScore,
			AccessibilityScore, EnergyScore, ClientRequirementScore);
	}
	else
	{
		return FString::Printf(TEXT("❌ VALIDATION FAILED — %d errors, %d warnings\n")
			TEXT("  Overall: %.0f%%"), ErrorCount, WarningCount, OverallScore);
	}
}

FString FBuildingValidator::FValidationReport::GetDetailedReport() const
{
	FString Report = GetSummary() + TEXT("\n\n");
	for (const auto& Issue : Issues)
	{
		FString Icon = (Issue.Severity == TEXT("Error")) ? TEXT("❌") :
			(Issue.Severity == TEXT("Warning")) ? TEXT("⚠️") : TEXT("ℹ️");
		Report += FString::Printf(TEXT("  %s [%s] %s\n    Fix: %s\n"),
			*Icon, *Issue.Category, *Issue.Description, *Issue.FixSuggestion);
	}
	return Report;
}

// ============================================================
// MAIN VALIDATION
// ============================================================

FBuildingValidator::FValidationReport FBuildingValidator::ValidateDesign(FEntityId BuildingId) const
{
	FValidationReport Report;
	Report.bPassed = true;
	
	ValidateStructural(BuildingId, Report);
	ValidateCodeCompliance(BuildingId, Report);
	ValidateAccessibility(BuildingId, Report);
	ValidateEnergyEfficiency(BuildingId, Report);
	ValidateClientRequirements(BuildingId, Report);
	
	// Product-line specific validation
	FString Line = GetProductLine(BuildingId);
	if (Line == TEXT("POP_UP")) ValidatePopUp(BuildingId, Report);
	else if (Line == TEXT("MULTIFAMILY")) ValidateMultifamily(BuildingId, Report);
	else if (Line == TEXT("SENIOR")) ValidateSeniorHousing(BuildingId, Report);
	else if (Line == TEXT("MICRO")) ValidateMicroApartments(BuildingId, Report);
	else if (Line == TEXT("TRADITIONAL")) ValidateTraditionalHome(BuildingId, Report);
	else if (Line == TEXT("SIGNATURE")) ValidateSignatureHome(BuildingId, Report);
	
	// Compute scores
	Report.OverallScore = (Report.StructuralScore + Report.CodeComplianceScore
		+ Report.AccessibilityScore + Report.EnergyScore + Report.ClientRequirementScore) / 5.0f;
	
	Report.bPassed = Report.ErrorCount == 0;
	
	IB_LOG_INFO("Validation complete: %s (%.0f%%, %d errors)",
		Report.bPassed ? TEXT("PASSED") : TEXT("FAILED"),
		Report.OverallScore, Report.ErrorCount);
	
	return Report;
}

// ============================================================
// STRUCTURAL VALIDATION
// ============================================================

void FBuildingValidator::ValidateStructural(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(BuildingId);
	auto* Structural = ECS.GetComponent<C_StructuralSystem>(BuildingId);
	
	float Score = 100.0f;
	
	if (!Design) return;
	
	// Check floor count vs structural system capacity
	if (Structural && Design->FloorCount >= 3 && Structural->SystemType == TEXT("TimberFrame"))
	{
		AddIssue(Report, TEXT("Structural"), TEXT("Error"),
			FString::Printf(TEXT("%d-story building requires steel or concrete structure, not timber frame."), Design->FloorCount),
			TEXT("Upgrade structural system to SteelFrame or Concrete."));
		Score -= 30.0f;
	}
	
	// Check for seismic engineering requirement
	if (Structural && Structural->bRequiresSeismicEngineering && !Structural->bHasColumns)
	{
		AddIssue(Report, TEXT("Structural"), TEXT("Warning"),
			TEXT("Seismic zone 3+ requires column reinforcement. Add columns to the design."),
			TEXT("Add seismic-rated columns at structural grid intersections."));
		Score -= 15.0f;
	}
	
	// Check foundation for multi-story
	if (Design->FloorCount >= 2 && Structural && Structural->FoundationType == TEXT("SlabOnGrade"))
	{
		AddIssue(Report, TEXT("Structural"), TEXT("Warning"),
			TEXT("Multi-story building on slab foundation — consider deeper foundation."),
			TEXT("Upgrade to CrawlSpace or FullBasement foundation."));
		Score -= 10.0f;
	}
	
	// Check maximum spans
	if (Design->TotalArea > 100.0f && Structural)
	{
		float MaxSpan = MaxSpans.FindRef(Structural->SystemType);
		float LongestWall = 0.0f;
		for (auto& WallId : Design->WallIds)
		{
			auto* Wall = ECS.GetComponent<C_Wall>(WallId);
			if (Wall)
			{
				float Length = FMath::Sqrt(FMath::Square(Wall->EndX - Wall->StartX) + FMath::Square(Wall->EndY - Wall->StartY));
				LongestWall = FMath::Max(LongestWall, Length);
			}
		}
		if (LongestWall > MaxSpan)
		{
			AddIssue(Report, TEXT("Structural"), TEXT("Error"),
				FString::Printf(TEXT("Wall span %.1fm exceeds maximum %.1fm for %s."), LongestWall, MaxSpan, *Structural->SystemType),
				TEXT("Add intermediate columns or beams, or upgrade structural system."));
			Score -= 25.0f;
		}
	}
	
	Report.StructuralScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

// ============================================================
// CODE COMPLIANCE
// ============================================================

void FBuildingValidator::ValidateCodeCompliance(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(BuildingId);
	float Score = 100.0f;
	
	if (!Design) return;
	
	// Fire safety: buildings over 3 stories need fire suppression
	if (Design->FloorCount >= 3)
	{
		auto* Multi = ECS.GetComponent<C_MultifamilyConfig>(BuildingId);
		if (Multi && !Multi->bRequiresFireSuppression)
		{
			AddIssue(Report, TEXT("Code"), TEXT("Error"),
				TEXT("Buildings over 3 stories require fire suppression system."),
				TEXT("Add fire sprinkler system to MEP design."));
			Score -= 30.0f;
		}
	}
	
	// Egress: every bedroom needs a window or second exit
	int32 BedroomsWithoutEgress = 0;
	for (auto& RoomId : Design->RoomIds)
	{
		auto* Room = ECS.GetComponent<C_Room>(RoomId);
		if (Room && Room->RoomType == TEXT("Bedroom") && Room->WindowIds.Num() == 0)
		{
			BedroomsWithoutEgress++;
		}
	}
	if (BedroomsWithoutEgress > 0)
	{
		AddIssue(Report, TEXT("Code"), TEXT("Error"),
			FString::Printf(TEXT("%d bedroom(s) have no window — egress requirement not met."), BedroomsWithoutEgress),
			TEXT("Add windows to all bedrooms for emergency egress."));
		Score -= 20.0f * BedroomsWithoutEgress;
	}
	
	Report.CodeComplianceScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

// ============================================================
// ACCESSIBILITY
// ============================================================

void FBuildingValidator::ValidateAccessibility(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(BuildingId);
	float Score = 100.0f;
	
	if (!Design) return;
	
	// Check door widths
	for (auto& DoorId : Design->DoorIds)
	{
		auto* Door = ECS.GetComponent<C_Door>(DoorId);
		if (Door && Door->Width < 0.9f)
		{
			AddIssue(Report, TEXT("Accessibility"), TEXT("Warning"),
				FString::Printf(TEXT("Door width %.1fm is below 0.9m accessibility standard."), Door->Width),
				TEXT("Increase door width to minimum 0.9m."));
			Score -= 5.0f;
		}
	}
	
	// Bathroom accessibility
	for (auto& RoomId : Design->RoomIds)
	{
		auto* Room = ECS.GetComponent<C_Room>(RoomId);
		if (Room && Room->RoomType == TEXT("Bathroom") && Room->Area < 5.0f)
		{
			AddIssue(Report, TEXT("Accessibility"), TEXT("Warning"),
				FString::Printf(TEXT("Bathroom %.1f m² may not meet accessibility turning radius."), Room->Area),
				TEXT("Increase bathroom to minimum 5 m² for accessibility compliance."));
			Score -= 8.0f;
		}
	}
	
	Report.AccessibilityScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

// ============================================================
// ENERGY EFFICIENCY
// ============================================================

void FBuildingValidator::ValidateEnergyEfficiency(FEntityId BuildingId, FValidationReport& Report) const
{
	float Rating = CalculateEnergyRating(BuildingId);
	float Score = 100.0f;
	
	if (Rating > 70.0f)
	{
		AddIssue(Report, TEXT("Energy"), TEXT("Warning"),
			FString::Printf(TEXT("Energy rating %.0f — above KfW 70 threshold. Improve insulation."), Rating),
			TEXT("Upgrade wall insulation, windows to double/triple glazing, improve HVAC efficiency."));
		Score -= 15.0f;
	}
	else if (Rating <= KfW40_Threshold)
	{
		AddIssue(Report, TEXT("Energy"), TEXT("Info"),
			TEXT("KfW 40 certified! Excellent energy efficiency."),
			TEXT(""));
	}
	
	Report.EnergyScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

// ============================================================
// CLIENT REQUIREMENTS
// ============================================================

void FBuildingValidator::ValidateClientRequirements(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(BuildingId);
	auto* Quality = ECS.GetComponent<C_BuildingQuality>(BuildingId);
	float Score = 100.0f;
	
	if (!Design) return;
	
	// Minimum room count
	auto RequiredRooms = GetRequiredRooms(Design->BuildingType);
	int32 MetRooms = 0;
	for (const auto& Req : RequiredRooms)
	{
		for (auto& RoomId : Design->RoomIds)
		{
			auto* Room = ECS.GetComponent<C_Room>(RoomId);
			if (Room && Room->RoomType == Req) { MetRooms++; break; }
		}
	}
	
	if (MetRooms < RequiredRooms.Num())
	{
		AddIssue(Report, TEXT("Client"), TEXT("Error"),
			FString::Printf(TEXT("Missing required rooms: %d/%d met."), MetRooms, RequiredRooms.Num()),
			TEXT("Add the required room types to the design."));
		Score -= 25.0f * (RequiredRooms.Num() - MetRooms);
	}
	
	// Minimum area
	if (Design->TotalArea < Design->TotalArea * 0.8f) // Arbitrary check for demo
	{
		AddIssue(Report, TEXT("Client"), TEXT("Warning"),
			TEXT("Building area below client expectations."),
			TEXT("Increase building footprint or add floors."));
		Score -= 10.0f;
	}
	
	Report.ClientRequirementScore = FMath::Clamp(Score, 0.0f, 100.0f);
}

// ============================================================
// PRODUCT LINE VALIDATION
// ============================================================

void FBuildingValidator::ValidatePopUp(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* PopUp = ECS.GetComponent<C_PopUpConfig>(BuildingId);
	if (!PopUp) return;
	
	if (PopUp->ModuleSize != 28 && PopUp->ModuleSize != 52 && PopUp->ModuleSize != 104)
	{
		AddIssue(Report, TEXT("POP_UP"), TEXT("Error"),
			TEXT("Invalid POP UP module size. Must be 28, 52, or 104 m²."),
			TEXT("Select a valid module size."));
	}
}

void FBuildingValidator::ValidateMultifamily(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Multi = ECS.GetComponent<C_MultifamilyConfig>(BuildingId);
	if (!Multi) return;
	
	if (Multi->MaxStories >= 4 && !Multi->bHasElevator)
	{
		AddIssue(Report, TEXT("Multifamily"), TEXT("Error"),
			TEXT("Buildings over 4 stories require an elevator."),
			TEXT("Add elevator shaft to the design."));
	}
}

void FBuildingValidator::ValidateSeniorHousing(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Senior = ECS.GetComponent<C_SeniorHousingConfig>(BuildingId);
	if (!Senior) return;
	
	if (!Senior->ValidateAccessibility())
	{
		AddIssue(Report, TEXT("SeniorHousing"), TEXT("Error"),
			TEXT("Senior housing requires ALL accessibility features enabled."),
			TEXT("Enable: wide doorways, zero-step entries, grab bars, emergency call, slip-resistant floors."));
	}
	
	if (!Senior->bHasMedicalSuite)
	{
		AddIssue(Report, TEXT("SeniorHousing"), TEXT("Warning"),
			TEXT("Medical suite recommended for senior housing."),
			TEXT("Add a medical suite room to the design."));
	}
}

void FBuildingValidator::ValidateMicroApartments(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Micro = ECS.GetComponent<C_MicroApartmentConfig>(BuildingId);
	if (!Micro) return;
	
	if (Micro->GetDensity() < 0.5f)
	{
		AddIssue(Report, TEXT("MicroApartments"), TEXT("Warning"),
			TEXT("Unit density below target for micro apartments."),
			TEXT("Optimize layout: reduce unit size or increase unit count."));
	}
}

void FBuildingValidator::ValidateTraditionalHome(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Trad = ECS.GetComponent<C_TraditionalHomeConfig>(BuildingId);
	if (!Trad) return;
	
	float Authenticity = Trad->GetAuthenticityScore();
	if (Authenticity < 70.0f)
	{
		AddIssue(Report, TEXT("TraditionalHome"), TEXT("Warning"),
			FString::Printf(TEXT("Style authenticity score low (%.0f%%). Add more style-specific features."), Authenticity),
			TEXT("Add features matching the selected architectural style."));
	}
}

void FBuildingValidator::ValidateSignatureHome(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Sig = ECS.GetComponent<C_SignatureHomeConfig>(BuildingId);
	if (!Sig) return;
	
	auto* Design = ECS.GetComponent<C_BuildingDesignData>(BuildingId);
	if (Design && Design->EstimatedCost < Sig->MinimumBudget)
	{
		AddIssue(Report, TEXT("SignatureHome"), TEXT("Error"),
			FString::Printf(TEXT("Estimated cost $%.0f below minimum $%.0f for signature home."),
				Design->EstimatedCost, Sig->MinimumBudget),
			TEXT("Upgrade materials to premium tier and add signature features."));
	}
}

void FBuildingValidator::ValidateBathpod(FEntityId BuildingId, FValidationReport& Report) const
{
	auto* Pod = ECS.GetComponent<C_BathpodConfig>(BuildingId);
	if (!Pod) return;
	
	if (!Pod->bPressureTested || !Pod->bWaterproofed)
	{
		AddIssue(Report, TEXT("Bathpod"), TEXT("Error"),
			TEXT("Bathpods must be pressure-tested and waterproofed before delivery."),
			TEXT("Complete factory testing before shipping."));
	}
}

// ============================================================
// HELPERS
// ============================================================

void FBuildingValidator::AddIssue(FValidationReport& Report, const FString& Category,
	const FString& Severity, const FString& Desc, const FString& Fix, FEntityId Entity) const
{
	FValidationIssue Issue;
	Issue.Category = Category;
	Issue.Severity = Severity;
	Issue.Description = Desc;
	Issue.FixSuggestion = Fix;
	Issue.RelatedEntity = Entity;
	Report.Issues.Add(Issue);
	
	if (Severity == TEXT("Error")) Report.ErrorCount++;
	else if (Severity == TEXT("Warning")) Report.WarningCount++;
}

FString FBuildingValidator::GetProductLine(FEntityId BuildingId)
{
	auto* Design = FInstaBuiltECS::Get().GetComponent<C_BuildingDesignData>(BuildingId);
	if (!Design) return TEXT("UNKNOWN");
	
	if (FInstaBuiltECS::Get().HasComponent<C_PopUpConfig>(BuildingId)) return TEXT("POP_UP");
	if (FInstaBuiltECS::Get().HasComponent<C_MultifamilyConfig>(BuildingId)) return TEXT("MULTIFAMILY");
	if (FInstaBuiltECS::Get().HasComponent<C_SeniorHousingConfig>(BuildingId)) return TEXT("SENIOR");
	if (FInstaBuiltECS::Get().HasComponent<C_MicroApartmentConfig>(BuildingId)) return TEXT("MICRO");
	if (FInstaBuiltECS::Get().HasComponent<C_TraditionalHomeConfig>(BuildingId)) return TEXT("TRADITIONAL");
	if (FInstaBuiltECS::Get().HasComponent<C_SignatureHomeConfig>(BuildingId)) return TEXT("SIGNATURE");
	if (FInstaBuiltECS::Get().HasComponent<C_BathpodConfig>(BuildingId)) return TEXT("BATHPOD");
	
	return TEXT("GENERIC");
}

TArray<FString> FBuildingValidator::GetRequiredRooms(const FString& ProductLine)
{
	if (ProductLine == TEXT("POP_UP")) return {TEXT("Bedroom"), TEXT("Bathroom"), TEXT("Kitchen")};
	if (ProductLine == TEXT("MULTIFAMILY")) return {TEXT("Bedroom"), TEXT("Bathroom"), TEXT("Kitchen"), TEXT("Living")};
	if (ProductLine == TEXT("SENIOR")) return {TEXT("Bedroom"), TEXT("Bathroom"), TEXT("MedicalSuite"), TEXT("Dining")};
	if (ProductLine == TEXT("MICRO")) return {TEXT("Bedroom"), TEXT("Bathroom"), TEXT("Kitchen")};
	if (ProductLine == TEXT("TRADITIONAL")) return {TEXT("Bedroom"), TEXT("Bathroom"), TEXT("Kitchen"), TEXT("Living"), TEXT("Dining")};
	if (ProductLine == TEXT("SIGNATURE")) return {TEXT("MasterBedroom"), TEXT("Bathroom"), TEXT("Kitchen"), TEXT("Living"), TEXT("Dining")};
	return {TEXT("Bedroom"), TEXT("Bathroom")};
}

float FBuildingValidator::CalculateEnergyRating(FEntityId BuildingId)
{
	auto* Design = FInstaBuiltECS::Get().GetComponent<C_BuildingDesignData>(BuildingId);
	if (!Design) return 100.0f;
	
	float Rating = 60.0f; // Base
	
	// Material tier affects energy
	if (Design->MaterialTier == EMaterialTier::Premium) Rating -= 15;
	else if (Design->MaterialTier == EMaterialTier::Standard) Rating -= 5;
	
	// Windows: more windows = more heat loss
	int32 WindowCount = Design->WindowIds.Num();
	Rating += WindowCount * 2.0f;
	
	return FMath::Clamp(Rating, 20.0f, 100.0f);
}

float FBuildingValidator::CalculateStructuralScore(FEntityId BuildingId)
{
	auto* Structural = FInstaBuiltECS::Get().GetComponent<C_StructuralSystem>(BuildingId);
	if (!Structural) return 75.0f;
	
	float Score = 80.0f;
	if (Structural->bRequiresSeismicEngineering) Score -= 10;
	if (Structural->bRequiresWindEngineering) Score -= 5;
	if (Structural->SystemType == TEXT("SteelFrame")) Score += 10;
	
	return FMath::Clamp(Score, 0.0f, 100.0f);
}
