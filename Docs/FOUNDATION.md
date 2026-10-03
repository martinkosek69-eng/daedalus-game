# Základ hry

Hra je čistě pro jednoho hráče na Windows. Vesmír může obsahovat více galaxií
a mnoho soustav. Velké lodě běžně zůstávají ve vesmíru; návštěvy povrchů a
interiérů probíhají transportem, raketoplány jsou budoucí rozšíření. Souvislé
přistávání velkých lodí ani plně prozkoumatelná kulatá planeta nejsou požadavkem
tohoto základu. Předloha BC zůstává samostatnou chráněnou referencí.

## Co znamená hotový základ

Funkční propojení katalogu, času, lodí, základního poškození, změny soustavy,
transportu a obnovy uloženého stavu, ověřené v editorových testech i samostatné
hře. Neznamená hotový příběh, ekonomiku, kompletní souboje nebo všechny funkce,
které nás v dalších letech napadnou. Nové schopnosti se průběžně implementují
a měří; složka nebo rozhraní samo není dokončená funkce.

## Kde co patří

| Umístění | Účel |
| --- | --- |
| Game/Daedalus/Source/DaedalusSimulation | Údaje a skutečný stav světa, nezávislý na scéně |
| Game/Daedalus/Source/Daedalus | Napojení na Unreal, ukládání, zobrazení a ovládání |
| Game/Daedalus/Content/Data | Jediný zdroj textových katalogů používaných i samostatnou hrou |
| Game/Daedalus/Content | Herní modely, materiály a mapy |
| Art | Editovatelné zdroje modelů a dohledatelný původ |
| Tools | Opakovatelné sestavení, kontroly a příprava obsahu |
| Docs | Dohody, architektura, současný stav a ověřování |
| Tasks | Vymezené úkoly a předávání agentům |

Přidání lodě obvykle znamená nová data a model. Nová schopnost lodě znamená
rozšíření příslušných pravidel. Herní údaje, konkrétní stav a vzhled jsou oddělené.

Podrobnosti: [architektura](ARCHITECTURE.md), [tvorba obsahu](CONTENT_WORKFLOW.md),
[kontroly](TESTING.md), [rozhodnutí](DECISIONS.md), [současný stav](CURRENT_STATE.md).
