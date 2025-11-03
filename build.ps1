$ErrorActionPreference = "Stop"

$preferredGcc = "D:\Program Files\Mingw64\bin\gcc.exe"
$gccCommand = Get-Command $preferredGcc -ErrorAction SilentlyContinue

if (-not $gccCommand) {
    $gccCommand = Get-Command "gcc.exe" -ErrorAction SilentlyContinue
}

if (-not $gccCommand) {
    throw "GCC was not found. Install MinGW or add gcc.exe to PATH."
}

$gccBin = Split-Path -Parent $gccCommand.Source
$env:PATH = "$gccBin;$env:PATH"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $root "build"
$tempDir = Join-Path $buildDir "tmp"
$output = Join-Path $buildDir "mini-rule-monitor.exe"

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

$env:TMP = $tempDir
$env:TEMP = $tempDir

$sources = Get-ChildItem -LiteralPath (Join-Path $root "src") -Filter "*.c" |
    Sort-Object Name |
    ForEach-Object { $_.FullName }

$args = @(
    "-std=c11",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-DUNICODE",
    "-D_UNICODE",
    "-municode",
    "-I", (Join-Path $root "include"),
    "-o", $output
) + $sources

& $gccCommand.Source @args

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE."
}

Write-Host "Built $output"
