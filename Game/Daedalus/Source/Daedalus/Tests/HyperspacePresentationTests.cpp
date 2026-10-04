#include "Solar/HyperspaceTimeline.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHyperPresentationTest,"Daedalus.Presentation.HyperspaceTimeline",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FHyperPresentationTest::RunTest(const FString& Parameters)
{
    FString Json;FHyperTimeline T;
    if(!FFileHelper::LoadFileToString(Json,*(FPaths::ProjectContentDir()/TEXT("Data/Hyperspace/entry.json"))))return false;
    if(!TestTrue(TEXT("presentation data accepted"),T.Load(Json)))return false;
    TestEqual(TEXT("window absent before ignition"),T.Sample(0).Radius,0.);
    TestEqual(TEXT("nose reaches physical window plane"),T.Sample(T.Nose).ShipX+T.Length*.5,T.WindowX);
    TestTrue(TEXT("tail clears physical plane before collapse"),FMath::Abs(T.Sample(T.Tail).ShipX-T.Length*.5-T.WindowX)<.0001);
    TestTrue(TEXT("window is fully open while whole hull crosses"),T.Sample(T.Nose).Radius==1&&T.Sample(T.Tail).Radius==1);
    TestTrue(TEXT("hull clears before aperture starts closing"),!T.Sample(T.Collapse).bExteriorShip);
    TestEqual(TEXT("no residual luminous plate after collapse"),T.Sample(T.Closed).Radius,0.);
    TestTrue(TEXT("interior enabled after cut"),T.Sample(T.Transit+1).Transit==1);
    const double Snapshot=T.Nose;
    const FString Invalid=Json.Replace(TEXT("\"version\": 1"),TEXT("\"version\": 99"));
    TestFalse(TEXT("unsupported version rejected"),T.Load(Invalid));
    TestEqual(TEXT("failed load preserves accepted timeline"),T.Nose,Snapshot);
    const double Before=(T.Sample(T.Nose).ShipX-T.Sample(T.Nose-.001).ShipX)/.001;
    const double After=(T.Sample(T.Nose+.001).ShipX-T.Sample(T.Nose).ShipX)/.001;
    TestTrue(TEXT("acceleration joins crossing without speed discontinuity"),Before>0&&FMath::Abs(After-Before)<After*.002);
    return true;
}
