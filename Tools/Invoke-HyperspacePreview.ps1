param([ValidateSet('Materials','Package','Stills','Render','Play')][string]$Mode='Play')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$config=Get-Content -LiteralPath (Join-Path $root '.local/toolchain.json') -Raw | ConvertFrom-Json
if($config.environment){foreach($entry in $config.environment.psobject.Properties){[Environment]::SetEnvironmentVariable($entry.Name,[string]$entry.Value,'Process')}}
$local=Join-Path $root '.local/hyper'
New-Item -ItemType Directory -Path $local -Force | Out-Null
$project=Join-Path $root 'Game/Daedalus/Daedalus.uproject'
$exe=Join-Path $root '.local/solar/Build-Hyperspace28/Windows/Daedalus/Binaries/Win64/Daedalus.exe'
if(Get-Process -Name UnrealEditor,UnrealEditor-Cmd,Daedalus -ErrorAction SilentlyContinue){throw 'Coordinate shared application ownership first.'}
switch($Mode){
 'Materials' {
   $editor=Join-Path $config.engine 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
   & $editor $project -run=pythonscript "-script=$(Join-Path $PSScriptRoot 'Prepare-HyperspaceMaterials.py')" -unattended -nullrhi -nosound "-abslog=$(Join-Path $local 'materials.log')"
   if($LASTEXITCODE -ne 0){throw 'Hyperspace material generation failed.'}
 }
 'Package' {
   & (Join-Path $PSScriptRoot 'Invoke-SolarFlight.ps1') -Mode Package -BuildName Build-Hyperspace28
   if($LASTEXITCODE -ne 0){throw 'Hyperspace package failed.'}
 }
 'Play' {
   if(-not(Test-Path -LiteralPath $exe)){throw 'Package the preview first.'}
   Write-Output 'DAEDALUS: R / 1 replay, SPACE pause, 2 hyperspace interior, ESC close.'
   & $exe '/Game/Maps/SolarFlight?game=/Script/Daedalus.SolarFlightGameMode' -HyperspacePreview -SolarNative "-UserDir=$(Join-Path $local 'PlayerData')"
 }
 {$_ -in @('Stills','Render')} {
   if(-not(Test-Path -LiteralPath $exe)){throw 'Package the preview first.'}
   $run=Join-Path $local ($Mode.ToLower()+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
   New-Item -ItemType Directory -Path $run -Force | Out-Null
   $args=@('/Game/Maps/SolarFlight?game=/Script/Daedalus.SolarFlightGameMode','-HyperspacePreview','-nosound','-unattended','-windowed','-ResX=3840','-ResY=2160','-forceres',"-UserDir=$run","-HyperCapture=$run","-abslog=$(Join-Path $run 'run.log')")
   if($Mode -eq 'Stills'){$args+='-HyperStills'}
   $quoted=@($args | ForEach-Object {'"'+$_+'"'})
   $process=Start-Process -FilePath $exe -ArgumentList ($quoted -join ' ') -WindowStyle Hidden -PassThru
   if(-not $process.WaitForExit(900000)){Stop-Process -Id $process.Id -Force;throw 'Own preview exceeded time limit.'}
   $process.Refresh()
   $result=Join-Path $run 'result.json'
   if($process.ExitCode -ne 0 -or -not(Test-Path -LiteralPath $result) -or -not(Get-Content -LiteralPath $result -Raw | ConvertFrom-Json).passed){throw "Preview check failed: $run"}
   if($Mode -eq 'Render'){
     $ffmpeg=if($env:DAEDALUS_FFMPEG){$env:DAEDALUS_FFMPEG}else{$config.ffmpeg}
     if(-not $ffmpeg){throw 'Frames passed; configure ffmpeg locally to encode preview.'}
     & $ffmpeg -hide_banner -loglevel error -y -framerate 30 -i (Join-Path $run '%04d.png') -c:v libx264 -preset medium -crf 16 -pix_fmt yuv420p -movflags +faststart (Join-Path $local 'Daedalus-hyperspace-4K.mp4')
     if($LASTEXITCODE -ne 0){throw 'Frames passed; encoding failed.'}
   }
   Write-Output "HYPERSPACE_PREVIEW_PASS: $run"
 }
}
