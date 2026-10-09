param([ValidateSet('Create','Render','Audit','Video')][string]$Mode='Render',
      [string]$Frames='1:108')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$cfg=Get-Content -LiteralPath (Join-Path $root '.local/toolchain.json') -Raw | ConvertFrom-Json
if($cfg.environment){foreach($e in $cfg.environment.psobject.Properties){[Environment]::SetEnvironmentVariable($e.Name,[string]$e.Value,'Process')}}
$blend=Join-Path $root 'Art/Effects/Hyperspace/Blender/Daedalus_GreenRupture.blend'
$local=Join-Path $root '.local/hyper/blender0030'
New-Item -ItemType Directory -Force -Path $local | Out-Null
if($Mode -ne 'Video' -and (Get-Process -Name blender,UnrealEditor,UnrealEditor-Cmd,Daedalus -ErrorAction SilentlyContinue)){
    throw 'Coordinate shared application ownership before rendering.'
}
switch($Mode){
    'Create' { & $cfg.blender -b --python-exit-code 1 --python (Join-Path $PSScriptRoot 'Create-BlenderHyperspace.py') -- --root $root --still 44 }
    'Render' { & $cfg.blender -b $blend --python-exit-code 1 --python (Join-Path $PSScriptRoot 'Render-BlenderHyperspace.py') -- --root $root --frames $Frames }
    'Audit' { & $cfg.blender -b $blend --python-exit-code 1 --python (Join-Path $PSScriptRoot 'Render-BlenderHyperspace.py') -- --root $root --audit-only }
    'Video' {
        $ffmpeg=if($env:DAEDALUS_FFMPEG){$env:DAEDALUS_FFMPEG}else{$cfg.ffmpeg}
        if(-not $ffmpeg){throw 'Configure ffmpeg in local toolchain.'}
        $sourceHash=(Get-FileHash -LiteralPath $blend -Algorithm SHA256).Hash.ToLowerInvariant()
        $framesPath=Join-Path $local ('frames/'+$sourceHash.Substring(0,12))
        for($i=1;$i -le 108;$i++){if(-not(Test-Path -LiteralPath (Join-Path $framesPath ('{0:d4}.png' -f $i)))){throw "Missing frame $i"}}
        & $ffmpeg -hide_banner -loglevel error -y -framerate 24 -start_number 1 -i (Join-Path $framesPath '%04d.png') -frames:v 108 -c:v libx264 -preset slow -crf 15 -pix_fmt yuv420p -movflags +faststart (Join-Path $local 'Daedalus-Blender-GreenRupture-4K.mp4')
    }
}
if($LASTEXITCODE -ne 0){throw "Blender hyperspace $Mode failed: $LASTEXITCODE"}
