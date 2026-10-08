# Fetches the public raw data behind Art/Space/PlanetQuality into ignored .local/planet-raw.
# Only needed to regenerate the tracked sources with Tools/Prepare-PlanetSources.py; the game
# build uses the tracked, prepared maps. Data files only; nothing downloaded is executed.
param([string]$Destination = (Join-Path (Split-Path -Parent $PSScriptRoot) '.local/planet-raw'))
$ErrorActionPreference = 'Stop'
$curl = (Get-Command curl.exe -ErrorAction Stop).Source
New-Item -ItemType Directory -Path $Destination -Force | Out-Null

$bmng = 'https://eoimages.gsfc.nasa.gov/images/imagerecords/74000/74167'
$files = [ordered]@{
    'world.200410.3x21600x21600.A1.png' = @("$bmng/world.200410.3x21600x21600.A1.png", 'c924446e3ec97a82338508667ed7375c4bbb3395af72d3572c29c9cbfd07aede')
    'world.200410.3x21600x21600.A2.png' = @("$bmng/world.200410.3x21600x21600.A2.png", '3dccef4c863fab402fc9cece538d1943f40edb1958c1fcd8aab1fcb96099d6dd')
    'world.200410.3x21600x21600.B1.png' = @("$bmng/world.200410.3x21600x21600.B1.png", 'c6787cb5d9abdb6c3914c559ec280bfc70add2cbb2492ed4b2b679d235176f7b')
    'world.200410.3x21600x21600.B2.png' = @("$bmng/world.200410.3x21600x21600.B2.png", '4d9702ef64f11d557cf98f5a495fdf49acf4e93153760fa980e303857c5697de')
    'world.200410.3x21600x21600.C1.png' = @("$bmng/world.200410.3x21600x21600.C1.png", '96abb5eaa38971f62913159cbd6ffb864ff60ab9850070d622939f878fb53488')
    'world.200410.3x21600x21600.C2.png' = @("$bmng/world.200410.3x21600x21600.C2.png", '82bf943c5993d4819039260568d1f13da57d55408b86ee05188571cb83ec02e2')
    'world.200410.3x21600x21600.D1.png' = @("$bmng/world.200410.3x21600x21600.D1.png", 'efa11f777ef023a0132bdee2170f13faaf57a6d91b8c50ecea78ff4dd23c2c2f')
    'world.200410.3x21600x21600.D2.png' = @("$bmng/world.200410.3x21600x21600.D2.png", '03101331c40c0a9dba97e0ba72b35b22525af6ef1fada5ba70c9a8c9087fe940')
    'ETOPO_2022_v1_60s_N90W180_surface.tif' = @('https://www.ngdc.noaa.gov/mgg/global/relief/ETOPO2022/data/60s/60s_surface_elev_gtif/ETOPO_2022_v1_60s_N90W180_surface.tif', '9d27d4b8ea8e76977e2988bca667d7c8fa68b927355feffcddd6b4875a7fd08e')
    'BlackMarble_2016_3km.jpg' = @('https://eoimages.gsfc.nasa.gov/images/imagerecords/144000/144898/BlackMarble_2016_3km.jpg', '230aac448ae68c358be433dd518888cccb3a85ccf66f7b44326441c324ad6725')
    # Moons (USGS Astrogeology global mosaics) and relief (NASA SVS CGI Moon Kit LOLA, PDS MGS MOLA).
    'Ganymede_Voyager_GalileoSSI_global_mosaic_1km.tif' = @('https://planetarymaps.usgs.gov/mosaic/Ganymede_Voyager_GalileoSSI_global_mosaic_1km.tif', 'c2c8d9506b8cf8f7a0a90d823d9052e91c8d9885cf7267fdce8de8216f4df888')
    'Callisto_Voyager_GalileoSSI_global_mosaic_1km.tif' = @('https://planetarymaps.usgs.gov/mosaic/Callisto_Voyager_GalileoSSI_global_mosaic_1km.tif', 'e1f0bd2e0e05de605d067d6b5f5ededddaf31ca6c064562a1ca770f15a7dbaa3')
    'Europa_Voyager_GalileoSSI_global_mosaic_500m.tif' = @('https://planetarymaps.usgs.gov/mosaic/Europa_Voyager_GalileoSSI_global_mosaic_500m.tif', 'a323f0c9ccb47d5af9902ea8297fe81f9a9708795645b80801f103c3f7c9a624')
    'Io_GalileoSSI-Voyager_Global_Mosaic_ClrMerge_1km.tif' = @('https://planetarymaps.usgs.gov/mosaic/Io_GalileoSSI-Voyager_Global_Mosaic_ClrMerge_1km.tif', '135f88cb645bfdb4e045b380971ab2ba14e230590c96c9c86cfe7310fda881ac')
    'Enceladus_Cassini_mosaic_global_110m.tif' = @('https://planetarymaps.usgs.gov/mosaic/Enceladus_Cassini_mosaic_global_110m.tif', 'c9d29292141dd986e52b2a24b692df34701d1dabaee69bcb9fb64ebc6baba335')
    'ldem_16_uint.tif' = @('https://svs.gsfc.nasa.gov/vis/a000000/a004700/a004720/ldem_16_uint.tif', '45a2b32d56e81ed30db07fead8abc842b249b6511219d9ca2c53f81bc2dc5d62')
    'megt90n000eb.img' = @('https://pds-geosciences.wustl.edu/mgs/mgs-m-mola-5-megdr-l3-v1/mgsl_300x/meg016/megt90n000eb.img', 'd18d9b9ab8c5516d02e157dd2cde0f1d0d160c21940e953ba22391269a545e7b')
}
# NASA GIBS daily VIIRS true colour, EPSG:4326 level 5 (40x20 tiles of 512 px). Imagery may be
# reprocessed by NASA, so tiles are not hash-pinned; the tracked prepared cloud map is canonical.
$gibs = @(
    @{Folder = 'gibs-viirs-snpp-2023-07-29'; Layer = 'VIIRS_SNPP_CorrectedReflectance_TrueColor'; Date = '2023-07-29'; Rows = 0..19},
    @{Folder = 'gibs-viirs-noaa20-2023-07-29'; Layer = 'VIIRS_NOAA20_CorrectedReflectance_TrueColor'; Date = '2023-07-29'; Rows = 0..19},
    @{Folder = 'gibs-viirs-snpp-2023-01-29'; Layer = 'VIIRS_SNPP_CorrectedReflectance_TrueColor'; Date = '2023-01-29'; Rows = 15..19}
)

