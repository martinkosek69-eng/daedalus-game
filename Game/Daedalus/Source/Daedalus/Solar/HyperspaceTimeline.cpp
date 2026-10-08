#include "Solar/HyperspaceTimeline.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace { double Ease(double V){V=FMath::Clamp(V,0.,1.);return V*V*(3-2*V);} }
bool FHyperTimeline::Load(const FString& Json)
{
    TSharedPtr<FJsonObject> O;
    if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),O)||!O.IsValid())return false;
    FHyperTimeline C;double Version=0;
    auto Read=[&](const TCHAR* Key,double& V){return O->TryGetNumberField(Key,V)&&FMath::IsFinite(V)&&V>=0;};
    if(!Read(TEXT("version"),Version)||Version!=1
       ||!Read(TEXT("openingAtSeconds"),C.Opening)||!Read(TEXT("fullAtSeconds"),C.Full)
       ||!Read(TEXT("accelerationAtSeconds"),C.Acceleration)||!Read(TEXT("noseAtSeconds"),C.Nose)
       ||!Read(TEXT("tailAtSeconds"),C.Tail)||!Read(TEXT("collapseAtSeconds"),C.Collapse)
       ||!Read(TEXT("closedAtSeconds"),C.Closed)||!Read(TEXT("transitAtSeconds"),C.Transit)
       ||!Read(TEXT("endAtSeconds"),C.End)||!Read(TEXT("shipLengthMetres"),C.Length)
       ||!Read(TEXT("windowXMetres"),C.WindowX)||!Read(TEXT("windowRadiusMetres"),C.WindowRadius))return false;
    if(!(C.Opening<C.Full&&C.Full<C.Nose&&C.Opening<=C.Acceleration&&C.Acceleration<C.Nose
         &&C.Nose<C.Tail&&C.Tail<C.Collapse&&C.Collapse<C.Closed&&C.Closed<C.Transit&&C.Transit<C.End
         &&C.End<120&&C.Tail-C.Nose>.05&&C.Full-C.Opening>.05&&C.Closed-C.Collapse>.05
         &&C.Nose-C.Acceleration>.05&&C.Length==600&&C.WindowRadius>C.Length*.6))return false;
    *this=C;return true;
}
FHyperFrame FHyperTimeline::Sample(double Seconds) const
{
    FHyperFrame F;
    const double T=FMath::IsFinite(Seconds)?FMath::Clamp(Seconds,0.,End):0;
    const double Speed=Length/(Tail-Nose), Ramp=Nose-Acceleration;
    const double Start=WindowX-Length*.5-Speed*Ramp*.5;
    const double A=FMath::Clamp(T-Acceleration,0.,Ramp);
    F.ShipX=Start+Speed*A*A/(2*Ramp)+Speed*FMath::Max(0.,T-Nose);
    F.Radius=Ease((T-Opening)/(Full-Opening))*(1-Ease((T-Collapse)/(Closed-Collapse)));
    F.Strength=Ease((T-Opening)/.12)*(1-Ease((T-(Closed-.22))/.22));
    F.Engine=.1+.9*Ease((T-Acceleration)/.45);
    F.Transit=Ease((T-Transit)/.35);
    F.bExteriorShip=T<Tail+.10;
    return F;
}
