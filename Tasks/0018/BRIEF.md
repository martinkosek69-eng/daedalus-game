# 0018 — Detailed interactive3Dgalaxy map presentation
Owner Codex maphelper, base6c8db7b, branch codex/solar-flight. Read shareddocs.
Pure presentationonly. Root owns worldstate/navigation/import/build/editor/Git.
AllowedONLY:
Game/Daedalus/Source/Daedalus/Solar/GalaxyMapView.h
Game/Daedalus/Source/Daedalus/Solar/GalaxyMapView.cpp
Tasks/0018/PROGRESS.md,HANDOFF.md.
No rootHeader/GameMode/Pawn/Probe/config/Tools edits. Coordinate interface early.

Implement ordinaryvalue renderer/inputviewclass FGalaxyMapView withviewstate
open flag, selectedsystem/body,pivotLY,cameradistance/yaw/pitch,desiredtripseconds.
These areview/preferences notactualshipstate. Emit commands root applies.
Interface agreed:
FGalaxyBodyView {FString Id,Name,Kind,SourceQuality; FVector3d PositionMetres;
 double RadiusMetres;bool bKnownRadius;UMaterialInterface* MapMaterial=nullptr;}
FGalaxySystemView {FString Id,Name;FVector3d GalaxyLightYears;
 bool bAvailable;TArray<FGalaxyBodyView> Bodies;}
FMapAction {enumType None/Browse/Navigate/Inspect;int32 SystemIndex,BodyIndex;}
FGalaxyMapView fields bOpen,SelectedSystem,SelectedBody,DesiredSeconds.
Input(FVector2D Cursor,FVector2D Delta,bool LeftPressed,bool RightHeld,
 bool MiddleHeld,float Wheel,bool Home,const TArray<FGalaxySystemView>&,
 FVector2D ViewportSize) -> FMapAction;
Draw(UCanvas*,UFont*,const TArray<FGalaxySystemView>&,int32 ActiveSystem,
 const Daedalus::FNavigationMetrics& PreviewMetrics);
MarkerScreenPosition(int32System,FVector2D& Out) const forrootpackagedtests.
FocusSystem(index,catalog); methods getters pivot/cameradistance optionaltests.
Root passesinputactualPlayerController pressedstate/mousedelta/wheelkeys,
Mtoggle handledroot,rootappliesreturnedNAVIGATE/INSPECT tocanonicalmodels.
Browse action root updatesseparatePreviewNavigation; selectionnevermovesship.

User asks beautiful illuminated 3D map,darkbackground,strongdetail,viewfromabove
andside,verydeepzoom todetails. Implement genuine3D pointdistribution/projection
of barredspiralgalaxy~100kLYdiameter/thindisk,bulge,subtlehalo,armsanddustlanes.
Plausibleillustrationnotexactstarcatalog. Staticseedpoints maybe20k–40k,
FCanvas batchedstars/polylines canrenderGPUwithout30kactors. Sortdepth/clipnear,
fadebydistance/LOD; avoidgiantpointstarswhenzooming. ClearOrbitRmouse,panning
MiddleMouse,wheel exponentialzoom fromwholegalaxy(~150kLY) downlocally
(~1e-8LY),Home resetgalaxy. Clampstablebutallowthroughplane cameraorbit±89deg.
Relativeprojection firstsubtractsystemGalaxyPos-PivotLY,thenaddlocalmetres/LY
toavoidlosingbodydetailswhenzoomednearfarawaygalaxylocations.
Viewshapes withwarmbulge/coolarms/subduedglow tasteful contrast, noopaque
colorednoisecloud. Panels/titlelabels/markersnativepixels wsharpRuntimeRoboto
FontInfo; drawblackplates/shadows behindtext. Native4K UI scale,minwidthheight.

Leftclicksystem marker choosesfocus/selection, buttonfocusrecenter+zoom,
listall6systems,scrollable/searchablebodydatabase (rootSol459+objects), selectable
rows,kind/radiusknownorunknown/sourceQuality. Showdistancecentre km/AU/LY,
ETAcurrent andfullimpulse,requiredspeedfor chosen1h/day/week durationcontrols.
Rootpassesmetrics, do notcompute/mutateflightdata. Navigatebutton emitsNav,
Testinspectbutton explicitlylabelledtest emitsInspect onlyif bAvailable;
nohyperdrive/energy implemented. IncomingmissingClaudeassetsmustshowwaiting,
neverpretendfinishedsource. Maplabelsandlistselectableevenwhensystemsoverlap;
hover/selectionglow,directsystemfocus aidscloseSol/Asterion positions.
Atdeepzoomshowselectedsystemtrue3Dlocalbodies/representativeorbitguidecircles
and texturedplanetdiscs if MapMaterial supplied; rootprovidesmaterialprojection
fromhighrestexture todisc. Unknownradiusisexplicitminpixelsmarker,nonphysical.
Planetschildmoonsgrouped/legiblewithqualitynotes; thousands tinycues LOD.
Prototype mapnoexternalGUI libraries orwebview. Renderingneedsscreensreviewroot.

Providefrozenheader/interfaceearly,meaningfulinputmathsanitychecksifpossible,
HANDOFFactualchanges andpendingroot4Kcontroller/maprenderverification.
Noapps/build/git. Rootinvokesbuildonceallhelpersready.
