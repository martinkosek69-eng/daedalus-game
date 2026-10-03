param([ValidateSet('Build','Assets','Test','Package','Smoke','Visual','Play')][string]$Mode='Play')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$config=Get-Content -LiteralPath (Join-Path $root '.local/toolchain.json') -Raw | ConvertFrom-Json
$engine=if($env:DAEDALUS_ENGINE){$env:DAEDALUS_ENGINE}else{$config.engine}
if(-not $engine){throw 'Configure .local/toolchain.json first.'}
if($config.environment){foreach($p in $config.environment.psobject.Properties){[Environment]::SetEnvironmentVariable($p.Name,[string]$p.Value,'Process')}}
$project=Join-Path $root 'Game/Daedalus/Daedalus.uproject'
$local=Join-Path $root '.local/solar'
New-Item -ItemType Directory -Path $local -Force | Out-Null
$env:uebp_LogFolder=Join-Path $local 'AutomationLogs'
$env:uebp_FinalLogFolder=$env:uebp_LogFolder
$editor=Join-Path $engine 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
function CheckExit {if($LASTEXITCODE -ne 0){throw "Tool failed with exit code $LASTEXITCODE"}}
function RunOwnGame([string]$Exe,[string[]]$GameArguments) {
    $quoted=@($GameArguments | ForEach-Object {if($_.Contains('"')){throw 'Unsupported quote.'};'"'+$_+'"'})
    $p=Start-Process -FilePath $Exe -ArgumentList ($quoted -join ' ') -WindowStyle Hidden -PassThru
    if(-not $p.WaitForExit(300000)){Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue;throw 'Own probe process exceeded five minutes.'}
    $p.Refresh()
    if($p.ExitCode -ne 0){throw "Own game test exited with $($p.ExitCode)"}
}
function RequireNoEditor {
    if(Get-Process -Name UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue){throw 'Coordinate editor ownership and close it before this operation.'}
}
switch($Mode){
 'Build' { & (Join-Path $PSScriptRoot 'Invoke-Foundation.ps1') Build; CheckExit }
 'Assets' {
    RequireNoEditor
    & $editor $project -run=pythonscript "-script=$(Join-Path $PSScriptRoot 'Prepare-SolarContent.py')" -unattended -nullrhi -nosound "-abslog=$(Join-Path $local 'assets.log')"
    CheckExit
 }
 'Test' {
    RequireNoEditor
    $report=Join-Path $local ('tests-'+[guid]::NewGuid().ToString('N'))
    & $editor $project -unattended -nullrhi -nosound '-ExecCmds=Automation RunTests Daedalus' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$report" "-abslog=$(Join-Path $report 'run.log')"
    CheckExit
    $index=Join-Path $report 'index.json'
    if(-not (Test-Path -LiteralPath $index)){throw 'Missing automation report.'}
    $r=Get-Content -LiteralPath $index -Raw | ConvertFrom-Json
    if($r.failed -ne 0 -or $r.notRun -ne 0 -or $r.inProcess -ne 0 -or $r.succeeded -lt 15){throw 'Suite incomplete or failed.'}
    $names=@('ThrottleAccelerationBraking','TurnPitchBankAndDrift','PartitionsPauseAndValidation','LargeCoordinateSweptContact')
    foreach($name in $names){
        $test=@($r.tests | Where-Object {$_.fullTestPath -eq "Daedalus.Flight.$name"})
        if($test.Count -ne 1 -or $test[0].state -ne 'Success'){throw "Required flight check failed: $name"}
    }
    $r | Select-Object succeeded,failed,notRun,inProcess
    Write-Output "Report: $report"
 }
 'Package' {
    RequireNoEditor
    & (Join-Path $engine 'Engine/Build/BatchFiles/RunUAT.bat') BuildCookRun "-project=$project" -nop4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -archive "-archivedirectory=$(Join-Path $local 'Build')" '-map=/Game/Maps/SolarFlight+/Game/Maps/Foundation' '-ubtargs=-MaxParallelActions=1' -utf8output
    CheckExit
 }
 'Smoke' {
    $exe=Join-Path $local 'Build/Windows/Daedalus/Binaries/Win64/Daedalus.exe'
    if(-not(Test-Path -LiteralPath $exe)){throw 'Package first.'}
    $run=Join-Path $local ('foundation-restart-'+[guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $run -Force | Out-Null
    foreach($phase in @('write','read')){
        $result=Join-Path $run ($phase+'.json')
        RunOwnGame $exe @('/Game/Maps/Foundation?game=/Script/Daedalus.DaedalusGameMode','-nullrhi','-nosound','-unattended',"-UserDir=$run","-DaedalusSaveDir=$(Join-Path $run 'SaveGames')","-DaedalusSmoke=$phase","-DaedalusSmokeResult=$result","-abslog=$(Join-Path $run ($phase+'.log'))")
        if(-not(Test-Path -LiteralPath $result) -or -not(Get-Content -LiteralPath $result -Raw | ConvertFrom-Json).passed){throw "Foundation $phase failed in solar package: $run"}
    }
    Write-Output "Foundation separate-process restart passed in solar package: $run"
 }
 'Visual' {
    $exe=Join-Path $local 'Build/Windows/Daedalus/Binaries/Win64/Daedalus.exe'
    if(-not(Test-Path -LiteralPath $exe)){throw 'Package first.'}
    $run=Join-Path $local ('visual-'+[guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $run -Force | Out-Null
    $args=@('/Game/Maps/SolarFlight?game=/Script/Daedalus.SolarFlightGameMode','-nosound','-unattended','-windowed','-ResX=1280','-ResY=720',"-UserDir=$run","-SolarProbe=$run","-abslog=$(Join-Path $run 'run.log')")
    RunOwnGame $exe $args
    $result=Join-Path $run 'result.json'
    if(-not(Test-Path -LiteralPath $result) -or -not(Get-Content -LiteralPath $result -Raw | ConvertFrom-Json).passed){throw "Solar input/render probe failed: $run"}
    Write-Output "Solar input/render checks passed; inspect earth.png, turn.png, sun.png: $run"
 }
 'Play' {
    $exe=Join-Path $local 'Build/Windows/Daedalus.exe'
    if(-not(Test-Path -LiteralPath $exe)){throw 'Playable package not found. Run Package first.'}
    $data=Join-Path $local 'PlayerData'
    & $exe '/Game/Maps/SolarFlight?game=/Script/Daedalus.SolarFlightGameMode' "-UserDir=$data" '-windowed' '-ResX=1280' '-ResY=720'
    CheckExit
 }
}
