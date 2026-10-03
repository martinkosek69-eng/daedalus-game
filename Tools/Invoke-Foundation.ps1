param([ValidateSet('Build','Assets','Test','Package','Smoke','Visual','Play','Editor')][string]$Mode='Build')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$config=Get-Content -LiteralPath (Join-Path $root '.local/toolchain.json') -Raw | ConvertFrom-Json
$engine=if($env:DAEDALUS_ENGINE){$env:DAEDALUS_ENGINE}else{$config.engine}
$blender=if($env:DAEDALUS_BLENDER){$env:DAEDALUS_BLENDER}else{$config.blender}
if(-not $engine){throw 'Configure .local/toolchain.json first.'}
if($config.environment){foreach($p in $config.environment.psobject.Properties){[Environment]::SetEnvironmentVariable($p.Name,[string]$p.Value,'Process')}}
$project=Join-Path $root 'Game/Daedalus/Daedalus.uproject'
$local=Join-Path $root '.local/foundation'
New-Item -ItemType Directory -Path $local -Force | Out-Null
$env:uebp_LogFolder=Join-Path $local 'AutomationLogs'
$env:uebp_FinalLogFolder=$env:uebp_LogFolder
$editor=Join-Path $engine 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
function CheckExit {if($LASTEXITCODE -ne 0){throw "Tool failed with exit code $LASTEXITCODE"}}
function RunTestGame([string]$Exe,[string[]]$GameArguments) {
    $quoted=@($GameArguments | ForEach-Object {if($_.Contains('"')){throw 'Quote in game argument is unsupported.'}; '"'+$_+'"'})
    $process=Start-Process -FilePath $Exe -ArgumentList ($quoted -join ' ') -WindowStyle Hidden -PassThru
    if(-not $process.WaitForExit(300000)){
        # Close only the test process this script created, never a pre-existing editor/game.
        Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        throw 'Test game exceeded five minutes.'
    }
    $process.Refresh()
    $global:LASTEXITCODE=$process.ExitCode
    if($process.ExitCode -ne 0){throw "Test game exited with $($process.ExitCode)"}
}
function RequireNoEditor {
    $busy=Get-Process -Name UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue
    if($busy){throw 'An Unreal editor is running. Coordinate ownership and close the applicable editor before this operation.'}
}
switch($Mode){
 'Editor' {
    RequireNoEditor
    $editorArguments=@(('"'+$project+'"'),'-ModelContextProtocolStartServer',('-abslog="'+(Join-Path $local 'editor.log')+'"'))
    Start-Process -FilePath (Join-Path $engine 'Engine/Binaries/Win64/UnrealEditor.exe') -ArgumentList $editorArguments -WindowStyle Hidden
 }
 'Build' {
    RequireNoEditor
    & (Join-Path $engine 'Engine/Build/BatchFiles/Build.bat') DaedalusEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=1
    CheckExit
 }
 'Assets' {
    RequireNoEditor
    & $blender --background --factory-startup --python-exit-code 1 --python (Join-Path $PSScriptRoot 'Create-FoundationAssets.py')
    CheckExit
    & $editor $project -run=pythonscript "-script=$(Join-Path $PSScriptRoot 'Prepare-FoundationContent.py')" -unattended -nullrhi -nosound "-abslog=$(Join-Path $local 'assets.log')"
    CheckExit
 }
 'Test' {
    RequireNoEditor
    $report=Join-Path $local ('tests-'+[guid]::NewGuid().ToString('N'))
    & $editor $project -unattended -nullrhi -nosound '-ExecCmds=Automation RunTests Daedalus.Foundation' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$report" "-abslog=$(Join-Path $report 'run.log')"
    CheckExit
    $index=Join-Path $report 'index.json'
    if(-not (Test-Path -LiteralPath $index)){throw 'Automation report missing; a zero exit alone is insufficient.'}
    $results=Get-Content -Raw -LiteralPath $index | ConvertFrom-Json
    if($results.failed -ne 0 -or $results.notRun -ne 0 -or $results.inProcess -ne 0 -or $results.succeeded -lt 11){throw 'Automation suite did not complete successfully.'}
    $expected=@('Persistence.Recovery','ArrivalAndSpawnSafety','BodyObstruction','CatalogExpansionAndSpawn','CatalogValidation','Combat','FixedStepPause','GuardPilot','LargeCatalogCoordinates','SaveValidation','TravelTransportPersistence')
    foreach($name in $expected){
        $test=@($results.tests | Where-Object {$_.fullTestPath -eq "Daedalus.Foundation.$name"})
        if($test.Count -ne 1 -or $test[0].state -ne 'Success'){throw "Required test did not succeed: $name"}
    }
    $results | Select-Object succeeded,failed,notRun,inProcess
    Write-Output "Report: $report"
 }
 'Package' {
    RequireNoEditor
    & (Join-Path $engine 'Engine/Build/BatchFiles/RunUAT.bat') BuildCookRun "-project=$project" -nop4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive "-archivedirectory=$(Join-Path $local 'Build')" '-map=/Game/Maps/Foundation' '-ubtargs=-MaxParallelActions=1' -utf8output
    CheckExit
 }
 'Smoke' {
    $exe=Join-Path $local 'Build/Windows/Daedalus/Binaries/Win64/Daedalus.exe'
    if(-not (Test-Path -LiteralPath $exe)){throw 'Package first.'}
    $run=Join-Path $local ('smoke-'+[guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $run -Force | Out-Null
    foreach($phase in @('write','read')){
        $result=Join-Path $run ($phase+'.json')
        RunTestGame $exe @('-nullrhi','-nosound','-unattended',"-UserDir=$run","-DaedalusSaveDir=$(Join-Path $run 'SaveGames')","-DaedalusSmoke=$phase","-DaedalusSmokeResult=$result","-abslog=$(Join-Path $run ($phase+'.log'))")
        if(-not (Test-Path -LiteralPath $result) -or -not (Get-Content -Raw -LiteralPath $result | ConvertFrom-Json).passed){throw "Packaged $phase did not pass."}
    }
    Write-Output "Packaged separate-process restart passed: $run"
 }
 'Visual' {
    $exe=Join-Path $local 'Build/Windows/Daedalus/Binaries/Win64/Daedalus.exe'
    if(-not (Test-Path -LiteralPath $exe)){throw 'Package first.'}
    $run=Join-Path $local ('visual-'+[guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $run -Force | Out-Null
    RunTestGame $exe @('-nosound','-unattended','-windowed','-ResX=1280','-ResY=720',"-UserDir=$run","-DaedalusSaveDir=$(Join-Path $run 'SaveGames')","-DaedalusVisualProbe=$run","-abslog=$(Join-Path $run 'run.log')")
    $result=Join-Path $run 'result.json'
    if(-not (Test-Path -LiteralPath $result) -or -not (Get-Content -Raw -LiteralPath $result | ConvertFrom-Json).passed){throw 'Rendered game/input probe did not pass.'}
    Write-Output "Rendered game/input checks passed; inspect space.png and location.png: $run"
 }
 'Play' {
    $exe=Join-Path $local 'Build/Windows/Daedalus.exe'
    if(-not (Test-Path -LiteralPath $exe)){throw 'Package first.'}
    $userData=Join-Path $local 'PlayerData'
    & $exe "-UserDir=$userData" "-DaedalusSaveDir=$(Join-Path $userData 'SaveGames')" '-windowed' '-ResX=1280' '-ResY=720'
    CheckExit
 }
}
