<#
.SYNOPSIS
    Stress tester: generator vs. solution vs. brute force (Windows PowerShell 5.1+ / PowerShell 7).

.EXAMPLE
    .\stress.ps1
    .\stress.ps1 -Iterations 5000 -GenArgs "type=tree nmin=2 n=10 shape=random"
    .\stress.ps1 -Sol fast.cpp -Brute slow.cpp -TimeLimit 2
    .\stress.ps1 -NoCompile -Start 500
    .\stress.ps1 -Exact

    The seed is appended to the generator arguments as seed=<i>.
    On failure the input is saved to fail_in.txt (plus fail_sol.txt / fail_brute.txt).
    If scripts are blocked:  powershell -ExecutionPolicy Bypass -File .\stress.ps1
#>
param(
    [int]    $Iterations = 1000,
    [int]    $Start      = 1,
    [string] $Gen        = "gen.cpp",
    [string] $Sol        = "sol.cpp",
    [string] $Brute      = "brute.cpp",
    [string] $GenArgs    = "type=graph nmin=2 n=8 mmin=1 m=12 weighted wl=1 wr=10",
    [double] $TimeLimit  = 5.0,
    [switch] $NoCompile,
    [switch] $Exact
)

$ErrorActionPreference = "Stop"
$isWin = ($PSVersionTable.PSEdition -ne "Core") -or $IsWindows
$ext = if ($isWin) { ".exe" } else { "" }
$genExe   = Join-Path $PWD "gen$ext"
$solExe   = Join-Path $PWD "sol$ext"
$bruteExe = Join-Path $PWD "brute$ext"

function Compile($src, $out) {
    Write-Host "compiling $src ..."
    & g++ -O2 -std=c++17 -o $out $src
    if ($LASTEXITCODE -ne 0) { throw "compilation failed: $src" }
}

# Runs a program with optional stdin file; returns @{Status=ok|tle|rte; Code=int; Err=string}
function Run-Exe($exePath, $inFile, $outFile, $timeoutSec, $argLine = "") {
    $p = @{
        FilePath               = $exePath
        RedirectStandardOutput = (Join-Path $PWD $outFile)
        RedirectStandardError  = (Join-Path $PWD "stderr.tmp")
        NoNewWindow            = $true
        PassThru               = $true
    }
    if ($inFile)  { $p.RedirectStandardInput = (Join-Path $PWD $inFile) }
    if ($argLine) { $p.ArgumentList = $argLine }
    $proc = Start-Process @p
    $null = $proc.Handle                      # keeps ExitCode readable in Windows PowerShell 5.1
    if (-not $proc.WaitForExit([int]($timeoutSec * 1000))) {
        try { $proc.Kill() } catch {}
        return @{ Status = "tle"; Code = $null; Err = "" }
    }
    $proc.WaitForExit()                       # make sure output is flushed
    $err = ""
    if (Test-Path "stderr.tmp") { $err = [string](Get-Content -Raw "stderr.tmp") }
    $status = if ($proc.ExitCode -eq 0) { "ok" } else { "rte" }
    return @{ Status = $status; Code = $proc.ExitCode; Err = $err }
}

function Normalize($path) {
    $raw = [string](Get-Content -Raw $path)
    if ($Exact) { return $raw }
    return (($raw -split '\s+' | Where-Object { $_ -ne '' }) -join ' ')
}

if (-not $NoCompile) {
    Compile $Gen $genExe
    Compile $Sol $solExe
    Compile $Brute $bruteExe
}

$sw = [System.Diagnostics.Stopwatch]::StartNew()
for ($i = $Start; $i -lt $Start + $Iterations; $i++) {
    $r = Run-Exe $genExe $null "in.txt" 10 "seed=$i $GenArgs"
    if ($r.Status -ne "ok") { Write-Host "generator failed on seed $i`n$($r.Err)"; exit 2 }

    $r = Run-Exe $solExe "in.txt" "out.txt" $TimeLimit
    if ($r.Status -ne "ok") {
        $label = if ($r.Status -eq "tle") { "Time limit exceeded" } else { "Runtime error (exit code $($r.Code))" }
        Write-Host "$label on seed $i"
        if ($r.Err) { Write-Host $r.Err }
        Copy-Item in.txt fail_in.txt -Force
        Write-Host "input saved to fail_in.txt"
        exit 1
    }

    $r = Run-Exe $bruteExe "in.txt" "ans.txt" 60
    if ($r.Status -ne "ok") { Write-Host "brute failed ($($r.Status)) on seed $i - fix the brute force first"; exit 2 }

    if ((Normalize "out.txt") -cne (Normalize "ans.txt")) {
        Write-Host "Wrong answer on seed $i"
        Copy-Item in.txt  fail_in.txt    -Force
        Copy-Item out.txt fail_sol.txt   -Force
        Copy-Item ans.txt fail_brute.txt -Force
        if ((Get-Item in.txt).Length -lt 2000) {
            Write-Host "--- input ---`n$(Get-Content -Raw in.txt)"
            Write-Host "--- yours ---`n$(Get-Content -Raw out.txt)"
            Write-Host "--- brute ---`n$(Get-Content -Raw ans.txt)"
        }
        Write-Host "saved fail_in.txt, fail_sol.txt, fail_brute.txt"
        exit 1
    }

    $done = $i - $Start + 1
    if ($done % 100 -eq 0) { Write-Host ("{0} tests passed ({1:N1}s)" -f $done, $sw.Elapsed.TotalSeconds) }
}
Write-Host "All $Iterations tests passed"
