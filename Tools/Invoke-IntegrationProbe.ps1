param([ValidatePattern('^[A-Za-z0-9_-]+$')][string]$BuildName='Build-Integrated25')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$config=Get-Content -LiteralPath (Join-Path $root '.local/toolchain.json') -Raw | ConvertFrom-Json
if($config.environment){foreach($p in $config.environment.psobject.Properties){[Environment]::SetEnvironmentVariable($p.Name,[string]$p.Value,'Process')}}
if(Get-Process -Name UnrealEditor,UnrealEditor-Cmd,Daedalus -ErrorAction SilentlyContinue){throw 'Coordinate application ownership before this operation.'}
$exe=Join-Path $root ".local/solar/$BuildName/Windows/Daedalus/Binaries/Win64/Daedalus.exe"
if(-not(Test-Path -LiteralPath $exe)){throw 'Package first.'}
$run=Join-Path $root ('.local/integration-probe/'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $run -Force | Out-Null
$args=@('/Game/Maps/SolarFlight?game=/Script/Daedalus.SolarFlightGameMode','-nosound','-unattended','-windowed','-ResX=3840','-ResY=2160','-forceres',
    "-UserDir=$run","-SolarSharpProbe=$run",'-SolarIntegrationProbe',"-abslog=$(Join-Path $run 'run.log')")
$quoted=@($args | ForEach-Object {'"'+$_+'"'})
$p=Start-Process -FilePath $exe -ArgumentList ($quoted -join ' ') -WindowStyle Hidden -PassThru
if(-not $p.WaitForExit(300000)){Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue;throw 'Own probe exceeded five minutes.'}
$p.Refresh()
$result=Join-Path $run 'result.json'
if(-not(Test-Path -LiteralPath $result) -or -not(Get-Content -LiteralPath $result -Raw | ConvertFrom-Json).passed -or $p.ExitCode -ne 0){throw "Integration checks failed: $run"}
Write-Output "INTEGRATION_PASS: $run"
