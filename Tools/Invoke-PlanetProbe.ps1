# Planet quality probe: native 3840x2160 captures of Earth (close reference-like view, full disc,
# limb, terminator, ocean glint, motion) and optional representative bodies, with GPU frame time,
# process GPU memory and resident planet texture memory in result.json.
#   -Source Editor  : this checkout's uncooked content through UnrealEditor -game (baseline/iteration)
#   -Source Package : .local/solar/<BuildName> produced by Invoke-SolarFlight.ps1 -Mode Package
# Uses its own -UserDir; never touches the player's saves, package or launcher.
param([ValidateSet('Editor','Package')][string]$Source='Package',
      [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$BuildName='Build-PlanetQuality',
      [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='planet',
      [ValidatePattern('^[A-Za-z0-9._,-]*$')][string]$Bodies='')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$config=Get-Content -LiteralPath (Join-Path $root '.local/toolchain.json') -Raw | ConvertFrom-Json
if($config.environment){foreach($p in $config.environment.psobject.Properties){[Environment]::SetEnvironmentVariable($p.Name,[string]$p.Value,'Process')}}
$engine=if($env:DAEDALUS_ENGINE){$env:DAEDALUS_ENGINE}else{$config.engine}
$project=Join-Path $root 'Game/Daedalus/Daedalus.uproject'
$run=Join-Path $root ('.local/planet-probe/'+$Label+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $run -Force | Out-Null
$game=@('/Game/Maps/SolarFlight?game=/Script/Daedalus.SolarFlightGameMode','-nosound','-unattended','-windowed','-ResX=3840','-ResY=2160','-forceres',
        "-UserDir=$run","-SolarSharpProbe=$run",'-SolarPlanetProbe',"-abslog=$(Join-Path $run 'run.log')")
if($Bodies){$game+="-SolarPlanetBodies=$Bodies"}
if($Source -eq 'Editor'){
    if(Get-Process -Name UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue){throw 'Coordinate editor ownership and close it before this operation.'}
    $exe=Join-Path $engine 'Engine/Binaries/Win64/UnrealEditor.exe'
    $arguments=@($project)+$game+@('-game')
}else{
    $exe=Join-Path $root ".local/solar/$BuildName/Windows/Daedalus/Binaries/Win64/Daedalus.exe"
    if(-not(Test-Path -LiteralPath $exe)){throw "Package first: $exe"}
    $arguments=$game
}
$quoted=@($arguments | ForEach-Object {if($_.Contains('"')){throw 'Unsupported quote.'};'"'+$_+'"'})
$p=Start-Process -FilePath $exe -ArgumentList ($quoted -join ' ') -PassThru
if(-not $p.WaitForExit(1200000)){Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue;throw 'Own probe process exceeded twenty minutes.'}
$result=Join-Path $run 'result.json'
if(-not(Test-Path -LiteralPath $result)){throw "Planet probe wrote no result: $run"}
$r=Get-Content -LiteralPath $result -Raw | ConvertFrom-Json
$r.shots | Format-Table name,width,height,gpuMs,frameMs,gpuMemoryMiB,planetTextureMiB,allTextureMiB,cloudShells,finerSpheres,system -AutoSize | Out-String -Width 220
if(-not $r.passed){throw "Planet probe checks failed: $run"}
Write-Output "Planet probe passed: $run"
