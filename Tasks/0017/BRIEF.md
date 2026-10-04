# 0017 — Pure navigation planner for local/galactic distances
Owner Codex navigationhelper, base81fd5d1, codex/solar-flight.
Read README,ENVIRONMENT,FOUNDATION,ARCHITECTURE,CODING_RULES,CURRENT_STATE,
Tasks/README. Pure singleplayer value model. No UI/app/Git/build ownership.
Allowed ONLY:
Game/Daedalus/Source/DaedalusSimulation/Public/DaedalusNavigationModel.h
Game/Daedalus/Source/DaedalusSimulation/Private/DaedalusNavigationModel.cpp
Game/Daedalus/Source/DaedalusSimulation/Private/Tests/DaedalusNavigationTests.cpp
Tasks/0017/PROGRESS.md,HANDOFF.md.
Root commits/builds/tests. Implement only after root confirms C++ freeze lifted.

FNavigationBody: Id,Name,ParentId,PositionMetres,RadiusMetres,bKnownRadius;
unknownradius0 withfalse flag, finite local doubles abs<=1e15.
FNavigationSystem: Id,Name,GalaxyPositionLightYears, TArray bodies.
FNavigationPlan: configure validatedimmutable catalog, active system+shiplocal
location, canonicaltargetID pair, querymetrics withoutmutation.
API:
Configure(const TArray<FNavigationSystem>&, FString& Error)
SetLocation(const FString& SystemId,const FVector3d&, FString& Error)
SelectTarget(const FString& SystemId,const FString& BodyId,FString& Error)
GetTargetSystemId/GetTargetBodyId/GetLocationSystemId const getters
Query(double CurrentSpeed,double PlannedSpeed,double DesiredSeconds) const
=> FNavigationMetrics fields bValid,bCurrentETA,bPlannedETA,DistanceMetres,
Direction (normalizedlocalgalaxybasis vector),CurrentETASeconds,PlannedETASeconds,
RequiredSpeedMetresPerSecond, bRequiredSpeed (onlywhenDesiredSeconds>0).
No travel,teleport,energy orpositionmutationwhen selectingtarget. Configure
atomicrollback invalidIDs/NaN/duplicateparent/bounds, uniqueSystem and perbodyIDs,
validparent references/no cycles, no falsephysicalradii for unknowns. Validate
bounded catalog (10000systems,10000bodies/sys withpracticaltotalguard).
Configure success picksfirstsystem andbody, locationzero, targetfirstbody.

Same-system distance computed directly fromlocalmetres first (avoid precision
loss atgalacticcoords). Acrosssystems difference galaxyLY*9460730472580800.0 +
targetLocalPos -shipLocalPos. Distance is centre-to-centre, clearlynamed.
Zero/negative/nonfinite speed gives unavailable ETA, neverNaN/Inf or divide0.
Absolute positive speed only (root passesabsvelocity); planned speed forestimates
can be impulse or future hyper settings without changingflight physics. Desired
seconds>0 yields required effective speed, otherwise flagfalse. Zero-distanceETA0
whenpositive speed. Query noneffectiveinvalidnumerics gracefullyfalseflag.

Meaningful automation groups Daedalus.Navigation.CatalogRollback,
LocalGalacticDistances,ETABoundaries,SelectionLocationCommands.
Check localcentimetreaccuracy whilegalaxyposition100000LY;1LY correctness,
smallseparations, vector direction, zero/invalid inputs, IDs/cycles/rollback,
targetselectiondoesnotmove, activelocationchange andcatalogrefresh semantics.
Don't modifyexistingflighttests. ExposedstructsvalueonlynoUObjects.
Finalhandoff actualfilechanges/checks, root integration/buildpending.
