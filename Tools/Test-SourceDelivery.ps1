param([string]$Branch='codex/game-foundation',[string]$Remote='origin')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    $current=(& git branch --show-current).Trim()
    if($LASTEXITCODE -ne 0 -or $current -ne $Branch){throw 'Run in the matching delivery checkout/branch.'}
    $metadata=(& git lfs ls-files --json | ConvertFrom-Json).files
    if($LASTEXITCODE -ne 0 -or $metadata.Count -lt 1){throw 'No LFS assets to verify.'}
    $store=Join-Path $root ('.local/lfs-verification-'+[guid]::NewGuid().ToString('N'))
    # A fresh independent store forces a real server fetch, not the normal local cache.
    & git -c "lfs.storage=$store" lfs fetch $Remote $Branch
    if($LASTEXITCODE -ne 0){throw 'Remote LFS fetch failed.'}
    foreach($file in $metadata){
        $oid=$file.oid
        if($oid -notmatch '^[a-f0-9]{64}$'){throw 'Unexpected LFS object identity.'}
        $object=Join-Path $store ("objects/"+$oid.Substring(0,2)+'/'+$oid.Substring(2,2)+'/'+$oid)
        $source=Join-Path $root $file.name
        if(-not (Test-Path -LiteralPath $object)){throw "Remote object missing: $($file.name)"}
        if((Get-FileHash -LiteralPath $object -Algorithm SHA256).Hash.ToLower() -ne $oid -or
           (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash.ToLower() -ne $oid -or
           (Get-Item -LiteralPath $object).Length -ne $file.size){throw "Delivery mismatch: $($file.name)"}
    }
    Write-Output "Verified $($metadata.Count) source/assets via independent remote LFS download and SHA256."
} finally {Pop-Location}