$config = Join-Path $Destination 'fetch.cfg'
$lines = New-Object System.Collections.Generic.List[string]
foreach ($name in $files.Keys) {
    $target = Join-Path $Destination $name
    if (-not (Test-Path -LiteralPath $target)) { $lines.Add("url = `"$($files[$name][0])`""); $lines.Add("output = `"$($target.Replace('\','/'))`"") }
}
foreach ($set in $gibs) {
    $folder = Join-Path $Destination $set.Folder
    New-Item -ItemType Directory -Path $folder -Force | Out-Null
    foreach ($row in $set.Rows) { foreach ($column in 0..39) {
        $target = Join-Path $folder "5_${row}_${column}.jpg"
        if (-not (Test-Path -LiteralPath $target)) {
            $lines.Add("url = `"https://gibs.earthdata.nasa.gov/wmts/epsg4326/best/$($set.Layer)/default/$($set.Date)/250m/5/$row/$column.jpg`"")
            $lines.Add("output = `"$($target.Replace('\','/'))`"")
        }
    } }
}
if ($lines.Count) {
    [IO.File]::WriteAllLines($config, $lines)
    # The NASA image server drops long transfers; resume (-C -) and retry until complete.
    for ($attempt = 1; $attempt -le 20; $attempt++) {
        & $curl -Z --parallel-max 6 -sS -L -C - --retry 5 --retry-delay 3 -K $config
        $missing = @($files.Keys | Where-Object { -not (Test-Path -LiteralPath (Join-Path $Destination $_)) -or (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $Destination $_)).Hash.ToLower() -ne $files[$_][1] })
        if (-not $missing) { break }
    }
}
foreach ($name in $files.Keys) {
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $Destination $name)).Hash.ToLower()
    if ($hash -ne $files[$name][1]) { throw "Checksum mismatch: $name" }
}
foreach ($set in $gibs) {
    $count = @(Get-ChildItem -LiteralPath (Join-Path $Destination $set.Folder) -Filter '5_*.jpg').Count
    if ($count -ne $set.Rows.Count * 40) { throw "Incomplete GIBS set $($set.Folder): $count" }
}
Write-Output "Planet raw sources verified in $Destination"
